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

test "parse empty main function" {
    var result = try testParse("package main;\npub fn main() -> void { return; }");
    defer result.diag.deinit();
    try testing.expect(!result.diag.hasErrors());
    try testing.expectEqualStrings("main", result.ast.program.package_name);
    try testing.expectEqual(@as(usize, 1), result.ast.program.decls.items.len);
    const decl = result.ast.program.decls.items[0];
    try testing.expectEqual(Ast.Visibility.public, decl.visibility);
    switch (decl.decl) {
        .function => |f| try testing.expectEqualStrings("main", f.name),
    }
}

test "parse function with parameters" {
    var result = try testParse("package main;\npub fn add(a: i32, b: i32) -> i32 { return a; }");
    defer result.diag.deinit();
    try testing.expect(!result.diag.hasErrors());
    const decl = result.ast.program.decls.items[0];
    switch (decl.decl) {
        .function => |f| {
            try testing.expectEqualStrings("add", f.name);
            try testing.expectEqual(@as(usize, 2), f.params.items.len);
            try testing.expectEqualStrings("a", f.params.items[0].name);
            try testing.expectEqualStrings("b", f.params.items[1].name);
        },
    }
}

test "parse variable declaration" {
    var result = try testParse("package main;\npub fn main() -> void { let x: i32 = 42; return; }");
    defer result.diag.deinit();
    try testing.expect(!result.diag.hasErrors());
    const decl = result.ast.program.decls.items[0];
    switch (decl.decl) {
        .function => |f| {
            try testing.expectEqual(@as(usize, 2), f.body.stmts.items.len);
            switch (f.body.stmts.items[0]) {
                .var_decl => |vd| {
                    try testing.expectEqualStrings("x", vd.name);
                    try testing.expect(!vd.is_mutable);
                },
                else => return error.TestUnexpectedResult,
            }
        },
    }
}

test "parse mutable variable declaration" {
    var result = try testParse("package main;\npub fn main() -> void { let mut y: i32 = 0; return; }");
    defer result.diag.deinit();
    try testing.expect(!result.diag.hasErrors());
    const decl = result.ast.program.decls.items[0];
    switch (decl.decl) {
        .function => |f| {
            switch (f.body.stmts.items[0]) {
                .var_decl => |vd| {
                    try testing.expectEqualStrings("y", vd.name);
                    try testing.expect(vd.is_mutable);
                },
                else => return error.TestUnexpectedResult,
            }
        },
    }
}

test "parse type-inferred variable declaration" {
    var result = try testParse("package main;\npub fn main() -> void { let x = 42; return; }");
    defer result.diag.deinit();
    try testing.expect(!result.diag.hasErrors());
    const decl = result.ast.program.decls.items[0];
    switch (decl.decl) {
        .function => |f| {
            try testing.expectEqual(@as(usize, 2), f.body.stmts.items.len);
            switch (f.body.stmts.items[0]) {
                .var_decl => |vd| {
                    try testing.expectEqualStrings("x", vd.name);
                    try testing.expect(!vd.is_mutable);
                    try testing.expect(vd.type_expr == .inferred);
                },
                else => return error.TestUnexpectedResult,
            }
        },
    }
}

test "parse mutable type-inferred variable declaration" {
    var result = try testParse("package main;\npub fn main() -> void { let mut counter = 0; return; }");
    defer result.diag.deinit();
    try testing.expect(!result.diag.hasErrors());
    const decl = result.ast.program.decls.items[0];
    switch (decl.decl) {
        .function => |f| {
            switch (f.body.stmts.items[0]) {
                .var_decl => |vd| {
                    try testing.expectEqualStrings("counter", vd.name);
                    try testing.expect(vd.is_mutable);
                    try testing.expect(vd.type_expr == .inferred);
                },
                else => return error.TestUnexpectedResult,
            }
        },
    }
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

test "parse arithmetic expression precedence" {
    var result = try testParse("package main;\npub fn main() -> void { let z: i32 = 1 + 2 * 3; return; }");
    defer result.diag.deinit();
    try testing.expect(!result.diag.hasErrors());
    const decl = result.ast.program.decls.items[0];
    switch (decl.decl) {
        .function => |f| {
            switch (f.body.stmts.items[0]) {
                .var_decl => |vd| {
                    // The init expr should be a binary add at the top
                    const expr = result.ast.getExpr(vd.init_expr);
                    switch (expr) {
                        .binary => |bin| {
                            try testing.expectEqual(Ast.BinaryOp.add, bin.op);
                            // RHS should be mul
                            const rhs = result.ast.getExpr(bin.rhs);
                            switch (rhs) {
                                .binary => |rhs_bin| try testing.expectEqual(Ast.BinaryOp.mul, rhs_bin.op),
                                else => return error.TestUnexpectedResult,
                            }
                        },
                        else => return error.TestUnexpectedResult,
                    }
                },
                else => return error.TestUnexpectedResult,
            }
        },
    }
}

test "parse error on missing semicolon" {
    var result = try testParse("package main;\npub fn main() -> void { return }");
    defer result.diag.deinit();
    try testing.expect(result.diag.hasErrors());
}

test "parse error on missing visibility" {
    var result = try testParse("package main;\nfn main() -> void { return; }");
    defer result.diag.deinit();
    try testing.expect(result.diag.hasErrors());
}

test "parse multiple functions" {
    var result = try testParse("package main;\npub fn foo() -> void { return; }\npriv fn bar() -> i32 { return 0; }");
    defer result.diag.deinit();
    try testing.expect(!result.diag.hasErrors());
    try testing.expectEqual(@as(usize, 2), result.ast.program.decls.items.len);
    try testing.expectEqual(Ast.Visibility.public, result.ast.program.decls.items[0].visibility);
    try testing.expectEqual(Ast.Visibility.private, result.ast.program.decls.items[1].visibility);
}

test "parseIntLiteral decimal" {
    const parseIntLiteral = @import("parser.zig").parseIntLiteral;
    try testing.expectEqual(@as(i64, 42), parseIntLiteral("42"));
    try testing.expectEqual(@as(i64, 0), parseIntLiteral("0"));
    try testing.expectEqual(@as(i64, 12345), parseIntLiteral("12345"));
}

test "parseIntLiteral hex" {
    const parseIntLiteral = @import("parser.zig").parseIntLiteral;
    try testing.expectEqual(@as(i64, 255), parseIntLiteral("0xFF"));
    try testing.expectEqual(@as(i64, 255), parseIntLiteral("0XFF"));
    try testing.expectEqual(@as(i64, 0x1A), parseIntLiteral("0x1A"));
}

test "parseIntLiteral binary" {
    const parseIntLiteral = @import("parser.zig").parseIntLiteral;
    try testing.expectEqual(@as(i64, 10), parseIntLiteral("0b1010"));
    try testing.expectEqual(@as(i64, 10), parseIntLiteral("0B1010"));
}

test "parseIntLiteral octal" {
    const parseIntLiteral = @import("parser.zig").parseIntLiteral;
    try testing.expectEqual(@as(i64, 63), parseIntLiteral("0o77"));
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

test "parse float literal" {
    var result = try testParse("package main;\npub fn main() -> void { let x: f64 = 3.14; return; }");
    defer result.diag.deinit();
    try testing.expect(!result.diag.hasErrors());
    const decl = result.ast.program.decls.items[0];
    switch (decl.decl) {
        .function => |f| {
            switch (f.body.stmts.items[0]) {
                .var_decl => |vd| {
                    try testing.expectEqualStrings("x", vd.name);
                    try testing.expectEqual(Ast.BuiltinType.f64_type, vd.type_expr.builtin);
                    const expr = result.ast.getExpr(vd.init_expr);
                    switch (expr) {
                        .float_literal => |val| try testing.expectApproxEqAbs(3.14, val, 0.001),
                        else => return error.TestUnexpectedResult,
                    }
                },
                else => return error.TestUnexpectedResult,
            }
        },
    }
}

test "parse struct declaration" {
    var result = try testParse("package main;\npub struct Point { pub x: i32, pub y: i32 }\npub fn main() -> void { return; }");
    defer result.diag.deinit();
    try testing.expect(!result.diag.hasErrors());
    try testing.expectEqual(@as(usize, 2), result.ast.program.decls.items.len);
    const decl = result.ast.program.decls.items[0];
    switch (decl.decl) {
        .struct_decl => |sd| {
            try testing.expectEqualStrings("Point", sd.name);
            try testing.expectEqual(@as(usize, 2), sd.fields.items.len);
            try testing.expectEqualStrings("x", sd.fields.items[0].name);
            try testing.expectEqualStrings("y", sd.fields.items[1].name);
        },
        else => return error.TestUnexpectedResult,
    }
}

test "parse struct literal" {
    var result = try testParse("package main;\npub fn main() -> void { let p: Point = Point { x: 1, y: 2 }; return; }");
    defer result.diag.deinit();
    try testing.expect(!result.diag.hasErrors());
    const decl = result.ast.program.decls.items[0];
    switch (decl.decl) {
        .function => |f| {
            switch (f.body.stmts.items[0]) {
                .var_decl => |vd| {
                    const expr = result.ast.getExpr(vd.init_expr);
                    switch (expr) {
                        .struct_literal => |sl| {
                            try testing.expectEqualStrings("Point", sl.name);
                            try testing.expectEqual(@as(usize, 2), sl.field_inits.items.len);
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

test "parse type conversion call" {
    var result = try testParse("package main;\npub fn main() -> void { let x: f64 = f64(42); return; }");
    defer result.diag.deinit();
    try testing.expect(!result.diag.hasErrors());
    const decl = result.ast.program.decls.items[0];
    switch (decl.decl) {
        .function => |f| {
            switch (f.body.stmts.items[0]) {
                .var_decl => |vd| {
                    const expr = result.ast.getExpr(vd.init_expr);
                    switch (expr) {
                        .call => |call| {
                            const callee = result.ast.getExpr(call.callee);
                            switch (callee) {
                                .identifier => |name| try testing.expectEqualStrings("f64", name),
                                else => return error.TestUnexpectedResult,
                            }
                        },
                        else => return error.TestUnexpectedResult,
                    }
                },
                else => return error.TestUnexpectedResult,
            }
        },
    }
}

test "parse array type and literal" {
    var result = try testParse("package main;\npub fn main() -> void { let a: [3]i32 = [1, 2, 3]; return; }");
    defer result.diag.deinit();
    try testing.expect(!result.diag.hasErrors());
    const decl = result.ast.program.decls.items[0];
    switch (decl.decl) {
        .function => |f| {
            switch (f.body.stmts.items[0]) {
                .var_decl => |vd| {
                    try testing.expectEqualStrings("a", vd.name);
                    switch (vd.type_expr) {
                        .array_type => |arr| {
                            try testing.expectEqual(@as(u32, 3), arr.size);
                            try testing.expectEqual(Ast.BuiltinType.i32_type, arr.element_type.builtin);
                        },
                        else => return error.TestUnexpectedResult,
                    }
                    const expr = result.ast.getExpr(vd.init_expr);
                    switch (expr) {
                        .array_literal => |al| {
                            try testing.expectEqual(@as(usize, 3), al.elements.items.len);
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

test "parse indexed assignment" {
    var result = try testParse("package main;\npub fn main() -> void { let mut a: [2]i32 = [1, 2]; a[0] = 99; return; }");
    defer result.diag.deinit();
    try testing.expect(!result.diag.hasErrors());
    const decl = result.ast.program.decls.items[0];
    switch (decl.decl) {
        .function => |f| {
            switch (f.body.stmts.items[1]) {
                .assign => |assign| {
                    try testing.expectEqualStrings("a", assign.target);
                    try testing.expect(assign.target_index != null);
                },
                else => return error.TestUnexpectedResult,
            }
        },
        else => return error.TestUnexpectedResult,
    }
}

test "parse Option type and constructors" {
    var result = try testParse("package main;\npub fn main() -> void { let a: Option<i32> = Option.Some(42); return; }");
    defer result.diag.deinit();
    try testing.expect(!result.diag.hasErrors());
    const decl = result.ast.program.decls.items[0];
    switch (decl.decl) {
        .function => |f| {
            switch (f.body.stmts.items[0]) {
                .var_decl => |vd| {
                    try testing.expectEqualStrings("a", vd.name);
                    switch (vd.type_expr) {
                        .option_type => |opt| {
                            try testing.expectEqual(Ast.BuiltinType.i32_type, opt.inner_type.builtin);
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

test "parse Result type and constructors" {
    var result = try testParse("package main;\npub fn main() -> void { let a: Result<i32, string> = Result.Ok(42); return; }");
    defer result.diag.deinit();
    try testing.expect(!result.diag.hasErrors());
    const decl = result.ast.program.decls.items[0];
    switch (decl.decl) {
        .function => |f| {
            switch (f.body.stmts.items[0]) {
                .var_decl => |vd| {
                    try testing.expectEqualStrings("a", vd.name);
                    switch (vd.type_expr) {
                        .result_type => |res| {
                            try testing.expectEqual(Ast.BuiltinType.i32_type, res.ok_type.builtin);
                            try testing.expectEqual(Ast.BuiltinType.string_type, res.err_type.builtin);
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

test "parse Result return type" {
    var result = try testParse("package main;\npub fn divide(a: i32, b: i32) -> Result<i32, string> { return Result.Ok(a); }");
    defer result.diag.deinit();
    try testing.expect(!result.diag.hasErrors());
    const decl = result.ast.program.decls.items[0];
    switch (decl.decl) {
        .function => |f| {
            try testing.expectEqualStrings("divide", f.name);
            switch (f.return_type) {
                .result_type => |res| {
                    try testing.expectEqual(Ast.BuiltinType.i32_type, res.ok_type.builtin);
                    try testing.expectEqual(Ast.BuiltinType.string_type, res.err_type.builtin);
                },
                else => return error.TestUnexpectedResult,
            }
        },
        else => return error.TestUnexpectedResult,
    }
}

test "parse Option return type" {
    var result = try testParse("package main;\npub fn find(x: i32) -> Option<i32> { return Option.None(); }");
    defer result.diag.deinit();
    try testing.expect(!result.diag.hasErrors());
    const decl = result.ast.program.decls.items[0];
    switch (decl.decl) {
        .function => |f| {
            try testing.expectEqualStrings("find", f.name);
            switch (f.return_type) {
                .option_type => |opt| {
                    try testing.expectEqual(Ast.BuiltinType.i32_type, opt.inner_type.builtin);
                },
                else => return error.TestUnexpectedResult,
            }
        },
        else => return error.TestUnexpectedResult,
    }
}

test "parse tuple type" {
    var result = try testParse("package main;\npub fn main() -> void { let p: (i32, f64) = (1, 2.0); return; }");
    defer result.diag.deinit();
    try testing.expect(!result.diag.hasErrors());
    const decl = result.ast.program.decls.items[0];
    switch (decl.decl) {
        .function => |f| {
            switch (f.body.stmts.items[0]) {
                .var_decl => |vd| {
                    try testing.expectEqualStrings("p", vd.name);
                    switch (vd.type_expr) {
                        .tuple_type => |tt| {
                            try testing.expectEqual(@as(usize, 2), tt.element_types.items.len);
                            try testing.expectEqual(Ast.BuiltinType.i32_type, tt.element_types.items[0].builtin);
                            try testing.expectEqual(Ast.BuiltinType.f64_type, tt.element_types.items[1].builtin);
                        },
                        else => return error.TestUnexpectedResult,
                    }
                    // Check the init is a tuple literal
                    const expr = result.ast.getExpr(vd.init_expr);
                    switch (expr) {
                        .tuple_literal => |tl| {
                            try testing.expectEqual(@as(usize, 2), tl.elements.items.len);
                        },
                        else => return error.TestUnexpectedResult,
                    }
                },
                else => return error.TestUnexpectedResult,
            }
        },
    }
}

test "parse tuple return type" {
    var result = try testParse("package main;\npub fn swap(a: i32, b: i32) -> (i32, i32) { return (b, a); }");
    defer result.diag.deinit();
    try testing.expect(!result.diag.hasErrors());
    const decl = result.ast.program.decls.items[0];
    switch (decl.decl) {
        .function => |f| {
            try testing.expectEqualStrings("swap", f.name);
            switch (f.return_type) {
                .tuple_type => |tt| {
                    try testing.expectEqual(@as(usize, 2), tt.element_types.items.len);
                    try testing.expectEqual(Ast.BuiltinType.i32_type, tt.element_types.items[0].builtin);
                    try testing.expectEqual(Ast.BuiltinType.i32_type, tt.element_types.items[1].builtin);
                },
                else => return error.TestUnexpectedResult,
            }
        },
    }
}

test "parse tuple element access" {
    var result = try testParse("package main;\npub fn main() -> void { let p: (i32, i32) = (1, 2); log(p.0); return; }");
    defer result.diag.deinit();
    try testing.expect(!result.diag.hasErrors());
    const decl = result.ast.program.decls.items[0];
    switch (decl.decl) {
        .function => |f| {
            // Second statement is log(p.0) which is an expr_stmt
            switch (f.body.stmts.items[1]) {
                .expr_stmt => |expr_idx| {
                    const expr = result.ast.getExpr(expr_idx);
                    switch (expr) {
                        .call => |call_e| {
                            // The argument is p.0 (field_access)
                            const arg_expr = result.ast.getExpr(call_e.args.items[0]);
                            switch (arg_expr) {
                                .field_access => |fa| {
                                    try testing.expectEqualStrings("0", fa.field);
                                },
                                else => return error.TestUnexpectedResult,
                            }
                        },
                        else => return error.TestUnexpectedResult,
                    }
                },
                else => return error.TestUnexpectedResult,
            }
        },
    }
}

test "parse grouped expression still works" {
    var result = try testParse("package main;\npub fn main() -> void { let x: i32 = (1 + 2); return; }");
    defer result.diag.deinit();
    try testing.expect(!result.diag.hasErrors());
    const decl = result.ast.program.decls.items[0];
    switch (decl.decl) {
        .function => |f| {
            switch (f.body.stmts.items[0]) {
                .var_decl => |vd| {
                    const expr = result.ast.getExpr(vd.init_expr);
                    switch (expr) {
                        .grouped => {},
                        else => return error.TestUnexpectedResult,
                    }
                },
                else => return error.TestUnexpectedResult,
            }
        },
    }
}

test "parse extern function declaration" {
    var result = try testParse("package main;\npub extern fn abs(x: i32) -> i32;\npub fn main() -> void { return; }");
    defer result.diag.deinit();
    try testing.expect(!result.diag.hasErrors());
    try testing.expectEqual(@as(usize, 2), result.ast.program.decls.items.len);
    const decl = result.ast.program.decls.items[0];
    try testing.expectEqual(Ast.Visibility.public, decl.visibility);
    switch (decl.decl) {
        .extern_decl => |ed| {
            try testing.expectEqualStrings("abs", ed.name);
            try testing.expectEqual(@as(usize, 1), ed.params.items.len);
            try testing.expectEqualStrings("x", ed.params.items[0].name);
            try testing.expectEqual(Ast.BuiltinType.i32_type, ed.return_type.builtin);
        },
        else => return error.TestUnexpectedResult,
    }
}

test "parse extern function with string parameter" {
    var result = try testParse("package main;\npub extern fn atoi(s: string) -> i32;\npub fn main() -> void { return; }");
    defer result.diag.deinit();
    try testing.expect(!result.diag.hasErrors());
    const decl = result.ast.program.decls.items[0];
    switch (decl.decl) {
        .extern_decl => |ed| {
            try testing.expectEqualStrings("atoi", ed.name);
            try testing.expectEqual(@as(usize, 1), ed.params.items.len);
            try testing.expectEqual(Ast.BuiltinType.string_type, ed.params.items[0].type_expr.builtin);
        },
        else => return error.TestUnexpectedResult,
    }
}

test "parse unsafe block" {
    var result = try testParse("package main;\npub fn main() -> void { unsafe { let x: i32 = 42; } return; }");
    defer result.diag.deinit();
    try testing.expect(!result.diag.hasErrors());
    const decl = result.ast.program.decls.items[0];
    switch (decl.decl) {
        .function => |f| {
            try testing.expectEqual(@as(usize, 2), f.body.stmts.items.len);
            switch (f.body.stmts.items[0]) {
                .unsafe_block => |block| {
                    try testing.expectEqual(@as(usize, 1), block.stmts.items.len);
                    switch (block.stmts.items[0]) {
                        .var_decl => |vd| {
                            try testing.expectEqualStrings("x", vd.name);
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

test "parse extern with link name" {
    var result = try testParse("package main;\npub extern \"c\" fn puts(s: string) -> i32;\npub fn main() -> void { return; }");
    defer result.diag.deinit();
    try testing.expect(!result.diag.hasErrors());
    const decl = result.ast.program.decls.items[0];
    switch (decl.decl) {
        .extern_decl => |ed| {
            try testing.expectEqualStrings("puts", ed.name);
        },
        else => return error.TestUnexpectedResult,
    }
}

test "parse const declaration" {
    var result = try testParse("package main;\npub const MAX: i32 = 100;\npub fn main() -> void { return; }");
    defer result.diag.deinit();
    try testing.expect(!result.diag.hasErrors());
    try testing.expectEqual(@as(usize, 2), result.ast.program.decls.items.len);
    const decl = result.ast.program.decls.items[0];
    try testing.expectEqual(Ast.Visibility.public, decl.visibility);
    switch (decl.decl) {
        .const_decl => |cd| {
            try testing.expectEqualStrings("MAX", cd.name);
            try testing.expectEqual(Ast.BuiltinType.i32_type, cd.type_expr.builtin);
            const expr = result.ast.getExpr(cd.init_expr);
            switch (expr) {
                .int_literal => |val| try testing.expectEqual(@as(i64, 100), val),
                else => return error.TestUnexpectedResult,
            }
        },
        else => return error.TestUnexpectedResult,
    }
}

test "parse const string declaration" {
    var result = try testParse("package main;\npub const MSG: string = \"hello\";\npub fn main() -> void { return; }");
    defer result.diag.deinit();
    try testing.expect(!result.diag.hasErrors());
    const decl = result.ast.program.decls.items[0];
    switch (decl.decl) {
        .const_decl => |cd| {
            try testing.expectEqualStrings("MSG", cd.name);
            try testing.expectEqual(Ast.BuiltinType.string_type, cd.type_expr.builtin);
        },
        else => return error.TestUnexpectedResult,
    }
}

test "parse const float declaration" {
    var result = try testParse("package main;\npub const PI: f64 = 3.14;\npub fn main() -> void { return; }");
    defer result.diag.deinit();
    try testing.expect(!result.diag.hasErrors());
    const decl = result.ast.program.decls.items[0];
    switch (decl.decl) {
        .const_decl => |cd| {
            try testing.expectEqualStrings("PI", cd.name);
            try testing.expectEqual(Ast.BuiltinType.f64_type, cd.type_expr.builtin);
            const expr = result.ast.getExpr(cd.init_expr);
            switch (expr) {
                .float_literal => |val| try testing.expectApproxEqAbs(3.14, val, 0.001),
                else => return error.TestUnexpectedResult,
            }
        },
        else => return error.TestUnexpectedResult,
    }
}

test "parse const bool declaration" {
    var result = try testParse("package main;\npub const DEBUG: bool = true;\npub fn main() -> void { return; }");
    defer result.diag.deinit();
    try testing.expect(!result.diag.hasErrors());
    const decl = result.ast.program.decls.items[0];
    switch (decl.decl) {
        .const_decl => |cd| {
            try testing.expectEqualStrings("DEBUG", cd.name);
            try testing.expectEqual(Ast.BuiltinType.bool_type, cd.type_expr.builtin);
        },
        else => return error.TestUnexpectedResult,
    }
}

test "parse sizeof expression" {
    var result = try testParse("package main;\npub fn main() -> void { let x: i32 = sizeof(i32); return; }");
    defer result.diag.deinit();
    try testing.expect(!result.diag.hasErrors());
    const decl = result.ast.program.decls.items[0];
    switch (decl.decl) {
        .function => |f| {
            switch (f.body.stmts.items[0]) {
                .var_decl => |vd| {
                    try testing.expectEqualStrings("x", vd.name);
                    const expr = result.ast.getExpr(vd.init_expr);
                    switch (expr) {
                        .sizeof_expr => |te| {
                            try testing.expectEqual(Ast.BuiltinType.i32_type, te.builtin);
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

test "parse sizeof f64" {
    var result = try testParse("package main;\npub fn main() -> void { let x: i32 = sizeof(f64); return; }");
    defer result.diag.deinit();
    try testing.expect(!result.diag.hasErrors());
    const decl = result.ast.program.decls.items[0];
    switch (decl.decl) {
        .function => |f| {
            switch (f.body.stmts.items[0]) {
                .var_decl => |vd| {
                    const expr = result.ast.getExpr(vd.init_expr);
                    switch (expr) {
                        .sizeof_expr => |te| {
                            try testing.expectEqual(Ast.BuiltinType.f64_type, te.builtin);
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

test "parse import with alias" {
    var result = try testParse("package main;\nimport \"std/math\" as math;\npub fn main() -> void { return; }");
    defer result.diag.deinit();
    try testing.expect(!result.diag.hasErrors());
    try testing.expectEqual(@as(usize, 1), result.ast.program.imports.items.len);
    const imp = result.ast.program.imports.items[0];
    try testing.expectEqualStrings("std/math", imp.path);
    try testing.expectEqualStrings("math", imp.alias.?);
    try testing.expectEqual(@as(usize, 1), result.ast.program.decls.items.len);
}

test "parse import without alias" {
    var result = try testParse("package main;\nimport \"std/io\";\npub fn main() -> void { return; }");
    defer result.diag.deinit();
    try testing.expect(!result.diag.hasErrors());
    try testing.expectEqual(@as(usize, 1), result.ast.program.imports.items.len);
    const imp = result.ast.program.imports.items[0];
    try testing.expectEqualStrings("std/io", imp.path);
    try testing.expect(imp.alias == null);
}

test "parse multiple imports" {
    var result = try testParse("package main;\nimport \"std/math\" as math;\nimport \"std/io\" as io;\npub fn main() -> void { return; }");
    defer result.diag.deinit();
    try testing.expect(!result.diag.hasErrors());
    try testing.expectEqual(@as(usize, 2), result.ast.program.imports.items.len);
    try testing.expectEqualStrings("std/math", result.ast.program.imports.items[0].path);
    try testing.expectEqualStrings("math", result.ast.program.imports.items[0].alias.?);
    try testing.expectEqualStrings("std/io", result.ast.program.imports.items[1].path);
    try testing.expectEqualStrings("io", result.ast.program.imports.items[1].alias.?);
}

test "parse no imports" {
    var result = try testParse("package main;\npub fn main() -> void { return; }");
    defer result.diag.deinit();
    try testing.expect(!result.diag.hasErrors());
    try testing.expectEqual(@as(usize, 0), result.ast.program.imports.items.len);
    try testing.expectEqual(@as(usize, 1), result.ast.program.decls.items.len);
}

test "parse generic struct declaration" {
    var result = try testParse("package main;\npub struct Pair<T> { pub first: T, pub second: T }\npub fn main() -> void { return; }");
    defer result.diag.deinit();
    try testing.expect(!result.diag.hasErrors());
    try testing.expectEqual(@as(usize, 2), result.ast.program.decls.items.len);
    const decl = result.ast.program.decls.items[0];
    switch (decl.decl) {
        .struct_decl => |sd| {
            try testing.expectEqualStrings("Pair", sd.name);
            try testing.expectEqual(@as(usize, 1), sd.type_params.items.len);
            try testing.expectEqualStrings("T", sd.type_params.items[0]);
            try testing.expectEqual(@as(usize, 2), sd.fields.items.len);
            try testing.expectEqualStrings("first", sd.fields.items[0].name);
            try testing.expectEqualStrings("second", sd.fields.items[1].name);
        },
        else => return error.TestUnexpectedResult,
    }
}

test "parse generic struct with multiple type params" {
    var result = try testParse("package main;\npub struct Map<K, V> { pub key: K, pub value: V }\npub fn main() -> void { return; }");
    defer result.diag.deinit();
    try testing.expect(!result.diag.hasErrors());
    const decl = result.ast.program.decls.items[0];
    switch (decl.decl) {
        .struct_decl => |sd| {
            try testing.expectEqualStrings("Map", sd.name);
            try testing.expectEqual(@as(usize, 2), sd.type_params.items.len);
            try testing.expectEqualStrings("K", sd.type_params.items[0]);
            try testing.expectEqualStrings("V", sd.type_params.items[1]);
        },
        else => return error.TestUnexpectedResult,
    }
}

test "parse generic type in variable declaration" {
    var result = try testParse("package main;\npub fn main() -> void { let p: Pair<i32> = Pair<i32> { first: 10, second: 20 }; return; }");
    defer result.diag.deinit();
    try testing.expect(!result.diag.hasErrors());
    const decl = result.ast.program.decls.items[0];
    switch (decl.decl) {
        .function => |f| {
            switch (f.body.stmts.items[0]) {
                .var_decl => |vd| {
                    try testing.expectEqualStrings("p", vd.name);
                    // Type should be generic_type
                    switch (vd.type_expr) {
                        .generic_type => |gt| {
                            try testing.expectEqualStrings("Pair", gt.name);
                            try testing.expectEqual(@as(usize, 1), gt.type_args.items.len);
                            try testing.expectEqual(Ast.BuiltinType.i32_type, gt.type_args.items[0].builtin);
                        },
                        else => return error.TestUnexpectedResult,
                    }
                    // Init should be a struct literal with type args
                    const expr = result.ast.getExpr(vd.init_expr);
                    switch (expr) {
                        .struct_literal => |sl| {
                            try testing.expectEqualStrings("Pair", sl.name);
                            try testing.expectEqual(@as(usize, 1), sl.type_args.items.len);
                            try testing.expectEqual(@as(usize, 2), sl.field_inits.items.len);
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

test "parse non-generic struct still works" {
    var result = try testParse("package main;\npub struct Point { pub x: i32, pub y: i32 }\npub fn main() -> void { return; }");
    defer result.diag.deinit();
    try testing.expect(!result.diag.hasErrors());
    const decl = result.ast.program.decls.items[0];
    switch (decl.decl) {
        .struct_decl => |sd| {
            try testing.expectEqualStrings("Point", sd.name);
            try testing.expectEqual(@as(usize, 0), sd.type_params.items.len);
            try testing.expectEqual(@as(usize, 2), sd.fields.items.len);
        },
        else => return error.TestUnexpectedResult,
    }
}

test "parse annotation with argument" {
    var result = try testParse("package main;\n#[human_review(high)]\npub fn foo() -> void { return; }");
    defer result.diag.deinit();
    try testing.expect(!result.diag.hasErrors());
    try testing.expectEqual(@as(usize, 1), result.ast.program.decls.items.len);
    const decl = result.ast.program.decls.items[0];
    try testing.expectEqual(@as(usize, 1), decl.annotations.items.len);
    try testing.expectEqualStrings("human_review", decl.annotations.items[0].name);
    try testing.expectEqual(@as(usize, 1), decl.annotations.items[0].args.items.len);
    try testing.expectEqualStrings("high", decl.annotations.items[0].args.items[0]);
    switch (decl.decl) {
        .function => |f| try testing.expectEqualStrings("foo", f.name),
    }
}

test "parse annotation without arguments" {
    var result = try testParse("package main;\n#[constant_time]\npub fn compare() -> void { return; }");
    defer result.diag.deinit();
    try testing.expect(!result.diag.hasErrors());
    const decl = result.ast.program.decls.items[0];
    try testing.expectEqual(@as(usize, 1), decl.annotations.items.len);
    try testing.expectEqualStrings("constant_time", decl.annotations.items[0].name);
    try testing.expectEqual(@as(usize, 0), decl.annotations.items[0].args.items.len);
}

test "parse multiple annotations on one declaration" {
    var result = try testParse("package main;\n#[human_review(high)]\n#[constant_time]\npub fn secure() -> void { return; }");
    defer result.diag.deinit();
    try testing.expect(!result.diag.hasErrors());
    const decl = result.ast.program.decls.items[0];
    try testing.expectEqual(@as(usize, 2), decl.annotations.items.len);
    try testing.expectEqualStrings("human_review", decl.annotations.items[0].name);
    try testing.expectEqualStrings("constant_time", decl.annotations.items[1].name);
}

test "parse annotation with multiple arguments" {
    var result = try testParse("package main;\n#[ownership(gc, shared)]\npub fn alloc() -> void { return; }");
    defer result.diag.deinit();
    try testing.expect(!result.diag.hasErrors());
    const decl = result.ast.program.decls.items[0];
    try testing.expectEqual(@as(usize, 1), decl.annotations.items.len);
    try testing.expectEqualStrings("ownership", decl.annotations.items[0].name);
    try testing.expectEqual(@as(usize, 2), decl.annotations.items[0].args.items.len);
    try testing.expectEqualStrings("gc", decl.annotations.items[0].args.items[0]);
    try testing.expectEqualStrings("shared", decl.annotations.items[0].args.items[1]);
}

test "parse declaration without annotations has empty list" {
    var result = try testParse("package main;\npub fn main() -> void { return; }");
    defer result.diag.deinit();
    try testing.expect(!result.diag.hasErrors());
    const decl = result.ast.program.decls.items[0];
    try testing.expectEqual(@as(usize, 0), decl.annotations.items.len);
}

test "parse annotation on struct" {
    var result = try testParse("package main;\n#[ownership(gc)]\npub struct Data { pub x: i32 }\npub fn main() -> void { return; }");
    defer result.diag.deinit();
    try testing.expect(!result.diag.hasErrors());
    const decl = result.ast.program.decls.items[0];
    try testing.expectEqual(@as(usize, 1), decl.annotations.items.len);
    try testing.expectEqualStrings("ownership", decl.annotations.items[0].name);
    switch (decl.decl) {
        .struct_decl => |sd| try testing.expectEqualStrings("Data", sd.name),
        else => return error.TestUnexpectedResult,
    }
}

test "parse spawn statement" {
    var result = try testParse("package main;\npub fn greet() -> void { return; }\npub fn main() -> void { spawn greet(); return; }");
    defer result.diag.deinit();
    try testing.expect(!result.diag.hasErrors());
    const decl = result.ast.program.decls.items[1];
    switch (decl.decl) {
        .function => |f| {
            try testing.expectEqual(@as(usize, 2), f.body.stmts.items.len);
            switch (f.body.stmts.items[0]) {
                .spawn_stmt => |spawn_s| {
                    const expr = result.ast.getExpr(spawn_s.expr);
                    switch (expr) {
                        .call => |call_e| {
                            const callee = result.ast.getExpr(call_e.callee);
                            switch (callee) {
                                .identifier => |name| try testing.expectEqualStrings("greet", name),
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

test "parse spawn_with_handle statement" {
    var result = try testParse("package main;\npub fn compute(x: i32) -> i32 { return x; }\npub fn main() -> void { spawn_with_handle h = compute(5); return; }");
    defer result.diag.deinit();
    try testing.expect(!result.diag.hasErrors());
    const decl = result.ast.program.decls.items[1];
    switch (decl.decl) {
        .function => |f| {
            try testing.expectEqual(@as(usize, 2), f.body.stmts.items.len);
            switch (f.body.stmts.items[0]) {
                .spawn_handle_stmt => |sh| {
                    try testing.expectEqualStrings("h", sh.handle_name);
                    const expr = result.ast.getExpr(sh.expr);
                    switch (expr) {
                        .call => |call_e| {
                            const callee = result.ast.getExpr(call_e.callee);
                            switch (callee) {
                                .identifier => |name| try testing.expectEqualStrings("compute", name),
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

test "parse await expression" {
    var result = try testParse("package main;\npub fn compute(x: i32) -> i32 { return x; }\npub fn main() -> void { spawn_with_handle h = compute(5); let v: i32 = await h; return; }");
    defer result.diag.deinit();
    try testing.expect(!result.diag.hasErrors());
    const decl = result.ast.program.decls.items[1];
    switch (decl.decl) {
        .function => |f| {
            try testing.expectEqual(@as(usize, 3), f.body.stmts.items.len);
            switch (f.body.stmts.items[1]) {
                .var_decl => |vd| {
                    try testing.expectEqualStrings("v", vd.name);
                    const init_expr = result.ast.getExpr(vd.init_expr);
                    switch (init_expr) {
                        .await_expr => |inner| {
                            const awaited = result.ast.getExpr(inner);
                            switch (awaited) {
                                .identifier => |name| try testing.expectEqualStrings("h", name),
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

test "parse triple-quoted string literal" {
    var result = try testParse("package main;\npub fn main() -> void { let s: string = \"\"\"hello\nworld\"\"\"; return; }");
    defer result.diag.deinit();
    try testing.expect(!result.diag.hasErrors());
    const decl = result.ast.program.decls.items[0];
    switch (decl.decl) {
        .function => |f| {
            switch (f.body.stmts.items[0]) {
                .var_decl => |vd| {
                    try testing.expectEqualStrings("s", vd.name);
                    const expr = result.ast.getExpr(vd.init_expr);
                    switch (expr) {
                        .string_literal => |val| try testing.expectEqualStrings("hello\nworld", val),
                        else => return error.TestUnexpectedResult,
                    }
                },
                else => return error.TestUnexpectedResult,
            }
        },
    }
}

test "parse raw triple-quoted string literal" {
    var result = try testParse("package main;\npub fn main() -> void { let s: string = r\"\"\"raw\\ncontent\"\"\"; return; }");
    defer result.diag.deinit();
    try testing.expect(!result.diag.hasErrors());
    const decl = result.ast.program.decls.items[0];
    switch (decl.decl) {
        .function => |f| {
            switch (f.body.stmts.items[0]) {
                .var_decl => |vd| {
                    try testing.expectEqualStrings("s", vd.name);
                    const expr = result.ast.getExpr(vd.init_expr);
                    switch (expr) {
                        .string_literal => |val| try testing.expectEqualStrings("raw\\ncontent", val),
                        else => return error.TestUnexpectedResult,
                    }
                },
                else => return error.TestUnexpectedResult,
            }
        },
    }
}

test "parse slice type" {
    var result = try testParse("package main;\npub fn foo(s: []i32) -> void { return; }");
    defer result.diag.deinit();
    try testing.expect(!result.diag.hasErrors());
    const decl = result.ast.program.decls.items[0];
    switch (decl.decl) {
        .function => |f| {
            try testing.expectEqual(@as(usize, 1), f.params.items.len);
            try testing.expectEqualStrings("s", f.params.items[0].name);
            switch (f.params.items[0].type_expr) {
                .slice_type => |sl| {
                    try testing.expectEqual(Ast.BuiltinType.i32_type, sl.element_type.builtin);
                },
                else => return error.TestUnexpectedResult,
            }
        },
        else => return error.TestUnexpectedResult,
    }
}

test "parse slice type in variable declaration" {
    var result = try testParse("package main;\npub fn main() -> void { let s: []i32 = arr[:]; return; }");
    defer result.diag.deinit();
    try testing.expect(!result.diag.hasErrors());
    const decl = result.ast.program.decls.items[0];
    switch (decl.decl) {
        .function => |f| {
            switch (f.body.stmts.items[0]) {
                .var_decl => |vd| {
                    try testing.expectEqualStrings("s", vd.name);
                    switch (vd.type_expr) {
                        .slice_type => |sl| {
                            try testing.expectEqual(Ast.BuiltinType.i32_type, sl.element_type.builtin);
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

test "parse full slice expression arr[:]" {
    var result = try testParse("package main;\npub fn main() -> void { let s: []i32 = arr[:]; return; }");
    defer result.diag.deinit();
    try testing.expect(!result.diag.hasErrors());
    const decl = result.ast.program.decls.items[0];
    switch (decl.decl) {
        .function => |f| {
            switch (f.body.stmts.items[0]) {
                .var_decl => |vd| {
                    const expr = result.ast.getExpr(vd.init_expr);
                    switch (expr) {
                        .slice_expr => |se| {
                            try testing.expect(se.start == null);
                            try testing.expect(se.end == null);
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

test "parse partial slice expression arr[1:4]" {
    var result = try testParse("package main;\npub fn main() -> void { let s: []i32 = arr[1:4]; return; }");
    defer result.diag.deinit();
    try testing.expect(!result.diag.hasErrors());
    const decl = result.ast.program.decls.items[0];
    switch (decl.decl) {
        .function => |f| {
            switch (f.body.stmts.items[0]) {
                .var_decl => |vd| {
                    const expr = result.ast.getExpr(vd.init_expr);
                    switch (expr) {
                        .slice_expr => |se| {
                            try testing.expect(se.start != null);
                            try testing.expect(se.end != null);
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

test "parse index access still works with slices" {
    var result = try testParse("package main;\npub fn main() -> void { let x: i32 = arr[0]; return; }");
    defer result.diag.deinit();
    try testing.expect(!result.diag.hasErrors());
    const decl = result.ast.program.decls.items[0];
    switch (decl.decl) {
        .function => |f| {
            switch (f.body.stmts.items[0]) {
                .var_decl => |vd| {
                    const expr = result.ast.getExpr(vd.init_expr);
                    switch (expr) {
                        .index_access => {},
                        else => return error.TestUnexpectedResult,
                    }
                },
                else => return error.TestUnexpectedResult,
            }
        },
        else => return error.TestUnexpectedResult,
    }
}

test "parse pointer type *mut i32" {
    var result = try testParse("package main;\npub fn foo(p: *mut i32) -> void { return; }");
    defer result.diag.deinit();
    try testing.expect(!result.diag.hasErrors());
    const decl = result.ast.program.decls.items[0];
    switch (decl.decl) {
        .function => |f| {
            try testing.expectEqual(@as(usize, 1), f.params.items.len);
            switch (f.params.items[0].type_expr) {
                .ptr_type => |pt| {
                    try testing.expect(pt.is_mutable);
                    try testing.expectEqual(Ast.BuiltinType.i32_type, pt.pointee.builtin);
                },
                else => return error.TestUnexpectedResult,
            }
        },
    }
}

test "parse pointer type *const i32" {
    var result = try testParse("package main;\npub fn foo(p: *const i32) -> void { return; }");
    defer result.diag.deinit();
    try testing.expect(!result.diag.hasErrors());
    const decl = result.ast.program.decls.items[0];
    switch (decl.decl) {
        .function => |f| {
            switch (f.params.items[0].type_expr) {
                .ptr_type => |pt| {
                    try testing.expect(!pt.is_mutable);
                    try testing.expectEqual(Ast.BuiltinType.i32_type, pt.pointee.builtin);
                },
                else => return error.TestUnexpectedResult,
            }
        },
    }
}

test "parse address-of expression" {
    var result = try testParse("package main;\npub fn main() -> void { let mut x: i32 = 10; unsafe { let p: *mut i32 = &x; } return; }");
    defer result.diag.deinit();
    try testing.expect(!result.diag.hasErrors());
    const decl = result.ast.program.decls.items[0];
    switch (decl.decl) {
        .function => |f| {
            // Second stmt is unsafe block containing var_decl
            switch (f.body.stmts.items[1]) {
                .unsafe_block => |block| {
                    switch (block.stmts.items[0]) {
                        .var_decl => |vd| {
                            try testing.expectEqualStrings("p", vd.name);
                            const expr = result.ast.getExpr(vd.init_expr);
                            switch (expr) {
                                .address_of => |operand_idx| {
                                    const operand = result.ast.getExpr(operand_idx);
                                    switch (operand) {
                                        .identifier => |name| try testing.expectEqualStrings("x", name),
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
        },
    }
}

test "parse deref assignment" {
    var result = try testParse("package main;\npub fn main() -> void { let mut x: i32 = 10; unsafe { let p: *mut i32 = &x; *p = 42; } return; }");
    defer result.diag.deinit();
    try testing.expect(!result.diag.hasErrors());
    const decl = result.ast.program.decls.items[0];
    switch (decl.decl) {
        .function => |f| {
            switch (f.body.stmts.items[1]) {
                .unsafe_block => |block| {
                    // Second stmt in unsafe block is deref_assign
                    switch (block.stmts.items[1]) {
                        .deref_assign => |da| {
                            // Target should be identifier "p"
                            const target_expr = result.ast.getExpr(da.target);
                            switch (target_expr) {
                                .identifier => |name| try testing.expectEqualStrings("p", name),
                                else => return error.TestUnexpectedResult,
                            }
                            // Value should be int literal 42
                            const val_expr = result.ast.getExpr(da.value);
                            switch (val_expr) {
                                .int_literal => |v| try testing.expectEqual(@as(i64, 42), v),
                                else => return error.TestUnexpectedResult,
                            }
                        },
                        else => return error.TestUnexpectedResult,
                    }
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

test "parse generic enum declaration" {
    var result = try testParse("package main;\npub enum Either<L, R> { pub Left(L), pub Right(R) }\npub fn main() -> void { return; }");
    defer result.diag.deinit();
    try testing.expect(!result.diag.hasErrors());
    try testing.expectEqual(@as(usize, 2), result.ast.program.decls.items.len);
    const decl = result.ast.program.decls.items[0];
    switch (decl.decl) {
        .enum_decl => |ed| {
            try testing.expectEqualStrings("Either", ed.name);
            try testing.expectEqual(@as(usize, 2), ed.type_params.items.len);
            try testing.expectEqualStrings("L", ed.type_params.items[0]);
            try testing.expectEqualStrings("R", ed.type_params.items[1]);
            try testing.expectEqual(@as(usize, 2), ed.variants.items.len);
            try testing.expectEqualStrings("Left", ed.variants.items[0].name);
            try testing.expectEqualStrings("Right", ed.variants.items[1].name);
            try testing.expectEqual(@as(usize, 1), ed.variants.items[0].data_types.items.len);
            try testing.expectEqual(@as(usize, 1), ed.variants.items[1].data_types.items.len);
        },
        else => return error.TestUnexpectedResult,
    }
}

test "parse generic enum with single type param" {
    var result = try testParse("package main;\npub enum Wrapper<T> { pub Value(T), pub Empty }\npub fn main() -> void { return; }");
    defer result.diag.deinit();
    try testing.expect(!result.diag.hasErrors());
    const decl = result.ast.program.decls.items[0];
    switch (decl.decl) {
        .enum_decl => |ed| {
            try testing.expectEqualStrings("Wrapper", ed.name);
            try testing.expectEqual(@as(usize, 1), ed.type_params.items.len);
            try testing.expectEqualStrings("T", ed.type_params.items[0]);
            try testing.expectEqual(@as(usize, 2), ed.variants.items.len);
            try testing.expectEqualStrings("Value", ed.variants.items[0].name);
            try testing.expectEqual(@as(usize, 1), ed.variants.items[0].data_types.items.len);
            try testing.expectEqualStrings("Empty", ed.variants.items[1].name);
            try testing.expectEqual(@as(usize, 0), ed.variants.items[1].data_types.items.len);
        },
        else => return error.TestUnexpectedResult,
    }
}

test "parse non-generic enum still works" {
    var result = try testParse("package main;\npub enum Color { pub Red, pub Green, pub Blue }\npub fn main() -> void { return; }");
    defer result.diag.deinit();
    try testing.expect(!result.diag.hasErrors());
    const decl = result.ast.program.decls.items[0];
    switch (decl.decl) {
        .enum_decl => |ed| {
            try testing.expectEqualStrings("Color", ed.name);
            try testing.expectEqual(@as(usize, 0), ed.type_params.items.len);
            try testing.expectEqual(@as(usize, 3), ed.variants.items.len);
        },
        else => return error.TestUnexpectedResult,
    }
}

test "parse generic enum type in variable declaration" {
    var result = try testParse("package main;\npub fn main() -> void { let a: Either<i32, string> = Either.Left(42); return; }");
    defer result.diag.deinit();
    try testing.expect(!result.diag.hasErrors());
    const decl = result.ast.program.decls.items[0];
    switch (decl.decl) {
        .function => |f| {
            switch (f.body.stmts.items[0]) {
                .var_decl => |vd| {
                    try testing.expectEqualStrings("a", vd.name);
                    switch (vd.type_expr) {
                        .generic_type => |gt| {
                            try testing.expectEqualStrings("Either", gt.name);
                            try testing.expectEqual(@as(usize, 2), gt.type_args.items.len);
                            try testing.expectEqual(Ast.BuiltinType.i32_type, gt.type_args.items[0].builtin);
                            try testing.expectEqual(Ast.BuiltinType.string_type, gt.type_args.items[1].builtin);
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

test "parse associated function call" {
    var result = try testParse("package main;\npub fn main() -> void { let c: Circle = Circle::new(5.0); return; }");
    defer result.diag.deinit();
    try testing.expect(!result.diag.hasErrors());
    const decl = result.ast.program.decls.items[0];
    switch (decl.decl) {
        .function => |f| {
            switch (f.body.stmts.items[0]) {
                .var_decl => |vd| {
                    try testing.expectEqualStrings("c", vd.name);
                    const init_expr = result.ast.getExpr(vd.init_expr);
                    switch (init_expr) {
                        .associated_call => |ac| {
                            try testing.expectEqualStrings("Circle", ac.type_name);
                            try testing.expectEqualStrings("new", ac.func_name);
                            try testing.expectEqual(@as(usize, 1), ac.args.items.len);
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

test "parse associated function call with no args" {
    var result = try testParse("package main;\npub fn main() -> void { let c: Circle = Circle::unit(); return; }");
    defer result.diag.deinit();
    try testing.expect(!result.diag.hasErrors());
    const decl = result.ast.program.decls.items[0];
    switch (decl.decl) {
        .function => |f| {
            switch (f.body.stmts.items[0]) {
                .var_decl => |vd| {
                    const init_expr = result.ast.getExpr(vd.init_expr);
                    switch (init_expr) {
                        .associated_call => |ac| {
                            try testing.expectEqualStrings("Circle", ac.type_name);
                            try testing.expectEqualStrings("unit", ac.func_name);
                            try testing.expectEqual(@as(usize, 0), ac.args.items.len);
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

test "parse associated function call with multiple args" {
    var result = try testParse("package main;\npub fn main() -> void { let r: Rect = Rect::create(10, 20); return; }");
    defer result.diag.deinit();
    try testing.expect(!result.diag.hasErrors());
    const decl = result.ast.program.decls.items[0];
    switch (decl.decl) {
        .function => |f| {
            switch (f.body.stmts.items[0]) {
                .var_decl => |vd| {
                    const init_expr = result.ast.getExpr(vd.init_expr);
                    switch (init_expr) {
                        .associated_call => |ac| {
                            try testing.expectEqualStrings("Rect", ac.type_name);
                            try testing.expectEqualStrings("create", ac.func_name);
                            try testing.expectEqual(@as(usize, 2), ac.args.items.len);
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

test "parse char literal" {
    var result = try testParse("package main;\npub fn main() -> void { let c: char = 'A'; return; }");
    defer result.diag.deinit();
    try testing.expect(!result.diag.hasErrors());
    const decl = result.ast.program.decls.items[0];
    switch (decl.decl) {
        .function => |f| {
            switch (f.body.stmts.items[0]) {
                .var_decl => |vd| {
                    try testing.expectEqualStrings("c", vd.name);
                    try testing.expectEqual(Ast.BuiltinType.char_type, vd.type_expr.builtin);
                    const expr = result.ast.getExpr(vd.init_expr);
                    switch (expr) {
                        .char_literal => |val| try testing.expectEqual(@as(u8, 'A'), val),
                        else => return error.TestUnexpectedResult,
                    }
                },
                else => return error.TestUnexpectedResult,
            }
        },
    }
}

test "parse char literal escape sequence" {
    var result = try testParse("package main;\npub fn main() -> void { let c: char = '\\n'; return; }");
    defer result.diag.deinit();
    try testing.expect(!result.diag.hasErrors());
    const decl = result.ast.program.decls.items[0];
    switch (decl.decl) {
        .function => |f| {
            switch (f.body.stmts.items[0]) {
                .var_decl => |vd| {
                    const expr = result.ast.getExpr(vd.init_expr);
                    switch (expr) {
                        .char_literal => |val| try testing.expectEqual(@as(u8, '\n'), val),
                        else => return error.TestUnexpectedResult,
                    }
                },
                else => return error.TestUnexpectedResult,
            }
        },
    }
}

test "parse char to i32 type conversion" {
    var result = try testParse("package main;\npub fn main() -> void { let c: char = 'A'; let code: i32 = i32(c); return; }");
    defer result.diag.deinit();
    try testing.expect(!result.diag.hasErrors());
    const decl = result.ast.program.decls.items[0];
    switch (decl.decl) {
        .function => |f| {
            // Second statement should be the i32 conversion
            switch (f.body.stmts.items[1]) {
                .var_decl => |vd| {
                    try testing.expectEqualStrings("code", vd.name);
                    try testing.expectEqual(Ast.BuiltinType.i32_type, vd.type_expr.builtin);
                },
                else => return error.TestUnexpectedResult,
            }
        },
    }
}

test "parse repeated array initializer" {
    var result = try testParse("package main;\npub fn main() -> void { let a: [5]i32 = [0; 5]; return; }");
    defer result.diag.deinit();
    try testing.expect(!result.diag.hasErrors());
    const decl = result.ast.program.decls.items[0];
    switch (decl.decl) {
        .function => |f| {
            switch (f.body.stmts.items[0]) {
                .var_decl => |vd| {
                    try testing.expectEqualStrings("a", vd.name);
                    switch (vd.type_expr) {
                        .array_type => |arr| {
                            try testing.expectEqual(@as(u32, 5), arr.size);
                            try testing.expectEqual(Ast.BuiltinType.i32_type, arr.element_type.builtin);
                        },
                        else => return error.TestUnexpectedResult,
                    }
                    const expr = result.ast.getExpr(vd.init_expr);
                    switch (expr) {
                        .repeated_array => |ra| {
                            const val_expr = result.ast.getExpr(ra.value);
                            try testing.expectEqual(@as(i64, 0), val_expr.int_literal);
                            const count_expr = result.ast.getExpr(ra.count);
                            try testing.expectEqual(@as(i64, 5), count_expr.int_literal);
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

test "parse repeated array does not break regular array literal" {
    var result = try testParse("package main;\npub fn main() -> void { let a: [3]i32 = [1, 2, 3]; return; }");
    defer result.diag.deinit();
    try testing.expect(!result.diag.hasErrors());
    const decl = result.ast.program.decls.items[0];
    switch (decl.decl) {
        .function => |f| {
            switch (f.body.stmts.items[0]) {
                .var_decl => |vd| {
                    const expr = result.ast.getExpr(vd.init_expr);
                    switch (expr) {
                        .array_literal => |al| {
                            try testing.expectEqual(@as(usize, 3), al.elements.items.len);
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

// ===== processEscapes unit tests =====
const processEscapes = @import("parser.zig").processEscapes;

test "processEscapes: no escapes returns original slice" {
    const input = "hello world";
    const result = try processEscapes(testing.allocator, input);
    // Should return the same pointer (no allocation)
    try testing.expectEqual(input.ptr, result.ptr);
    try testing.expectEqualStrings("hello world", result);
}

test "processEscapes: newline escape" {
    const result = try processEscapes(testing.allocator, "line1\\nline2");
    defer testing.allocator.free(result);
    try testing.expectEqualStrings("line1\nline2", result);
}

test "processEscapes: tab escape" {
    const result = try processEscapes(testing.allocator, "col1\\tcol2");
    defer testing.allocator.free(result);
    try testing.expectEqualStrings("col1\tcol2", result);
}

test "processEscapes: carriage return escape" {
    const result = try processEscapes(testing.allocator, "hello\\rworld");
    defer testing.allocator.free(result);
    try testing.expectEqualStrings("hello\rworld", result);
}

test "processEscapes: backslash escape" {
    const result = try processEscapes(testing.allocator, "path\\\\file");
    defer testing.allocator.free(result);
    try testing.expectEqualStrings("path\\file", result);
}

test "processEscapes: double quote escape" {
    const result = try processEscapes(testing.allocator, "say \\\"hello\\\"");
    defer testing.allocator.free(result);
    try testing.expectEqualStrings("say \"hello\"", result);
}

test "processEscapes: single quote escape" {
    const result = try processEscapes(testing.allocator, "it\\'s");
    defer testing.allocator.free(result);
    try testing.expectEqualStrings("it's", result);
}

test "processEscapes: null byte escape" {
    const result = try processEscapes(testing.allocator, "hello\\0");
    defer testing.allocator.free(result);
    try testing.expectEqual(@as(usize, 6), result.len);
    try testing.expectEqual(@as(u8, 0), result[5]);
}

test "processEscapes: multiple escapes" {
    const result = try processEscapes(testing.allocator, "a\\tb\\nc\\\\d");
    defer testing.allocator.free(result);
    try testing.expectEqualStrings("a\tb\nc\\d", result);
}

test "processEscapes: empty string" {
    const result = try processEscapes(testing.allocator, "");
    try testing.expectEqualStrings("", result);
}
