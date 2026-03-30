const std = @import("std");
const testing = std.testing;
const Parser = @import("parser.zig").Parser;
const Ast = @import("ast.zig").Ast;
const Diagnostics = @import("diagnostics.zig").Diagnostics;

fn testParse(source: []const u8) !struct { ast: Ast, diag: Diagnostics } {
    var ast = Ast.init(testing.allocator);
    var diag = Diagnostics.init(testing.allocator, source, "test.ast");
    var parser = Parser.init(source, &ast, &diag);
    parser.parse() catch {};
    return .{ .ast = ast, .diag = diag };
}

test "parse if/else statement" {
    var result = try testParse("package main;\npub fn main() -> void { if x > 0 { return; } else { return; } }");
    defer result.diag.deinit();
    try testing.expect(!result.diag.hasErrors());
    const decl = result.ast.program.decls.items[0];
    switch (decl.decl) {
        .function => |f| {
            switch (f.body.stmts.items[0]) {
                .if_stmt => |if_s| {
                    try testing.expect(if_s.else_block != null);
                },
                else => return error.TestUnexpectedResult,
            }
        },
    }
}

test "parse for loop" {
    var result = try testParse("package main;\npub fn main() -> void { for i in range(10) { log(i); } return; }");
    defer result.diag.deinit();
    try testing.expect(!result.diag.hasErrors());
    const decl = result.ast.program.decls.items[0];
    switch (decl.decl) {
        .function => |f| {
            switch (f.body.stmts.items[0]) {
                .for_stmt => |for_s| {
                    try testing.expectEqualStrings("i", for_s.iter_var);
                },
                else => return error.TestUnexpectedResult,
            }
        },
    }
}

test "parse break statement" {
    var result = try testParse("package main;\npub fn main() -> void { for i in range(10) { break; } return; }");
    defer result.diag.deinit();
    try testing.expect(!result.diag.hasErrors());
    const decl = result.ast.program.decls.items[0];
    switch (decl.decl) {
        .function => |f| {
            switch (f.body.stmts.items[0]) {
                .for_stmt => |for_s| {
                    try testing.expectEqual(Ast.Stmt.break_stmt, for_s.body.stmts.items[0]);
                },
                else => return error.TestUnexpectedResult,
            }
        },
    }
}

test "parse continue statement" {
    var result = try testParse("package main;\npub fn main() -> void { for i in range(10) { continue; } return; }");
    defer result.diag.deinit();
    try testing.expect(!result.diag.hasErrors());
    const decl = result.ast.program.decls.items[0];
    switch (decl.decl) {
        .function => |f| {
            switch (f.body.stmts.items[0]) {
                .for_stmt => |for_s| {
                    try testing.expectEqual(Ast.Stmt.continue_stmt, for_s.body.stmts.items[0]);
                },
                else => return error.TestUnexpectedResult,
            }
        },
    }
}

test "parse while loop" {
    var result = try testParse("package main;\npub fn main() -> void { let mut x: i32 = 0; while x < 5 { x = x + 1; } return; }");
    defer result.diag.deinit();
    try testing.expect(!result.diag.hasErrors());
    const decl = result.ast.program.decls.items[0];
    switch (decl.decl) {
        .function => |f| {
            switch (f.body.stmts.items[1]) {
                .while_stmt => |ws| {
                    try testing.expect(ws.body.stmts.items.len > 0);
                },
                else => return error.TestUnexpectedResult,
            }
        },
    }
}

test "parse while true with break" {
    var result = try testParse("package main;\npub fn main() -> void { while true { break; } return; }");
    defer result.diag.deinit();
    try testing.expect(!result.diag.hasErrors());
    const decl = result.ast.program.decls.items[0];
    switch (decl.decl) {
        .function => |f| {
            switch (f.body.stmts.items[0]) {
                .while_stmt => |ws| {
                    try testing.expectEqual(@as(usize, 1), ws.body.stmts.items.len);
                    try testing.expectEqual(Ast.Stmt.break_stmt, ws.body.stmts.items[0]);
                },
                else => return error.TestUnexpectedResult,
            }
        },
    }
}

test "parse if let with Option.Some" {
    var result = try testParse("package main;\npub fn main() -> void { let a: Option<i32> = Option.Some(42); if let Option.Some(val) = a { log(val); } else { log(0); } return; }");
    defer result.diag.deinit();
    try testing.expect(!result.diag.hasErrors());
    const decl = result.ast.program.decls.items[0];
    switch (decl.decl) {
        .function => |f| {
            // Second statement should be if_let_stmt
            switch (f.body.stmts.items[1]) {
                .if_let_stmt => |il| {
                    // Pattern should be enum_pattern with Option.Some
                    switch (il.pattern) {
                        .enum_pattern => |ep| {
                            try testing.expectEqualStrings("Option", ep.enum_name);
                            try testing.expectEqualStrings("Some", ep.variant_name);
                            try testing.expectEqual(@as(usize, 1), ep.bindings.items.len);
                            try testing.expectEqualStrings("val", ep.bindings.items[0]);
                        },
                        else => return error.TestUnexpectedResult,
                    }
                    // Should have then and else blocks
                    try testing.expect(il.then_block.stmts.items.len > 0);
                    try testing.expect(il.else_block != null);
                },
                else => return error.TestUnexpectedResult,
            }
        },
    }
}

test "parse if let without else block" {
    var result = try testParse("package main;\npub fn main() -> void { let a: Option<i32> = Option.Some(42); if let Option.Some(val) = a { log(val); } return; }");
    defer result.diag.deinit();
    try testing.expect(!result.diag.hasErrors());
    const decl = result.ast.program.decls.items[0];
    switch (decl.decl) {
        .function => |f| {
            switch (f.body.stmts.items[1]) {
                .if_let_stmt => |il| {
                    // Should have then block but no else block
                    try testing.expect(il.then_block.stmts.items.len > 0);
                    try testing.expect(il.else_block == null);
                },
                else => return error.TestUnexpectedResult,
            }
        },
    }
}

test "parse if let with Result.Ok" {
    var result = try testParse("package main;\npub fn main() -> void { let r: Result<i32, string> = Result.Ok(100); if let Result.Ok(v) = r { log(v); } else { log(0); } return; }");
    defer result.diag.deinit();
    try testing.expect(!result.diag.hasErrors());
    const decl = result.ast.program.decls.items[0];
    switch (decl.decl) {
        .function => |f| {
            switch (f.body.stmts.items[1]) {
                .if_let_stmt => |il| {
                    switch (il.pattern) {
                        .enum_pattern => |ep| {
                            try testing.expectEqualStrings("Result", ep.enum_name);
                            try testing.expectEqualStrings("Ok", ep.variant_name);
                            try testing.expectEqual(@as(usize, 1), ep.bindings.items.len);
                            try testing.expectEqualStrings("v", ep.bindings.items[0]);
                        },
                        else => return error.TestUnexpectedResult,
                    }
                },
                else => return error.TestUnexpectedResult,
            }
        },
    }
}

test "parse wildcard pattern in match" {
    var result = try testParse(
        \\package main;
        \\pub enum Color { pub Red, pub Green, pub Blue }
        \\pub fn main() -> void {
        \\    let c: Color = Color.Red;
        \\    match c {
        \\        Color.Red => { log("red"); }
        \\        _ => { log("other"); }
        \\    }
        \\    return;
        \\}
    );
    defer result.diag.deinit();
    try testing.expect(!result.diag.hasErrors());
    // Find the match statement
    const decl = result.ast.program.decls.items[1]; // fn main
    switch (decl.decl) {
        .function => |f| {
            switch (f.body.stmts.items[1]) { // match stmt
                .match_stmt => |ms| {
                    try testing.expectEqual(@as(usize, 2), ms.arms.items.len);
                    // First arm is enum pattern
                    switch (ms.arms.items[0].pattern) {
                        .enum_pattern => |ep| {
                            try testing.expectEqualStrings("Color", ep.enum_name);
                            try testing.expectEqualStrings("Red", ep.variant_name);
                        },
                        else => return error.TestUnexpectedResult,
                    }
                    // Second arm is wildcard
                    switch (ms.arms.items[1].pattern) {
                        .wildcard => {},
                        else => return error.TestUnexpectedResult,
                    }
                },
                else => return error.TestUnexpectedResult,
            }
        },
        else => return error.TestUnexpectedResult,
    }
}

test "parse identifier catch-all pattern in match" {
    var result = try testParse(
        \\package main;
        \\pub enum Color { pub Red, pub Green, pub Blue }
        \\pub fn main() -> void {
        \\    let c: Color = Color.Red;
        \\    match c {
        \\        Color.Red => { log("red"); }
        \\        other => { log("other"); }
        \\    }
        \\    return;
        \\}
    );
    defer result.diag.deinit();
    try testing.expect(!result.diag.hasErrors());
    const decl = result.ast.program.decls.items[1];
    switch (decl.decl) {
        .function => |f| {
            switch (f.body.stmts.items[1]) {
                .match_stmt => |ms| {
                    try testing.expectEqual(@as(usize, 2), ms.arms.items.len);
                    // Second arm is identifier catch-all
                    switch (ms.arms.items[1].pattern) {
                        .identifier => |name| {
                            try testing.expectEqualStrings("other", name);
                        },
                        else => return error.TestUnexpectedResult,
                    }
                },
                else => return error.TestUnexpectedResult,
            }
        },
        else => return error.TestUnexpectedResult,
    }
}

test "parse tuple pattern in match" {
    var result = try testParse(
        \\package main;
        \\pub fn main() -> void {
        \\    let p: (i32, i32) = (1, 2);
        \\    match p {
        \\        (x, y) => { log(x); }
        \\    }
        \\    return;
        \\}
    );
    defer result.diag.deinit();
    try testing.expect(!result.diag.hasErrors());
    const decl = result.ast.program.decls.items[0];
    switch (decl.decl) {
        .function => |f| {
            switch (f.body.stmts.items[1]) { // match stmt
                .match_stmt => |ms| {
                    try testing.expectEqual(@as(usize, 1), ms.arms.items.len);
                    switch (ms.arms.items[0].pattern) {
                        .tuple_pattern => |tp| {
                            try testing.expectEqual(@as(usize, 2), tp.elements.items.len);
                            switch (tp.elements.items[0]) {
                                .identifier => |name| try testing.expectEqualStrings("x", name),
                                else => return error.TestUnexpectedResult,
                            }
                            switch (tp.elements.items[1]) {
                                .identifier => |name| try testing.expectEqualStrings("y", name),
                                else => return error.TestUnexpectedResult,
                            }
                        },
                        else => return error.TestUnexpectedResult,
                    }
                },
                else => return error.TestUnexpectedResult,
            }
        },
        else => return error.TestUnexpectedResult,
    }
}

test "parse tuple pattern with wildcard sub-pattern" {
    var result = try testParse(
        \\package main;
        \\pub fn main() -> void {
        \\    let p: (i32, bool) = (1, true);
        \\    match p {
        \\        (val, _) => { log(val); }
        \\    }
        \\    return;
        \\}
    );
    defer result.diag.deinit();
    try testing.expect(!result.diag.hasErrors());
    const decl = result.ast.program.decls.items[0];
    switch (decl.decl) {
        .function => |f| {
            switch (f.body.stmts.items[1]) {
                .match_stmt => |ms| {
                    try testing.expectEqual(@as(usize, 1), ms.arms.items.len);
                    switch (ms.arms.items[0].pattern) {
                        .tuple_pattern => |tp| {
                            try testing.expectEqual(@as(usize, 2), tp.elements.items.len);
                            switch (tp.elements.items[0]) {
                                .identifier => |name| try testing.expectEqualStrings("val", name),
                                else => return error.TestUnexpectedResult,
                            }
                            switch (tp.elements.items[1]) {
                                .wildcard => {},
                                else => return error.TestUnexpectedResult,
                            }
                        },
                        else => return error.TestUnexpectedResult,
                    }
                },
                else => return error.TestUnexpectedResult,
            }
        },
        else => return error.TestUnexpectedResult,
    }
}

test "parse triple-element tuple pattern" {
    var result = try testParse(
        \\package main;
        \\pub fn main() -> void {
        \\    let t: (i32, i32, i32) = (1, 2, 3);
        \\    match t {
        \\        (a, b, c) => { log(a); }
        \\    }
        \\    return;
        \\}
    );
    defer result.diag.deinit();
    try testing.expect(!result.diag.hasErrors());
    const decl = result.ast.program.decls.items[0];
    switch (decl.decl) {
        .function => |f| {
            switch (f.body.stmts.items[1]) {
                .match_stmt => |ms| {
                    switch (ms.arms.items[0].pattern) {
                        .tuple_pattern => |tp| {
                            try testing.expectEqual(@as(usize, 3), tp.elements.items.len);
                            switch (tp.elements.items[0]) {
                                .identifier => |name| try testing.expectEqualStrings("a", name),
                                else => return error.TestUnexpectedResult,
                            }
                            switch (tp.elements.items[1]) {
                                .identifier => |name| try testing.expectEqualStrings("b", name),
                                else => return error.TestUnexpectedResult,
                            }
                            switch (tp.elements.items[2]) {
                                .identifier => |name| try testing.expectEqualStrings("c", name),
                                else => return error.TestUnexpectedResult,
                            }
                        },
                        else => return error.TestUnexpectedResult,
                    }
                },
                else => return error.TestUnexpectedResult,
            }
        },
        else => return error.TestUnexpectedResult,
    }
}
