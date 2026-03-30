const std = @import("std");

pub const Version = struct {
    major: u32,
    minor: u32,
    patch: u32,

    pub fn parse(s: []const u8) ?Version {
        // Strip leading 'v' if present
        const input = if (s.len > 0 and s[0] == 'v') s[1..] else s;

        var parts: [3]u32 = .{ 0, 0, 0 };
        var part_idx: usize = 0;
        var start: usize = 0;

        for (input, 0..) |ch, i| {
            if (ch == '.') {
                if (part_idx >= 2) return null;
                parts[part_idx] = std.fmt.parseInt(u32, input[start..i], 10) catch return null;
                part_idx += 1;
                start = i + 1;
            } else if (ch < '0' or ch > '9') {
                // Stop at pre-release markers like -beta, +build
                break;
            }
        }

        // Parse final part
        const end = for (input[start..], start..) |ch, i| {
            if (ch < '0' or ch > '9') break i;
        } else input.len;

        if (end > start) {
            parts[part_idx] = std.fmt.parseInt(u32, input[start..end], 10) catch return null;
        }

        return .{ .major = parts[0], .minor = parts[1], .patch = parts[2] };
    }

    pub fn compare(a: Version, b: Version) std.math.Order {
        if (a.major != b.major) return std.math.order(a.major, b.major);
        if (a.minor != b.minor) return std.math.order(a.minor, b.minor);
        return std.math.order(a.patch, b.patch);
    }

    pub fn gte(self: Version, other: Version) bool {
        return self.compare(other) != .lt;
    }

    pub fn lt(self: Version, other: Version) bool {
        return self.compare(other) == .lt;
    }

    pub fn satisfies(self: Version, range: Range) bool {
        return switch (range.op) {
            .exact => self.compare(range.version) == .eq,
            .gte => self.gte(range.version),
            .caret => blk: {
                // ^1.2.3: >=1.2.3, <2.0.0 (major must match, at least minor.patch)
                if (!self.gte(range.version)) break :blk false;
                if (range.version.major == 0) {
                    // ^0.x: minor must match
                    break :blk self.major == 0 and self.minor == range.version.minor;
                }
                break :blk self.major == range.version.major;
            },
            .tilde => blk: {
                // ~1.2.3: >=1.2.3, <1.3.0 (major and minor must match)
                if (!self.gte(range.version)) break :blk false;
                break :blk self.major == range.version.major and self.minor == range.version.minor;
            },
        };
    }
};

pub const Range = struct {
    op: Op,
    version: Version,

    pub const Op = enum { caret, tilde, gte, exact };

    pub fn parse(s: []const u8) ?Range {
        if (s.len == 0) return null;

        if (s[0] == '^') {
            const v = Version.parse(s[1..]) orelse return null;
            return .{ .op = .caret, .version = v };
        } else if (s[0] == '~') {
            const v = Version.parse(s[1..]) orelse return null;
            return .{ .op = .tilde, .version = v };
        } else if (s.len >= 2 and s[0] == '>' and s[1] == '=') {
            const v = Version.parse(s[2..]) orelse return null;
            return .{ .op = .gte, .version = v };
        } else {
            const v = Version.parse(s) orelse return null;
            return .{ .op = .exact, .version = v };
        }
    }
};

// ── Tests ──────────────────────────────────────────────────────────────────

const testing = std.testing;

test "parse version" {
    const v = Version.parse("1.2.3").?;
    try testing.expectEqual(@as(u32, 1), v.major);
    try testing.expectEqual(@as(u32, 2), v.minor);
    try testing.expectEqual(@as(u32, 3), v.patch);
}

test "parse version with v prefix" {
    const v = Version.parse("v2.0.1").?;
    try testing.expectEqual(@as(u32, 2), v.major);
    try testing.expectEqual(@as(u32, 0), v.minor);
    try testing.expectEqual(@as(u32, 1), v.patch);
}

test "parse version major only" {
    const v = Version.parse("3").?;
    try testing.expectEqual(@as(u32, 3), v.major);
    try testing.expectEqual(@as(u32, 0), v.minor);
    try testing.expectEqual(@as(u32, 0), v.patch);
}

test "parse version major.minor" {
    const v = Version.parse("1.5").?;
    try testing.expectEqual(@as(u32, 1), v.major);
    try testing.expectEqual(@as(u32, 5), v.minor);
}

test "compare versions" {
    const v1 = Version.parse("1.2.3").?;
    const v2 = Version.parse("1.2.4").?;
    const v3 = Version.parse("1.2.3").?;
    try testing.expect(v1.lt(v2));
    try testing.expect(v2.gte(v1));
    try testing.expect(v1.compare(v3) == .eq);
}

test "parse caret range" {
    const r = Range.parse("^1.2.3").?;
    try testing.expect(r.op == .caret);
    try testing.expectEqual(@as(u32, 1), r.version.major);
}

test "parse tilde range" {
    const r = Range.parse("~1.2.0").?;
    try testing.expect(r.op == .tilde);
}

test "parse gte range" {
    const r = Range.parse(">=2.0.0").?;
    try testing.expect(r.op == .gte);
}

test "parse exact range" {
    const r = Range.parse("1.0.0").?;
    try testing.expect(r.op == .exact);
}

test "caret range satisfaction" {
    const range = Range.parse("^1.2.0").?;
    try testing.expect(Version.parse("1.2.0").?.satisfies(range));
    try testing.expect(Version.parse("1.9.9").?.satisfies(range));
    try testing.expect(!Version.parse("2.0.0").?.satisfies(range));
    try testing.expect(!Version.parse("1.1.0").?.satisfies(range));
}

test "tilde range satisfaction" {
    const range = Range.parse("~1.2.0").?;
    try testing.expect(Version.parse("1.2.0").?.satisfies(range));
    try testing.expect(Version.parse("1.2.9").?.satisfies(range));
    try testing.expect(!Version.parse("1.3.0").?.satisfies(range));
    try testing.expect(!Version.parse("2.0.0").?.satisfies(range));
}

test "gte range satisfaction" {
    const range = Range.parse(">=1.0.0").?;
    try testing.expect(Version.parse("1.0.0").?.satisfies(range));
    try testing.expect(Version.parse("2.5.0").?.satisfies(range));
    try testing.expect(!Version.parse("0.9.9").?.satisfies(range));
}
