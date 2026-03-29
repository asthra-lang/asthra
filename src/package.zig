const std = @import("std");
const toml = @import("toml.zig");

pub const Dependency = struct {
    name: []const u8,
    git_url: []const u8,
    tag: ?[]const u8,
    commit: ?[]const u8,
};

pub const PackageManifest = struct {
    name: []const u8,
    version: []const u8,
    dependencies: std.ArrayList(Dependency),
    allocator: std.mem.Allocator,

    pub fn parseFromFile(allocator: std.mem.Allocator, path: []const u8) !PackageManifest {
        var table = try toml.parseFile(allocator, path);
        defer table.deinit();
        return fromToml(allocator, &table);
    }

    pub fn parseFromSource(allocator: std.mem.Allocator, source: []const u8) !PackageManifest {
        var table = try toml.parse(allocator, source);
        defer table.deinit();
        return fromToml(allocator, &table);
    }

    fn fromToml(allocator: std.mem.Allocator, table: *toml.TomlTable) !PackageManifest {
        var manifest = PackageManifest{
            .name = "",
            .version = "",
            .dependencies = std.ArrayList(Dependency){},
            .allocator = allocator,
        };

        // [package] section
        if (table.getString("package", "name")) |name| {
            manifest.name = try allocator.dupe(u8, name);
        }
        if (table.getString("package", "version")) |version| {
            manifest.version = try allocator.dupe(u8, version);
        }

        // [dependencies] section
        if (table.getSection("dependencies")) |deps_section| {
            var it = deps_section.iterator();
            while (it.next()) |entry| {
                const dep_name = entry.key_ptr.*;
                switch (entry.value_ptr.*) {
                    .table => |*dep_table| {
                        const git_url = dep_table.get("git") orelse continue;
                        try manifest.dependencies.append(allocator, .{
                            .name = try allocator.dupe(u8, dep_name),
                            .git_url = try allocator.dupe(u8, git_url),
                            .tag = if (dep_table.get("tag")) |t| try allocator.dupe(u8, t) else null,
                            .commit = if (dep_table.get("commit")) |c| try allocator.dupe(u8, c) else null,
                        });
                    },
                    .string => {},
                }
            }
        }

        return manifest;
    }

    pub fn deinit(self: *PackageManifest) void {
        if (self.name.len > 0) self.allocator.free(self.name);
        if (self.version.len > 0) self.allocator.free(self.version);
        for (self.dependencies.items) |dep| {
            self.allocator.free(dep.name);
            self.allocator.free(dep.git_url);
            if (dep.tag) |t| self.allocator.free(t);
            if (dep.commit) |c| self.allocator.free(c);
        }
        self.dependencies.deinit(self.allocator);
    }

    pub fn findByGitUrl(self: *const PackageManifest, url: []const u8) ?Dependency {
        for (self.dependencies.items) |dep| {
            if (std.mem.eql(u8, dep.git_url, url)) return dep;
        }
        return null;
    }
};

pub const ResolvedDep = struct {
    name: []const u8,
    git_url: []const u8,
    commit: []const u8,
    tag: ?[]const u8,
};

pub const Lockfile = struct {
    packages: std.ArrayList(ResolvedDep),
    allocator: std.mem.Allocator,

    pub fn parseFromFile(allocator: std.mem.Allocator, path: []const u8) !?Lockfile {
        const source = std.fs.cwd().readFileAlloc(allocator, path, 1024 * 1024) catch return null;
        defer allocator.free(source);
        return @as(?Lockfile, try parseFromSource(allocator, source));
    }

    pub fn parseFromSource(allocator: std.mem.Allocator, source: []const u8) !Lockfile {
        var lockfile = Lockfile{
            .packages = std.ArrayList(ResolvedDep){},
            .allocator = allocator,
        };

        // Parse [[package]] blocks
        var current_name: ?[]const u8 = null;
        var current_git: ?[]const u8 = null;
        var current_commit: ?[]const u8 = null;
        var current_tag: ?[]const u8 = null;

        var line_start: usize = 0;
        while (line_start < source.len) {
            var line_end = line_start;
            while (line_end < source.len and source[line_end] != '\n') : (line_end += 1) {}
            const line = std.mem.trim(u8, source[line_start..line_end], " \t\r");
            line_start = line_end + 1;

            if (line.len == 0 or line[0] == '#') continue;

            if (std.mem.eql(u8, line, "[[package]]")) {
                // Flush previous package
                if (current_name != null and current_git != null and current_commit != null) {
                    try lockfile.packages.append(allocator, .{
                        .name = try allocator.dupe(u8, current_name.?),
                        .git_url = try allocator.dupe(u8, current_git.?),
                        .commit = try allocator.dupe(u8, current_commit.?),
                        .tag = if (current_tag) |t| try allocator.dupe(u8, t) else null,
                    });
                }
                current_name = null;
                current_git = null;
                current_commit = null;
                current_tag = null;
                continue;
            }

            if (std.mem.indexOf(u8, line, "=")) |eq_pos| {
                const key = std.mem.trim(u8, line[0..eq_pos], " \t");
                const val_raw = std.mem.trim(u8, line[eq_pos + 1 ..], " \t");
                const val = if (val_raw.len >= 2 and val_raw[0] == '"' and val_raw[val_raw.len - 1] == '"')
                    val_raw[1 .. val_raw.len - 1]
                else
                    val_raw;

                if (std.mem.eql(u8, key, "name")) current_name = val;
                if (std.mem.eql(u8, key, "git")) current_git = val;
                if (std.mem.eql(u8, key, "commit")) current_commit = val;
                if (std.mem.eql(u8, key, "tag")) current_tag = val;
            }
        }

        // Flush last package
        if (current_name != null and current_git != null and current_commit != null) {
            try lockfile.packages.append(allocator, .{
                .name = try allocator.dupe(u8, current_name.?),
                .git_url = try allocator.dupe(u8, current_git.?),
                .commit = try allocator.dupe(u8, current_commit.?),
                .tag = if (current_tag) |t| try allocator.dupe(u8, t) else null,
            });
        }

        return lockfile;
    }

    pub fn writeToFile(self: *const Lockfile, path: []const u8) !void {
        const file = try std.fs.cwd().createFile(path, .{});
        defer file.close();
        const writer = file.deprecatedWriter();

        try writer.writeAll("# Auto-generated by asthra. Do not edit.\n\n");
        for (self.packages.items) |pkg| {
            try writer.writeAll("[[package]]\n");
            try writer.print("name = \"{s}\"\n", .{pkg.name});
            try writer.print("git = \"{s}\"\n", .{pkg.git_url});
            try writer.print("commit = \"{s}\"\n", .{pkg.commit});
            if (pkg.tag) |tag| {
                try writer.print("tag = \"{s}\"\n", .{tag});
            }
            try writer.writeAll("\n");
        }
    }

    pub fn findByGitUrl(self: *const Lockfile, url: []const u8) ?ResolvedDep {
        for (self.packages.items) |pkg| {
            if (std.mem.eql(u8, pkg.git_url, url)) return pkg;
        }
        return null;
    }

    pub fn deinit(self: *Lockfile) void {
        for (self.packages.items) |pkg| {
            self.allocator.free(pkg.name);
            self.allocator.free(pkg.git_url);
            self.allocator.free(pkg.commit);
            if (pkg.tag) |t| self.allocator.free(t);
        }
        self.packages.deinit(self.allocator);
    }
};

// ── Tests ──────────────────────────────────────────────────────────────────

const testing = std.testing;

test "parse package manifest" {
    const source =
        \\[package]
        \\name = "myapp"
        \\version = "0.1.0"
        \\
        \\[dependencies]
        \\mathutils = { git = "github.com/user/mathutils", tag = "v1.0.0" }
    ;
    var manifest = try PackageManifest.parseFromSource(testing.allocator, source);
    defer manifest.deinit();

    try testing.expectEqualStrings("myapp", manifest.name);
    try testing.expectEqualStrings("0.1.0", manifest.version);
    try testing.expectEqual(@as(usize, 1), manifest.dependencies.items.len);

    const dep = manifest.dependencies.items[0];
    try testing.expectEqualStrings("mathutils", dep.name);
    try testing.expectEqualStrings("github.com/user/mathutils", dep.git_url);
    try testing.expectEqualStrings("v1.0.0", dep.tag.?);
    try testing.expect(dep.commit == null);
}

test "find dependency by git url" {
    const source =
        \\[dependencies]
        \\mathutils = { git = "github.com/user/mathutils", tag = "v1.0.0" }
        \\logger = { git = "gitlab.com/team/logger", commit = "abc123" }
    ;
    var manifest = try PackageManifest.parseFromSource(testing.allocator, source);
    defer manifest.deinit();

    const found = manifest.findByGitUrl("github.com/user/mathutils");
    try testing.expect(found != null);
    try testing.expectEqualStrings("mathutils", found.?.name);

    try testing.expect(manifest.findByGitUrl("github.com/nonexistent") == null);
}

test "parse lockfile" {
    const source =
        \\# Auto-generated by asthra. Do not edit.
        \\
        \\[[package]]
        \\name = "mathutils"
        \\git = "github.com/user/mathutils"
        \\commit = "a1b2c3d4"
        \\tag = "v1.0.0"
        \\
        \\[[package]]
        \\name = "logger"
        \\git = "gitlab.com/team/logger"
        \\commit = "abc123f0"
    ;
    var lockfile = try Lockfile.parseFromSource(testing.allocator, source);
    defer lockfile.deinit();

    try testing.expectEqual(@as(usize, 2), lockfile.packages.items.len);
    try testing.expectEqualStrings("mathutils", lockfile.packages.items[0].name);
    try testing.expectEqualStrings("a1b2c3d4", lockfile.packages.items[0].commit);
    try testing.expectEqualStrings("v1.0.0", lockfile.packages.items[0].tag.?);
    try testing.expectEqualStrings("logger", lockfile.packages.items[1].name);
    try testing.expect(lockfile.packages.items[1].tag == null);
}
