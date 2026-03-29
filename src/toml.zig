const std = @import("std");

pub const TomlValue = union(enum) {
    string: []const u8,
    table: std.StringHashMap([]const u8),
};

pub const TomlTable = struct {
    allocator: std.mem.Allocator,
    sections: std.StringHashMap(std.StringHashMap(TomlValue)),

    pub fn init(allocator: std.mem.Allocator) TomlTable {
        return .{
            .allocator = allocator,
            .sections = std.StringHashMap(std.StringHashMap(TomlValue)).init(allocator),
        };
    }

    pub fn deinit(self: *TomlTable) void {
        var sect_it = self.sections.iterator();
        while (sect_it.next()) |entry| {
            var val_it = entry.value_ptr.iterator();
            while (val_it.next()) |val_entry| {
                switch (val_entry.value_ptr.*) {
                    .table => |*t| t.deinit(),
                    .string => {},
                }
            }
            entry.value_ptr.deinit();
        }
        self.sections.deinit();
    }

    pub fn getSection(self: *const TomlTable, name: []const u8) ?*const std.StringHashMap(TomlValue) {
        return self.sections.getPtr(name);
    }

    pub fn getString(self: *const TomlTable, section: []const u8, key: []const u8) ?[]const u8 {
        const sect = self.sections.getPtr(section) orelse return null;
        const val = sect.get(key) orelse return null;
        return switch (val) {
            .string => |s| s,
            .table => null,
        };
    }

    pub fn getInlineTable(self: *const TomlTable, section: []const u8, key: []const u8) ?*const std.StringHashMap([]const u8) {
        const sect = self.sections.getPtr(section) orelse return null;
        const val = sect.getPtr(key) orelse return null;
        return switch (val.*) {
            .table => |*t| t,
            .string => null,
        };
    }
};

pub fn parse(allocator: std.mem.Allocator, source: []const u8) !TomlTable {
    var result = TomlTable.init(allocator);
    errdefer result.deinit();

    var current_section: []const u8 = "";
    var line_start: usize = 0;

    while (line_start < source.len) {
        // Find end of line
        var line_end = line_start;
        while (line_end < source.len and source[line_end] != '\n') : (line_end += 1) {}
        const line_raw = source[line_start..line_end];
        line_start = line_end + 1;

        // Strip comments and whitespace
        const line = stripComment(std.mem.trim(u8, line_raw, " \t\r"));
        if (line.len == 0) continue;

        // Section header: [name]
        if (line[0] == '[' and line[line.len - 1] == ']') {
            current_section = line[1 .. line.len - 1];
            if (!result.sections.contains(current_section)) {
                try result.sections.put(current_section, std.StringHashMap(TomlValue).init(allocator));
            }
            continue;
        }

        // Key = value
        if (std.mem.indexOf(u8, line, "=")) |eq_pos| {
            const key = std.mem.trim(u8, line[0..eq_pos], " \t");
            const val_raw = std.mem.trim(u8, line[eq_pos + 1 ..], " \t");

            // Ensure section exists
            if (!result.sections.contains(current_section)) {
                try result.sections.put(current_section, std.StringHashMap(TomlValue).init(allocator));
            }
            const section = result.sections.getPtr(current_section).?;

            if (val_raw.len > 0 and val_raw[0] == '{') {
                // Inline table: { k1 = "v1", k2 = "v2" }
                const table = try parseInlineTable(allocator, val_raw);
                try section.put(key, .{ .table = table });
            } else if (val_raw.len >= 2 and val_raw[0] == '"' and val_raw[val_raw.len - 1] == '"') {
                // String value: "value"
                try section.put(key, .{ .string = val_raw[1 .. val_raw.len - 1] });
            } else {
                // Bare value (treated as string)
                try section.put(key, .{ .string = val_raw });
            }
        }
    }

    return result;
}

fn parseInlineTable(allocator: std.mem.Allocator, raw: []const u8) !std.StringHashMap([]const u8) {
    var table = std.StringHashMap([]const u8).init(allocator);
    errdefer table.deinit();

    // Strip outer braces
    if (raw.len < 2) return table;
    const inner = std.mem.trim(u8, raw[1 .. raw.len - 1], " \t");
    if (inner.len == 0) return table;

    // Split by commas (simple: no nested braces/strings with commas)
    var iter = std.mem.splitScalar(u8, inner, ',');
    while (iter.next()) |pair| {
        const trimmed = std.mem.trim(u8, pair, " \t");
        if (trimmed.len == 0) continue;

        if (std.mem.indexOf(u8, trimmed, "=")) |eq_pos| {
            const k = std.mem.trim(u8, trimmed[0..eq_pos], " \t");
            const v_raw = std.mem.trim(u8, trimmed[eq_pos + 1 ..], " \t");
            // Strip quotes from value
            const v = if (v_raw.len >= 2 and v_raw[0] == '"' and v_raw[v_raw.len - 1] == '"')
                v_raw[1 .. v_raw.len - 1]
            else
                v_raw;
            try table.put(k, v);
        }
    }

    return table;
}

fn stripComment(line: []const u8) []const u8 {
    // Find # outside of quotes
    var in_quotes = false;
    for (line, 0..) |ch, i| {
        if (ch == '"') in_quotes = !in_quotes;
        if (ch == '#' and !in_quotes) {
            return std.mem.trim(u8, line[0..i], " \t");
        }
    }
    return line;
}

pub fn parseFile(allocator: std.mem.Allocator, path: []const u8) !TomlTable {
    const source = try std.fs.cwd().readFileAlloc(allocator, path, 1024 * 1024);
    defer allocator.free(source);
    return parse(allocator, source);
}

// ── Tests ──────────────────────────────────────────────────────────────────

const testing = std.testing;

test "parse basic sections and strings" {
    const source =
        \\[package]
        \\name = "myapp"
        \\version = "0.1.0"
    ;
    var table = try parse(testing.allocator, source);
    defer table.deinit();

    try testing.expectEqualStrings("myapp", table.getString("package", "name").?);
    try testing.expectEqualStrings("0.1.0", table.getString("package", "version").?);
}

test "parse inline tables" {
    const source =
        \\[dependencies]
        \\mathutils = { git = "github.com/user/mathutils", tag = "v1.0.0" }
    ;
    var table = try parse(testing.allocator, source);
    defer table.deinit();

    const dep = table.getInlineTable("dependencies", "mathutils").?;
    try testing.expectEqualStrings("github.com/user/mathutils", dep.get("git").?);
    try testing.expectEqualStrings("v1.0.0", dep.get("tag").?);
}

test "parse comments and empty lines" {
    const source =
        \\# This is a comment
        \\
        \\[package]
        \\name = "test" # inline comment
    ;
    var table = try parse(testing.allocator, source);
    defer table.deinit();

    try testing.expectEqualStrings("test", table.getString("package", "name").?);
}

test "missing key returns null" {
    const source =
        \\[package]
        \\name = "test"
    ;
    var table = try parse(testing.allocator, source);
    defer table.deinit();

    try testing.expect(table.getString("package", "missing") == null);
    try testing.expect(table.getString("missing_section", "name") == null);
}
