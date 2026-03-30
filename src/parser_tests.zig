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

test "parse multiple functions" {
    var result = try testParse("package main;\npub fn foo() -> void { return; }\npriv fn bar() -> i32 { return 0; }");
    defer result.diag.deinit();
    try testing.expect(!result.diag.hasErrors());
    try testing.expectEqual(@as(usize, 2), result.ast.program.decls.items.len);
    try testing.expectEqual(Ast.Visibility.public, result.ast.program.decls.items[0].visibility);
    try testing.expectEqual(Ast.Visibility.private, result.ast.program.decls.items[1].visibility);
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

test "parse type alias declaration" {
    var result = try testParse("package main;\npub type Integer = i32;\npub fn main() -> void { return; }");
    defer result.diag.deinit();
    try testing.expect(!result.diag.hasErrors());
    try testing.expectEqual(@as(usize, 2), result.ast.program.decls.items.len);
    const decl = result.ast.program.decls.items[0];
    try testing.expectEqual(Ast.Visibility.public, decl.visibility);
    switch (decl.decl) {
        .type_alias => |ta| {
            try testing.expectEqualStrings("Integer", ta.name);
            try testing.expectEqual(Ast.BuiltinType.i32_type, ta.target.builtin);
        },
        else => return error.TestUnexpectedResult,
    }
}

test "parse type alias with string type" {
    var result = try testParse("package main;\npub type Text = string;\npub fn main() -> void { return; }");
    defer result.diag.deinit();
    try testing.expect(!result.diag.hasErrors());
    const decl = result.ast.program.decls.items[0];
    switch (decl.decl) {
        .type_alias => |ta| {
            try testing.expectEqualStrings("Text", ta.name);
            try testing.expectEqual(Ast.BuiltinType.string_type, ta.target.builtin);
        },
        else => return error.TestUnexpectedResult,
    }
}
