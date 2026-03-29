const std = @import("std");

pub const Severity = enum {
    @"error",
    warning,
    note,

    pub fn label(self: Severity) []const u8 {
        return switch (self) {
            .@"error" => "error",
            .warning => "warning",
            .note => "note",
        };
    }
};

pub const Diagnostic = struct {
    severity: Severity,
    start: u32,
    message: []const u8,
};

pub const Diagnostics = struct {
    errors: std.ArrayList(Diagnostic) = .{},
    allocator: std.mem.Allocator,
    source: []const u8,
    filename: []const u8,

    pub fn init(allocator: std.mem.Allocator, source: []const u8, filename: []const u8) Diagnostics {
        return .{
            .allocator = allocator,
            .source = source,
            .filename = filename,
        };
    }

    pub fn deinit(self: *Diagnostics) void {
        for (self.errors.items) |err| {
            self.allocator.free(err.message);
        }
        self.errors.deinit(self.allocator);
    }

    pub fn report(self: *Diagnostics, severity: Severity, start: u32, comptime fmt: []const u8, args: anytype) void {
        const message = std.fmt.allocPrint(self.allocator, fmt, args) catch return;
        self.errors.append(self.allocator, .{
            .severity = severity,
            .start = start,
            .message = message,
        }) catch {
            self.allocator.free(message);
        };
    }

    pub fn hasErrors(self: *const Diagnostics) bool {
        for (self.errors.items) |err| {
            if (err.severity == .@"error") return true;
        }
        return false;
    }

    pub fn printAll(self: *const Diagnostics) void {
        const file = std.fs.File.stderr();
        const writer = file.deprecatedWriter();
        for (self.errors.items) |err| {
            const loc = self.getLineAndColumn(err.start);
            writer.print("{s}:{d}:{d}: {s}: {s}\n", .{
                self.filename,
                loc.line,
                loc.column,
                err.severity.label(),
                err.message,
            }) catch {};
        }
    }

    pub fn getLineAndColumn(self: *const Diagnostics, byte_offset: u32) struct { line: u32, column: u32 } {
        var line: u32 = 1;
        var col: u32 = 1;
        const end = @min(byte_offset, @as(u32, @intCast(self.source.len)));
        for (self.source[0..end]) |ch| {
            if (ch == '\n') {
                line += 1;
                col = 1;
            } else {
                col += 1;
            }
        }
        return .{ .line = line, .column = col };
    }
};

// --- Tests ---

const testing = std.testing;

test "getLineAndColumn at start of file" {
    var diag = Diagnostics.init(testing.allocator, "hello\nworld", "test.ast");
    defer diag.deinit();
    const loc = diag.getLineAndColumn(0);
    try testing.expectEqual(@as(u32, 1), loc.line);
    try testing.expectEqual(@as(u32, 1), loc.column);
}

test "getLineAndColumn middle of first line" {
    var diag = Diagnostics.init(testing.allocator, "hello\nworld", "test.ast");
    defer diag.deinit();
    const loc = diag.getLineAndColumn(3);
    try testing.expectEqual(@as(u32, 1), loc.line);
    try testing.expectEqual(@as(u32, 4), loc.column);
}

test "getLineAndColumn second line" {
    var diag = Diagnostics.init(testing.allocator, "hello\nworld", "test.ast");
    defer diag.deinit();
    const loc = diag.getLineAndColumn(6);
    try testing.expectEqual(@as(u32, 2), loc.line);
    try testing.expectEqual(@as(u32, 1), loc.column);
}

test "getLineAndColumn third line" {
    var diag = Diagnostics.init(testing.allocator, "a\nb\ncde", "test.ast");
    defer diag.deinit();
    const loc = diag.getLineAndColumn(5);
    try testing.expectEqual(@as(u32, 3), loc.line);
    try testing.expectEqual(@as(u32, 2), loc.column);
}

test "report and hasErrors" {
    var diag = Diagnostics.init(testing.allocator, "test", "test.ast");
    defer diag.deinit();
    try testing.expect(!diag.hasErrors());
    diag.report(.@"error", 0, "something went wrong: {s}", .{"details"});
    try testing.expect(diag.hasErrors());
    try testing.expectEqual(@as(usize, 1), diag.errors.items.len);
}

test "warnings do not count as errors" {
    var diag = Diagnostics.init(testing.allocator, "test", "test.ast");
    defer diag.deinit();
    diag.report(.warning, 0, "just a warning", .{});
    try testing.expect(!diag.hasErrors());
}

test "severity labels" {
    try testing.expectEqualStrings("error", Severity.@"error".label());
    try testing.expectEqualStrings("warning", Severity.warning.label());
    try testing.expectEqualStrings("note", Severity.note.label());
}
