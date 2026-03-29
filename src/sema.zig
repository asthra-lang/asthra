const std = @import("std");
const Ast = @import("ast.zig").Ast;
const Diagnostics = @import("diagnostics.zig").Diagnostics;

pub const SemanticAnalyzer = struct {
    allocator: std.mem.Allocator,
    ast: *const Ast,
    diagnostics: *Diagnostics,
    scopes: std.ArrayList(Scope),
    functions: std.StringHashMap(FnSig),
    struct_names: std.StringHashMap(void),
    enum_names: std.StringHashMap(void),
    const_names: std.StringHashMap(void),
    type_aliases: std.StringHashMap(void),
    import_aliases: std.StringHashMap(void),
    in_loop: u32,
    error_count: u32,

    const Scope = struct {
        symbols: std.StringHashMap(SymbolInfo),
        is_function_scope: bool,
    };

    const SymbolInfo = struct {
        is_mutable: bool,
        is_used: bool,
        is_param: bool,
        start: u32,
    };

    const FnSig = struct {
        param_count: u32,
    };

    pub fn init(allocator: std.mem.Allocator, ast: *const Ast, diagnostics: *Diagnostics) SemanticAnalyzer {
        return .{
            .allocator = allocator,
            .ast = ast,
            .diagnostics = diagnostics,
            .scopes = std.ArrayList(Scope){},
            .functions = std.StringHashMap(FnSig).init(allocator),
            .struct_names = std.StringHashMap(void).init(allocator),
            .enum_names = std.StringHashMap(void).init(allocator),
            .const_names = std.StringHashMap(void).init(allocator),
            .type_aliases = std.StringHashMap(void).init(allocator),
            .import_aliases = std.StringHashMap(void).init(allocator),
            .in_loop = 0,
            .error_count = 0,
        };
    }

    pub fn deinit(self: *SemanticAnalyzer) void {
        for (self.scopes.items) |*scope| {
            scope.symbols.deinit();
        }
        self.scopes.deinit(self.allocator);
        self.functions.deinit();
        self.struct_names.deinit();
        self.enum_names.deinit();
        self.const_names.deinit();
        self.type_aliases.deinit();
        self.import_aliases.deinit();
    }

    pub fn analyze(self: *SemanticAnalyzer) void {
        // Register builtins
        self.registerBuiltin("log", 1);
        self.registerBuiltin("len", 1);
        self.registerBuiltin("range", 2); // range(end) or range(start, end)
        self.registerBuiltin("panic", 1);
        self.registerBuiltin("exit", 1);
        // Type conversion functions
        self.registerBuiltin("i32", 1);
        self.registerBuiltin("f64", 1);
        self.registerBuiltin("i64", 1);
        self.registerBuiltin("char", 1);
        self.registerBuiltin("bool", 1);
        self.registerBuiltin("sizeof", 1);
        // Higher-order array functions
        self.registerBuiltin("map", 2);
        self.registerBuiltin("filter", 2);
        self.registerBuiltin("reduce", 3);

        // Pass 1: Register all top-level declarations
        for (self.ast.program.decls.items) |decl| {
            switch (decl.decl) {
                .function => |f| {
                    self.functions.put(f.name, .{
                        .param_count = @intCast(f.params.items.len),
                    }) catch {};
                },
                .struct_decl => |s| {
                    self.struct_names.put(s.name, {}) catch {};
                },
                .enum_decl => |e| {
                    self.enum_names.put(e.name, {}) catch {};
                },
                .const_decl => |c| {
                    self.const_names.put(c.name, {}) catch {};
                },
                .type_alias => |t| {
                    self.type_aliases.put(t.name, {}) catch {};
                },
                .extern_decl => |e| {
                    self.functions.put(e.name, .{
                        .param_count = @intCast(e.params.items.len),
                    }) catch {};
                },
                .impl_decl => |impl| {
                    for (impl.methods.items) |method| {
                        // Register as TypeName_methodName for associated call resolution
                        const qualified = std.fmt.allocPrint(self.allocator, "{s}_{s}", .{ impl.type_name, method.name }) catch continue;
                        const actual_params: u32 = @intCast(method.params.items.len);
                        self.functions.put(qualified, .{
                            .param_count = actual_params,
                        }) catch {};
                        // Also register bare method name for method call resolution
                        self.functions.put(method.name, .{
                            .param_count = actual_params,
                        }) catch {};
                    }
                },
            }
        }

        // Register import aliases
        var alias_iter = self.ast.program.import_aliases.keyIterator();
        while (alias_iter.next()) |alias| {
            self.import_aliases.put(alias.*, {}) catch {};
        }

        // Pass 2: Check each function body
        for (self.ast.program.decls.items) |decl| {
            switch (decl.decl) {
                .function => |f| {
                    self.checkFunction(f);
                },
                .impl_decl => |impl| {
                    self.checkImplMethods(impl);
                },
                .const_decl => |c| {
                    // Check const initializer expression
                    if (c.init_expr != Ast.null_expr) {
                        self.checkExpr(c.init_expr);
                    }
                },
                else => {},
            }
        }
    }

    fn registerBuiltin(self: *SemanticAnalyzer, name: []const u8, param_count: u32) void {
        self.functions.put(name, .{ .param_count = param_count }) catch {};
    }

    fn checkFunction(self: *SemanticAnalyzer, f: Ast.FnDecl) void {
        self.pushScope(true);
        // Register parameters
        for (f.params.items) |param| {
            self.defineSymbol(param.name, .{
                .is_mutable = false,
                .is_used = false,
                .is_param = true,
                .start = 0,
            });
        }
        self.checkBlock(f.body, false);
        self.popScope();
    }

    fn checkImplMethods(self: *SemanticAnalyzer, impl: Ast.ImplDecl) void {
        for (impl.methods.items) |method| {
            self.pushScope(true);
            // Register self if method has self
            if (method.has_self) {
                self.defineSymbol("self", .{
                    .is_mutable = false,
                    .is_used = false,
                    .is_param = true,
                    .start = 0,
                });
            }
            // Register parameters
            for (method.params.items) |param| {
                self.defineSymbol(param.name, .{
                    .is_mutable = false,
                    .is_used = false,
                    .is_param = true,
                    .start = 0,
                });
            }
            self.checkBlock(method.body, false);
            self.popScope();
        }
    }

    fn checkBlock(self: *SemanticAnalyzer, block: Ast.Block, is_function_scope: bool) void {
        self.pushScope(is_function_scope);
        for (block.stmts.items) |stmt| {
            self.checkStmt(stmt);
        }
        self.popScope();
    }

    fn checkStmt(self: *SemanticAnalyzer, stmt: Ast.Stmt) void {
        switch (stmt) {
            .var_decl => |vd| self.checkVarDecl(vd),
            .assign => |assign| self.checkAssign(assign),
            .if_stmt => |if_s| {
                self.checkExpr(if_s.condition);
                self.checkBlock(if_s.then_block.*, false);
                if (if_s.else_block) |else_blk| {
                    self.checkBlock(else_blk.*, false);
                }
            },
            .if_let_stmt => |if_let| {
                self.checkExpr(if_let.expr);
                // then_block gets pattern bindings
                self.pushScope(false);
                self.registerPatternBindings(if_let.pattern);
                for (if_let.then_block.stmts.items) |s| {
                    self.checkStmt(s);
                }
                self.popScope();
                if (if_let.else_block) |else_blk| {
                    self.checkBlock(else_blk.*, false);
                }
            },
            .for_stmt => |for_s| {
                self.checkExpr(for_s.iterable);
                self.in_loop += 1;
                self.pushScope(false);
                self.defineSymbol(for_s.iter_var, .{
                    .is_mutable = false,
                    .is_used = false,
                    .is_param = false,
                    .start = 0,
                });
                for (for_s.body.stmts.items) |s| {
                    self.checkStmt(s);
                }
                self.popScope();
                self.in_loop -= 1;
            },
            .while_stmt => |while_s| {
                self.checkExpr(while_s.condition);
                self.in_loop += 1;
                self.checkBlock(while_s.body.*, false);
                self.in_loop -= 1;
            },
            .return_stmt => |ret| {
                if (ret.expr != Ast.null_expr) {
                    self.checkExpr(ret.expr);
                }
            },
            .expr_stmt => |expr_idx| {
                self.checkExpr(expr_idx);
            },
            .match_stmt => |match_s| {
                self.checkExpr(match_s.expr);
                for (match_s.arms.items) |arm| {
                    self.pushScope(false);
                    self.registerPatternBindings(arm.pattern);
                    for (arm.body.stmts.items) |s| {
                        self.checkStmt(s);
                    }
                    self.popScope();
                }
            },
            .break_stmt => {
                if (self.in_loop == 0) {
                    self.reportError(0, "break statement outside of loop");
                }
            },
            .continue_stmt => {
                if (self.in_loop == 0) {
                    self.reportError(0, "continue statement outside of loop");
                }
            },
            .unsafe_block => |block| {
                self.checkBlock(block.*, false);
            },
            .spawn_stmt => |spawn| {
                self.checkExpr(spawn.expr);
            },
            .spawn_handle_stmt => |spawn_h| {
                self.checkExpr(spawn_h.expr);
                self.defineSymbol(spawn_h.handle_name, .{
                    .is_mutable = false,
                    .is_used = false,
                    .is_param = false,
                    .start = 0,
                });
            },
            .deref_assign => |da| {
                self.checkExpr(da.target);
                self.checkExpr(da.value);
            },
        }
    }

    fn checkVarDecl(self: *SemanticAnalyzer, vd: Ast.VarDecl) void {
        // Check the initializer first
        if (vd.init_expr != Ast.null_expr) {
            self.checkExpr(vd.init_expr);
        }
        // Then define the symbol
        self.defineSymbol(vd.name, .{
            .is_mutable = vd.is_mutable,
            .is_used = false,
            .is_param = false,
            .start = vd.start,
        });
    }

    fn checkAssign(self: *SemanticAnalyzer, assign: Ast.AssignStmt) void {
        // Look up the target variable
        if (self.lookupSymbol(assign.target)) |info| {
            if (!info.is_mutable) {
                self.diagnostics.report(.@"error", assign.start, "cannot assign to immutable variable '{s}'", .{assign.target});
                self.error_count += 1;
            }
            self.markUsed(assign.target);
        } else {
            self.reportErrorWithName(assign.start, "undefined variable", assign.target);
        }
        // Check index expression if present
        if (assign.target_index) |idx| {
            self.checkExpr(idx);
        }
        // Check the value expression
        self.checkExpr(assign.value);
    }

    fn checkExpr(self: *SemanticAnalyzer, expr_idx: Ast.ExprIndex) void {
        if (expr_idx == Ast.null_expr) return;
        if (expr_idx >= self.ast.exprs.items.len) return;

        const expr = self.ast.getExpr(expr_idx);
        switch (expr) {
            .identifier => |name| {
                // Look up the identifier
                if (self.lookupSymbol(name)) |_| {
                    self.markUsed(name);
                } else {
                    // Don't report error if it's a known struct, enum, const, type alias, or import alias
                    if (self.struct_names.contains(name)) return;
                    if (self.enum_names.contains(name)) return;
                    if (self.const_names.contains(name)) return;
                    if (self.type_aliases.contains(name)) return;
                    if (self.import_aliases.contains(name)) return;
                    // Also allow well-known type names: Option, Result
                    if (std.mem.eql(u8, name, "Option")) return;
                    if (std.mem.eql(u8, name, "Result")) return;
                    // Allow stdlib namespace identifiers
                    if (std.mem.eql(u8, name, "math")) return;
                    if (std.mem.eql(u8, name, "str")) return;
                    if (std.mem.eql(u8, name, "io")) return;
                    if (std.mem.eql(u8, name, "os")) return;
                    self.reportErrorWithName(0, "undefined variable", name);
                }
            },
            .call => |call| {
                self.checkCallExpr(call);
            },
            .binary => |bin| {
                self.checkExpr(bin.lhs);
                self.checkExpr(bin.rhs);
            },
            .unary => |un| {
                self.checkExpr(un.operand);
            },
            .field_access => |fa| {
                self.checkExpr(fa.object);
            },
            .method_call => |mc| {
                self.checkExpr(mc.object);
                for (mc.args.items) |arg| {
                    self.checkExpr(arg);
                }
            },
            .index_access => |ia| {
                self.checkExpr(ia.object);
                self.checkExpr(ia.index);
            },
            .struct_literal => |sl| {
                for (sl.field_inits.items) |fi| {
                    self.checkExpr(fi.value);
                }
            },
            .array_literal => |al| {
                for (al.elements.items) |elem| {
                    self.checkExpr(elem);
                }
            },
            .tuple_literal => |tl| {
                for (tl.elements.items) |elem| {
                    self.checkExpr(elem);
                }
            },
            .grouped => |inner| {
                self.checkExpr(inner);
            },
            .associated_call => |ac| {
                for (ac.args.items) |arg| {
                    self.checkExpr(arg);
                }
            },
            .enum_constructor => |ec| {
                for (ec.args.items) |arg| {
                    self.checkExpr(arg);
                }
            },
            .await_expr => |inner| {
                self.checkExpr(inner);
            },
            .address_of => |inner| {
                self.checkExpr(inner);
            },
            .deref => |inner| {
                self.checkExpr(inner);
            },
            .slice_expr => |se| {
                self.checkExpr(se.object);
                if (se.start) |s| self.checkExpr(s);
                if (se.end) |e| self.checkExpr(e);
            },
            .repeated_array => |ra| {
                self.checkExpr(ra.value);
                self.checkExpr(ra.count);
            },
            .sizeof_expr => {},
            .int_literal => {},
            .float_literal => {},
            .bool_literal => {},
            .string_literal => {},
            .char_literal => {},
            .closure => |cl| {
                self.pushScope(true);
                for (cl.params.items) |param| {
                    self.defineSymbol(param.name, .{
                        .is_mutable = false,
                        .is_used = true,
                        .is_param = true,
                        .start = 0,
                    });
                }
                self.checkBlock(cl.body, false);
                self.popScope();
            },
        }
    }

    fn checkCallExpr(self: *SemanticAnalyzer, call: Ast.CallExpr) void {
        // Check each argument
        for (call.args.items) |arg| {
            self.checkExpr(arg);
        }

        // Get callee name if it's an identifier
        if (call.callee >= self.ast.exprs.items.len) return;
        const callee_expr = self.ast.getExpr(call.callee);
        switch (callee_expr) {
            .identifier => |name| {
                // Mark as used
                if (self.lookupSymbol(name)) |_| {
                    self.markUsed(name);
                }

                if (self.functions.get(name)) |sig| {
                    const arg_count: u32 = @intCast(call.args.items.len);
                    // Special case: range() accepts 1 or 2 args
                    if (std.mem.eql(u8, name, "range")) {
                        if (arg_count < 1 or arg_count > 2) {
                            self.diagnostics.report(.@"error", 0, "function 'range' takes 1 or 2 arguments, got {d}", .{arg_count});
                            self.error_count += 1;
                        }
                    } else if (arg_count != sig.param_count) {
                        self.diagnostics.report(.@"error", 0, "function '{s}' takes {d} arguments, got {d}", .{ name, sig.param_count, arg_count });
                        self.error_count += 1;
                    }
                }
                // If callee not in functions map, it might be a variable holding a function
                // or an import alias call - skip strict checking
            },
            else => {
                // Callee is a complex expression - just check it
                self.checkExpr(call.callee);
            },
        }
    }

    fn registerPatternBindings(self: *SemanticAnalyzer, pattern: Ast.Pattern) void {
        switch (pattern) {
            .enum_pattern => |ep| {
                for (ep.bindings.items) |binding| {
                    if (!std.mem.eql(u8, binding, "_")) {
                        self.defineSymbol(binding, .{
                            .is_mutable = false,
                            .is_used = false,
                            .is_param = false,
                            .start = 0,
                        });
                    }
                }
            },
            .tuple_pattern => |tp| {
                for (tp.elements.items) |elem| {
                    self.registerPatternBindings(elem);
                }
            },
            .identifier => |name| {
                if (!std.mem.eql(u8, name, "_")) {
                    self.defineSymbol(name, .{
                        .is_mutable = false,
                        .is_used = false,
                        .is_param = false,
                        .start = 0,
                    });
                }
            },
            .wildcard => {},
        }
    }

    // Scope management
    fn pushScope(self: *SemanticAnalyzer, is_function_scope: bool) void {
        self.scopes.append(self.allocator, .{
            .symbols = std.StringHashMap(SymbolInfo).init(self.allocator),
            .is_function_scope = is_function_scope,
        }) catch {};
    }

    fn popScope(self: *SemanticAnalyzer) void {
        if (self.scopes.items.len == 0) return;
        var scope = self.scopes.pop().?;

        // Warn on unused variables (skip params and underscore-prefixed names)
        var it = scope.symbols.iterator();
        while (it.next()) |entry| {
            const name = entry.key_ptr.*;
            const info = entry.value_ptr.*;
            if (!info.is_used and !info.is_param) {
                // Skip if name starts with '_'
                if (name.len > 0 and name[0] == '_') continue;
                self.diagnostics.report(.warning, info.start, "unused variable '{s}'", .{name});
            }
        }

        scope.symbols.deinit();
    }

    fn defineSymbol(self: *SemanticAnalyzer, name: []const u8, info: SymbolInfo) void {
        if (self.scopes.items.len == 0) return;
        var current = &self.scopes.items[self.scopes.items.len - 1];
        if (current.symbols.contains(name)) {
            self.diagnostics.report(.@"error", info.start, "duplicate variable '{s}' in this scope", .{name});
            self.error_count += 1;
            return;
        }
        current.symbols.put(name, info) catch {};
    }

    fn lookupSymbol(self: *SemanticAnalyzer, name: []const u8) ?SymbolInfo {
        // Walk from top of scope stack to bottom
        var i: usize = self.scopes.items.len;
        while (i > 0) {
            i -= 1;
            if (self.scopes.items[i].symbols.get(name)) |info| {
                return info;
            }
        }
        return null;
    }

    fn markUsed(self: *SemanticAnalyzer, name: []const u8) void {
        var i: usize = self.scopes.items.len;
        while (i > 0) {
            i -= 1;
            if (self.scopes.items[i].symbols.getPtr(name)) |info| {
                info.is_used = true;
                return;
            }
        }
    }

    fn reportError(self: *SemanticAnalyzer, start: u32, comptime msg: []const u8) void {
        self.diagnostics.report(.@"error", start, msg, .{});
        self.error_count += 1;
    }

    fn reportErrorWithName(self: *SemanticAnalyzer, start: u32, comptime prefix: []const u8, name: []const u8) void {
        self.diagnostics.report(.@"error", start, prefix ++ " '{s}'", .{name});
        self.error_count += 1;
    }
};

// --- Tests ---

const testing = std.testing;

fn testAnalyze(source: []const u8) !struct { diagnostics: Diagnostics, sema: SemanticAnalyzer } {
    const Parser = @import("parser.zig").Parser;
    var ast = Ast.init(testing.allocator);
    var diagnostics = Diagnostics.init(testing.allocator, source, "test.ast");
    var parser = Parser.init(source, &ast, &diagnostics);
    parser.parse() catch {};
    if (diagnostics.hasErrors()) {
        return .{ .diagnostics = diagnostics, .sema = undefined };
    }

    // We need to keep the AST alive - store it on the heap
    const ast_ptr = try testing.allocator.create(Ast);
    ast_ptr.* = ast;

    var sema = SemanticAnalyzer.init(testing.allocator, ast_ptr, &diagnostics);
    sema.analyze();
    return .{ .diagnostics = diagnostics, .sema = sema };
}

fn cleanupTest(result: *@TypeOf(testAnalyze("") catch unreachable)) void {
    const ast_ptr = @constCast(result.sema.ast);
    result.sema.deinit();
    // Free the AST expressions
    ast_ptr.exprs.deinit(ast_ptr.allocator);
    // Free the AST program fields
    ast_ptr.program.imports.deinit(ast_ptr.allocator);
    ast_ptr.program.decls.deinit(ast_ptr.allocator);
    ast_ptr.program.import_aliases.deinit();
    testing.allocator.destroy(ast_ptr);
    result.diagnostics.deinit();
}

test "no errors on valid program" {
    var result = try testAnalyze("package main;\npub fn main() -> void { let x: i32 = 42; log(x); return; }");
    defer cleanupTest(&result);

    try testing.expect(!result.diagnostics.hasErrors());
}

test "undefined variable detected" {
    var result = try testAnalyze("package main;\npub fn main() -> void { log(y); return; }");
    defer cleanupTest(&result);

    try testing.expect(result.diagnostics.hasErrors());
    var found = false;
    for (result.diagnostics.errors.items) |err| {
        if (err.severity == .@"error" and std.mem.indexOf(u8, err.message, "undefined variable") != null) {
            found = true;
            break;
        }
    }
    try testing.expect(found);
}

test "immutable assignment detected" {
    var result = try testAnalyze("package main;\npub fn main() -> void { let x: i32 = 42; x = 10; return; }");
    defer cleanupTest(&result);

    try testing.expect(result.diagnostics.hasErrors());
    var found = false;
    for (result.diagnostics.errors.items) |err| {
        if (err.severity == .@"error" and std.mem.indexOf(u8, err.message, "cannot assign to immutable") != null) {
            found = true;
            break;
        }
    }
    try testing.expect(found);
}

test "function argument count mismatch" {
    var result = try testAnalyze("package main;\npub fn add(a: i32, b: i32) -> i32 { return a; }\npub fn main() -> void { add(1); return; }");
    defer cleanupTest(&result);

    try testing.expect(result.diagnostics.hasErrors());
    var found = false;
    for (result.diagnostics.errors.items) |err| {
        if (err.severity == .@"error" and std.mem.indexOf(u8, err.message, "takes") != null) {
            found = true;
            break;
        }
    }
    try testing.expect(found);
}

test "break outside loop detected" {
    var result = try testAnalyze("package main;\npub fn main() -> void { break; return; }");
    defer cleanupTest(&result);

    try testing.expect(result.diagnostics.hasErrors());
    var found = false;
    for (result.diagnostics.errors.items) |err| {
        if (err.severity == .@"error" and std.mem.indexOf(u8, err.message, "break") != null) {
            found = true;
            break;
        }
    }
    try testing.expect(found);
}

test "duplicate variable in scope" {
    var result = try testAnalyze("package main;\npub fn main() -> void { let x: i32 = 1; let x: i32 = 2; return; }");
    defer cleanupTest(&result);

    try testing.expect(result.diagnostics.hasErrors());
    var found = false;
    for (result.diagnostics.errors.items) |err| {
        if (err.severity == .@"error" and std.mem.indexOf(u8, err.message, "duplicate variable") != null) {
            found = true;
            break;
        }
    }
    try testing.expect(found);
}
