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

            // Print source context: the offending line and a caret pointing to the error
            if (self.getSourceLine(err.start)) |line_info| {
                writer.print("{s}\n", .{line_info.line}) catch {};
                // Print spaces up to the column, then a caret
                var col: u32 = 1;
                while (col < loc.column) : (col += 1) {
                    writer.print(" ", .{}) catch {};
                }
                writer.print("^\n", .{}) catch {};
            }
        }
    }

    /// Extract the source line containing the given byte offset.
    pub fn getSourceLine(self: *const Diagnostics, byte_offset: u32) ?struct { line: []const u8 } {
        if (byte_offset >= self.source.len) return null;

        // Find start of line
        var line_start: usize = byte_offset;
        while (line_start > 0 and self.source[line_start - 1] != '\n') {
            line_start -= 1;
        }

        // Find end of line
        var line_end: usize = byte_offset;
        while (line_end < self.source.len and self.source[line_end] != '\n') {
            line_end += 1;
        }

        return .{ .line = self.source[line_start..line_end] };
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

test "getSourceLine extracts correct line" {
    var diag = Diagnostics.init(testing.allocator, "hello\nworld\nfoo", "test.ast");
    defer diag.deinit();
    // Offset 0 is in first line
    const line1 = diag.getSourceLine(0).?;
    try testing.expectEqualStrings("hello", line1.line);
    // Offset 6 is in second line (w of "world")
    const line2 = diag.getSourceLine(6).?;
    try testing.expectEqualStrings("world", line2.line);
    // Offset 12 is in third line (f of "foo")
    const line3 = diag.getSourceLine(12).?;
    try testing.expectEqualStrings("foo", line3.line);
}

test "getSourceLine returns null for out of bounds" {
    var diag = Diagnostics.init(testing.allocator, "hello", "test.ast");
    defer diag.deinit();
    try testing.expect(diag.getSourceLine(100) == null);
}
