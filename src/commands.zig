const std = @import("std");
const package = @import("package.zig");
const fetcher = @import("fetcher.zig");

fn writeOut(comptime fmt: []const u8, args: anytype) void {
    const file = std.fs.File.stdout();
    const w = file.deprecatedWriter();
    w.print(fmt, args) catch {};
}

fn writeErr(comptime fmt: []const u8, args: anytype) void {
    const file = std.fs.File.stderr();
    const w = file.deprecatedWriter();
    w.print(fmt, args) catch {};
}

/// `asthra init` — create a new asthra.toml in the current directory
pub fn init(allocator: std.mem.Allocator) void {
    _ = allocator;
    // Check if asthra.toml already exists
    std.fs.cwd().access("asthra.toml", .{}) catch {
        // File doesn't exist — create it
        const dir_name = std.fs.path.basename(std.fs.cwd().realpathAlloc(std.heap.page_allocator, ".") catch "mypackage");
        const file = std.fs.cwd().createFile("asthra.toml", .{}) catch {
            writeErr("error: could not create asthra.toml\n", .{});
            std.process.exit(1);
        };
        defer file.close();
        const writer = file.deprecatedWriter();
        writer.print("[package]\nname = \"{s}\"\nversion = \"0.1.0\"\n\n[dependencies]\n", .{dir_name}) catch {};
        writeOut("Created asthra.toml\n", .{});
        return;
    };
    writeErr("error: asthra.toml already exists\n", .{});
    std.process.exit(1);
}

/// `asthra add <git-path> [--tag=v1.0.0] [--commit=abc123]` — add a dependency
pub fn add(allocator: std.mem.Allocator, args: []const []const u8) void {
    if (args.len < 1) {
        writeErr("Usage: asthra add <git-path> [--tag=TAG] [--commit=SHA]\n", .{});
        std.process.exit(1);
    }

    const git_url = args[0];
    var tag: ?[]const u8 = null;
    var commit: ?[]const u8 = null;
    var version: ?[]const u8 = null;

    var i: usize = 1;
    while (i < args.len) : (i += 1) {
        if (std.mem.startsWith(u8, args[i], "--tag=")) {
            tag = args[i][6..];
        } else if (std.mem.startsWith(u8, args[i], "--commit=")) {
            commit = args[i][9..];
        } else if (std.mem.startsWith(u8, args[i], "--version=")) {
            version = args[i]["--version=".len..];
        }
    }

    // Resolve version range to tag+commit
    if (version != null and tag == null and commit == null) {
        const resolved = fetcher.resolveVersionRange(allocator, git_url, version.?) catch {
            writeErr("error: could not resolve version '{s}' for {s}\n", .{ version.?, git_url });
            std.process.exit(1);
        };
        tag = resolved.tag;
        commit = resolved.commit;
    }

    // Resolve tag to commit if needed
    if (tag != null and commit == null) {
        const resolved = fetcher.resolveTagToCommit(allocator, git_url, tag.?) catch {
            writeErr("error: could not resolve tag '{s}' for {s}\n", .{ tag.?, git_url });
            std.process.exit(1);
        };
        commit = resolved;
    }

    if (commit == null) {
        writeErr("error: must specify --tag or --commit\n", .{});
        std.process.exit(1);
    }

    // Extract repo name from URL (last segment)
    const dep_name = extractRepoName(git_url);

    // Read existing asthra.toml and append dependency
    const existing = std.fs.cwd().readFileAlloc(allocator, "asthra.toml", 1024 * 1024) catch {
        writeErr("error: asthra.toml not found. Run 'asthra init' first.\n", .{});
        std.process.exit(1);
    };
    defer allocator.free(existing);

    const file = std.fs.cwd().createFile("asthra.toml", .{}) catch {
        writeErr("error: could not write asthra.toml\n", .{});
        std.process.exit(1);
    };
    defer file.close();
    const writer = file.deprecatedWriter();

    // Write existing content
    writer.writeAll(existing) catch {};

    // Append new dependency
    if (tag != null) {
        writer.print("{s} = {{ git = \"{s}\", tag = \"{s}\" }}\n", .{ dep_name, git_url, tag.? }) catch {};
    } else {
        writer.print("{s} = {{ git = \"{s}\", commit = \"{s}\" }}\n", .{ dep_name, git_url, commit.? }) catch {};
    }

    writeOut("Added dependency: {s}\n", .{dep_name});

    // Fetch the package
    const cache_path = fetcher.fetchPackage(allocator, git_url, commit.?) catch {
        writeErr("error: failed to fetch {s}\n", .{git_url});
        std.process.exit(1);
    };
    defer allocator.free(cache_path);
    writeOut("Fetched {s} -> {s}\n", .{ git_url, cache_path });

    // Update lockfile
    updateLockfile(allocator, git_url, dep_name, commit.?, tag);
}

/// `asthra install` — fetch all dependencies from asthra.toml
pub fn install(allocator: std.mem.Allocator) void {
    var manifest = package.PackageManifest.parseFromFile(allocator, "asthra.toml") catch {
        writeErr("error: could not read asthra.toml\n", .{});
        std.process.exit(1);
    };
    defer manifest.deinit();

    if (manifest.dependencies.items.len == 0) {
        writeOut("No dependencies to install.\n", .{});
        return;
    }

    // Try to read existing lockfile
    var lockfile = package.Lockfile.parseFromFile(allocator, "asthra.lock") catch null;
    defer if (lockfile) |*lf| lf.deinit();

    var new_lockfile = package.Lockfile{
        .packages = std.ArrayList(package.ResolvedDep){},
        .allocator = allocator,
    };
    defer new_lockfile.deinit();

    for (manifest.dependencies.items) |dep| {
        // Check lockfile for existing resolution
        var resolved_commit: []const u8 = undefined;
        const resolved_tag: ?[]const u8 = dep.tag;
        var needs_free = false;

        if (lockfile) |lf| {
            if (lf.findByGitUrl(dep.git_url)) |resolved| {
                resolved_commit = resolved.commit;
            } else {
                resolved_commit = resolveCommit(allocator, dep, &needs_free);
            }
        } else {
            resolved_commit = resolveCommit(allocator, dep, &needs_free);
        }

        // Fetch
        const cache_path = fetcher.fetchPackage(allocator, dep.git_url, resolved_commit) catch {
            writeErr("error: failed to fetch {s}\n", .{dep.git_url});
            std.process.exit(1);
        };
        defer allocator.free(cache_path);
        writeOut("Installed {s} -> {s}\n", .{ dep.name, cache_path });

        // Add to new lockfile
        new_lockfile.packages.append(allocator, .{
            .name = allocator.dupe(u8, dep.name) catch unreachable,
            .git_url = allocator.dupe(u8, dep.git_url) catch unreachable,
            .commit = allocator.dupe(u8, resolved_commit) catch unreachable,
            .tag = if (resolved_tag) |t| (allocator.dupe(u8, t) catch unreachable) else null,
        }) catch {};

        if (needs_free) allocator.free(resolved_commit);
    }

    // Resolve transitive dependencies (BFS)
    resolveTransitiveDeps(allocator, &new_lockfile);

    // Write lockfile
    new_lockfile.writeToFile("asthra.lock") catch {
        writeErr("error: could not write asthra.lock\n", .{});
    };
    writeOut("Wrote asthra.lock\n", .{});
}

fn resolveTransitiveDeps(allocator: std.mem.Allocator, lockfile: *package.Lockfile) void {
    // Track visited git URLs to prevent cycles and duplicates
    var visited = std.StringHashMap(void).init(allocator);
    defer visited.deinit();

    // Mark all direct deps as visited
    for (lockfile.packages.items) |pkg| {
        visited.put(pkg.git_url, {}) catch {};
    }

    // BFS: process each package's dependencies
    var idx: usize = 0;
    while (idx < lockfile.packages.items.len) : (idx += 1) {
        const pkg = lockfile.packages.items[idx];

        // Construct path to the package's asthra.toml
        const cache_path = fetcher.getCachePath(allocator, pkg.git_url, pkg.commit) catch continue;
        defer allocator.free(cache_path);

        const toml_path = std.fmt.allocPrint(allocator, "{s}/asthra.toml", .{cache_path}) catch continue;
        defer allocator.free(toml_path);

        // Try to parse the package's manifest (it may not have one)
        var dep_manifest = package.PackageManifest.parseFromFile(allocator, toml_path) catch continue;
        defer dep_manifest.deinit();

        // Process each transitive dependency
        for (dep_manifest.dependencies.items) |dep| {
            // Skip if already visited (dedup + cycle prevention)
            if (visited.contains(dep.git_url)) continue;
            visited.put(dep.git_url, {}) catch {};

            // Resolve commit
            var needs_free = false;
            const resolved_commit = resolveCommit(allocator, dep, &needs_free);
            const resolved_tag: ?[]const u8 = dep.tag;

            // Fetch
            const dep_cache = fetcher.fetchPackage(allocator, dep.git_url, resolved_commit) catch {
                writeErr("error: failed to fetch transitive dep {s}\n", .{dep.git_url});
                continue;
            };
            allocator.free(dep_cache);
            writeOut("Installed (transitive) {s}\n", .{dep.name});

            // Add to lockfile
            lockfile.packages.append(allocator, .{
                .name = allocator.dupe(u8, dep.name) catch continue,
                .git_url = allocator.dupe(u8, dep.git_url) catch continue,
                .commit = allocator.dupe(u8, resolved_commit) catch continue,
                .tag = if (resolved_tag) |t| (allocator.dupe(u8, t) catch null) else null,
            }) catch {};

            if (needs_free) allocator.free(resolved_commit);
        }
    }
}

fn resolveCommit(allocator: std.mem.Allocator, dep: package.Dependency, needs_free: *bool) []const u8 {
    if (dep.commit) |c| {
        needs_free.* = false;
        return c;
    }
    if (dep.tag) |t| {
        needs_free.* = true;
        return fetcher.resolveTagToCommit(allocator, dep.git_url, t) catch {
            writeErr("error: could not resolve tag '{s}' for {s}\n", .{ t, dep.git_url });
            std.process.exit(1);
        };
    }
    if (dep.version) |v| {
        needs_free.* = true;
        const resolved = fetcher.resolveVersionRange(allocator, dep.git_url, v) catch {
            writeErr("error: could not resolve version '{s}' for {s}\n", .{ v, dep.git_url });
            std.process.exit(1);
        };
        return resolved.commit;
    }
    writeErr("error: dependency {s} has no tag, commit, or version\n", .{dep.name});
    std.process.exit(1);
}

fn updateLockfile(allocator: std.mem.Allocator, git_url: []const u8, name: []const u8, commit: []const u8, tag: ?[]const u8) void {
    // Read existing lockfile or create new
    var lockfile = (package.Lockfile.parseFromFile(allocator, "asthra.lock") catch null) orelse package.Lockfile{
        .packages = std.ArrayList(package.ResolvedDep){},
        .allocator = allocator,
    };
    defer lockfile.deinit();

    // Check if already present
    for (lockfile.packages.items) |pkg| {
        if (std.mem.eql(u8, pkg.git_url, git_url)) return;
    }

    lockfile.packages.append(allocator, .{
        .name = allocator.dupe(u8, name) catch return,
        .git_url = allocator.dupe(u8, git_url) catch return,
        .commit = allocator.dupe(u8, commit) catch return,
        .tag = if (tag) |t| (allocator.dupe(u8, t) catch return) else null,
    }) catch {};

    lockfile.writeToFile("asthra.lock") catch {};
}

pub fn extractRepoName(git_url: []const u8) []const u8 {
    if (std.mem.lastIndexOf(u8, git_url, "/")) |last_slash| {
        return git_url[last_slash + 1 ..];
    }
    return git_url;
}
