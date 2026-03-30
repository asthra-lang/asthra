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
