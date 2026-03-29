const std = @import("std");
const Ast = @import("ast.zig").Ast;
const Diagnostics = @import("diagnostics.zig").Diagnostics;

const c = @cImport({
    @cInclude("llvm-c/Core.h");
    @cInclude("llvm-c/Analysis.h");
    @cInclude("llvm-c/Target.h");
    @cInclude("llvm-c/TargetMachine.h");
});

pub const CodeGen = struct {
    pub const GenError = error{ CodeGenError, OutOfMemory };

    context: c.LLVMContextRef,
    module: c.LLVMModuleRef,
    builder: c.LLVMBuilderRef,
    named_values: std.StringHashMap(NamedValue),
    diagnostics: *Diagnostics,
    allocator: std.mem.Allocator,
    ast: *const Ast,
    printf_fn: c.LLVMValueRef,
    fmt_int: c.LLVMValueRef,
    fmt_str: c.LLVMValueRef,
    fmt_float: c.LLVMValueRef,
    fmt_bool_true: c.LLVMValueRef,
    fmt_bool_false: c.LLVMValueRef,
    loop_stack: std.ArrayList(LoopContext) = .{},

    const LoopContext = struct {
        continue_bb: c.LLVMBasicBlockRef,
        break_bb: c.LLVMBasicBlockRef,
    };

    const NamedValue = struct {
        alloca: c.LLVMValueRef,
        is_mutable: bool,
        type_tag: TypeTag,
    };

    pub const TypeTag = enum {
        i32_type,
        i64_type,
        f64_type,
        bool_type,
        void_type,
        string_type,
    };

    pub fn init(allocator: std.mem.Allocator, module_name: [*:0]const u8, diagnostics: *Diagnostics, ast: *const Ast) CodeGen {
        const context = c.LLVMContextCreate();
        const module = c.LLVMModuleCreateWithNameInContext(module_name, context);
        const builder = c.LLVMCreateBuilderInContext(context);

        // Declare printf
        const printf_param_types = [_]c.LLVMTypeRef{c.LLVMPointerType(c.LLVMInt8TypeInContext(context), 0)};
        const printf_type = c.LLVMFunctionType(c.LLVMInt32TypeInContext(context), @constCast(&printf_param_types), 1, 1);
        const printf_fn = c.LLVMAddFunction(module, "printf", printf_type);

        // Create format strings — need a temp function to position the builder
        const init_fn_type = c.LLVMFunctionType(c.LLVMVoidTypeInContext(context), null, 0, 0);
        const init_fn = c.LLVMAddFunction(module, "__asthra_init_strings", init_fn_type);
        const init_bb = c.LLVMAppendBasicBlockInContext(context, init_fn, "entry");
        c.LLVMPositionBuilderAtEnd(builder, init_bb);

        const fmt_int = c.LLVMBuildGlobalStringPtr(builder, "%d\n", "fmt_int");
        const fmt_str = c.LLVMBuildGlobalStringPtr(builder, "%s\n", "fmt_str");
        const fmt_float = c.LLVMBuildGlobalStringPtr(builder, "%f\n", "fmt_float");
        const fmt_bool_true = c.LLVMBuildGlobalStringPtr(builder, "true\n", "fmt_bool_true");
        const fmt_bool_false = c.LLVMBuildGlobalStringPtr(builder, "false\n", "fmt_bool_false");

        // Clean up the init function
        _ = c.LLVMBuildRetVoid(builder);
        c.LLVMDeleteFunction(init_fn);

        return .{
            .context = context,
            .module = module,
            .builder = builder,
            .named_values = std.StringHashMap(NamedValue).init(allocator),
            .diagnostics = diagnostics,
            .allocator = allocator,
            .ast = ast,
            .printf_fn = printf_fn,
            .fmt_int = fmt_int,
            .fmt_str = fmt_str,
            .fmt_float = fmt_float,
            .fmt_bool_true = fmt_bool_true,
            .fmt_bool_false = fmt_bool_false,
        };
    }

    pub fn deinit(self: *CodeGen) void {
        c.LLVMDisposeBuilder(self.builder);
        c.LLVMDisposeModule(self.module);
        c.LLVMContextDispose(self.context);
        self.named_values.deinit();
        self.loop_stack.deinit(self.allocator);
    }

    pub fn generate(self: *CodeGen) GenError!void {
        for (self.ast.program.decls.items) |decl| {
            switch (decl.decl) {
                .function => |fn_decl| try self.genFunction(&fn_decl),
            }
        }

        // Verify module
        var err_msg: [*c]u8 = null;
        if (c.LLVMVerifyModule(self.module, c.LLVMReturnStatusAction, &err_msg) != 0) {
            self.diagnostics.report(.@"error", 0, "LLVM module verification failed: {s}", .{
                std.mem.span(err_msg),
            });
            c.LLVMDisposeMessage(err_msg);
            return error.CodeGenError;
        }
    }

    fn genFunction(self: *CodeGen, fn_decl: *const Ast.FnDecl) GenError!void {
        const is_main = std.mem.eql(u8, fn_decl.name, "main");
        const return_type_tag = builtinToTypeTag(fn_decl.return_type);
        const llvm_return_type = if (is_main)
            c.LLVMInt32TypeInContext(self.context)
        else
            self.typeTagToLLVM(return_type_tag);

        var param_types = std.ArrayList(c.LLVMTypeRef){};
        defer param_types.deinit(self.allocator);
        for (fn_decl.params.items) |param| {
            const pt = builtinToTypeTag(param.type_expr);
            try param_types.append(self.allocator, self.typeTagToLLVM(pt));
        }

        const fn_type = c.LLVMFunctionType(llvm_return_type, param_types.items.ptr, @intCast(param_types.items.len), 0);

        // Null-terminate function name
        const name_z = self.allocator.dupeZ(u8, fn_decl.name) catch return error.CodeGenError;
        defer self.allocator.free(name_z);

        const function = c.LLVMAddFunction(self.module, name_z.ptr, fn_type);
        const entry = c.LLVMAppendBasicBlockInContext(self.context, function, "entry");
        c.LLVMPositionBuilderAtEnd(self.builder, entry);

        self.named_values.clearRetainingCapacity();

        for (fn_decl.params.items, 0..) |param, i| {
            const llvm_param = c.LLVMGetParam(function, @intCast(i));
            const pt = builtinToTypeTag(param.type_expr);

            const pname_z = self.allocator.dupeZ(u8, param.name) catch return error.CodeGenError;
            defer self.allocator.free(pname_z);

            const alloca = c.LLVMBuildAlloca(self.builder, self.typeTagToLLVM(pt), pname_z.ptr);
            _ = c.LLVMBuildStore(self.builder, llvm_param, alloca);
            self.named_values.put(param.name, .{
                .alloca = alloca,
                .is_mutable = false,
                .type_tag = pt,
            }) catch {};
        }

        try self.genBlock(&fn_decl.body, is_main);

        const current_bb = c.LLVMGetInsertBlock(self.builder);
        if (c.LLVMGetBasicBlockTerminator(current_bb) == null) {
            if (is_main) {
                _ = c.LLVMBuildRet(self.builder, c.LLVMConstInt(c.LLVMInt32TypeInContext(self.context), 0, 0));
            } else if (return_type_tag == .void_type) {
                _ = c.LLVMBuildRetVoid(self.builder);
            }
        }
    }

    fn genBlock(self: *CodeGen, block: *const Ast.Block, is_main: bool) GenError!void {
        for (block.stmts.items) |stmt| {
            if (c.LLVMGetBasicBlockTerminator(c.LLVMGetInsertBlock(self.builder)) != null) {
                break;
            }
            try self.genStmt(stmt, is_main);
        }
    }

    fn genStmt(self: *CodeGen, stmt: Ast.Stmt, is_main: bool) GenError!void {
        switch (stmt) {
            .return_stmt => |ret| {
                if (is_main) {
                    _ = c.LLVMBuildRet(self.builder, c.LLVMConstInt(c.LLVMInt32TypeInContext(self.context), 0, 0));
                } else if (ret.expr == Ast.null_expr) {
                    _ = c.LLVMBuildRetVoid(self.builder);
                } else {
                    const val = try self.genExpr(ret.expr);
                    _ = c.LLVMBuildRet(self.builder, val.value);
                }
            },
            .var_decl => |vd| {
                const type_tag = builtinToTypeTag(vd.type_expr);
                const llvm_type = self.typeTagToLLVM(type_tag);
                const name_z = self.allocator.dupeZ(u8, vd.name) catch return error.CodeGenError;
                defer self.allocator.free(name_z);
                const alloca = c.LLVMBuildAlloca(self.builder, llvm_type, name_z.ptr);
                const init_val = try self.genExpr(vd.init_expr);
                _ = c.LLVMBuildStore(self.builder, init_val.value, alloca);
                self.named_values.put(vd.name, .{
                    .alloca = alloca,
                    .is_mutable = vd.is_mutable,
                    .type_tag = type_tag,
                }) catch {};
            },
            .assign => |assign| {
                const val = try self.genExpr(assign.value);
                if (self.named_values.get(assign.target)) |nv| {
                    _ = c.LLVMBuildStore(self.builder, val.value, nv.alloca);
                } else {
                    self.diagnostics.report(.@"error", 0, "undefined variable '{s}'", .{assign.target});
                    return error.CodeGenError;
                }
            },
            .if_stmt => |if_s| try self.genIfStmt(if_s, is_main),
            .for_stmt => |for_s| try self.genForStmt(for_s, is_main),
            .expr_stmt => |expr_idx| {
                _ = try self.genExpr(expr_idx);
            },
            .break_stmt => {
                if (self.loop_stack.items.len == 0) {
                    self.diagnostics.report(.@"error", 0, "'break' used outside of a loop", .{});
                    return error.CodeGenError;
                }
                const loop_ctx = self.loop_stack.items[self.loop_stack.items.len - 1];
                _ = c.LLVMBuildBr(self.builder, loop_ctx.break_bb);
            },
            .continue_stmt => {
                if (self.loop_stack.items.len == 0) {
                    self.diagnostics.report(.@"error", 0, "'continue' used outside of a loop", .{});
                    return error.CodeGenError;
                }
                const loop_ctx = self.loop_stack.items[self.loop_stack.items.len - 1];
                _ = c.LLVMBuildBr(self.builder, loop_ctx.continue_bb);
            },
        }
    }

    fn genIfStmt(self: *CodeGen, if_stmt: Ast.IfStmt, is_main: bool) GenError!void {
        const cond = try self.genExpr(if_stmt.condition);
        const function = c.LLVMGetBasicBlockParent(c.LLVMGetInsertBlock(self.builder));

        const then_bb = c.LLVMAppendBasicBlockInContext(self.context, function, "then");
        const else_bb = c.LLVMAppendBasicBlockInContext(self.context, function, "else");
        const merge_bb = c.LLVMAppendBasicBlockInContext(self.context, function, "merge");

        _ = c.LLVMBuildCondBr(self.builder, cond.value, then_bb, if (if_stmt.else_block != null) else_bb else merge_bb);

        c.LLVMPositionBuilderAtEnd(self.builder, then_bb);
        try self.genBlock(if_stmt.then_block, is_main);
        if (c.LLVMGetBasicBlockTerminator(c.LLVMGetInsertBlock(self.builder)) == null) {
            _ = c.LLVMBuildBr(self.builder, merge_bb);
        }

        c.LLVMPositionBuilderAtEnd(self.builder, else_bb);
        if (if_stmt.else_block) |eb| {
            try self.genBlock(eb, is_main);
        }
        if (c.LLVMGetBasicBlockTerminator(c.LLVMGetInsertBlock(self.builder)) == null) {
            _ = c.LLVMBuildBr(self.builder, merge_bb);
        }

        c.LLVMPositionBuilderAtEnd(self.builder, merge_bb);

        // If merge block has no predecessors (both branches terminated), add unreachable
        if (c.LLVMGetFirstUse(c.LLVMBasicBlockAsValue(merge_bb)) == null) {
            _ = c.LLVMBuildUnreachable(self.builder);
        }
    }

    fn genForStmt(self: *CodeGen, for_stmt: Ast.ForStmt, is_main: bool) GenError!void {
        const function = c.LLVMGetBasicBlockParent(c.LLVMGetInsertBlock(self.builder));

        // Extract range(end) or range(start, end) arguments
        const range_info = try self.extractRangeArgs(for_stmt.iterable);

        const iter_name_z = self.allocator.dupeZ(u8, for_stmt.iter_var) catch return error.CodeGenError;
        defer self.allocator.free(iter_name_z);

        const counter_alloca = c.LLVMBuildAlloca(self.builder, c.LLVMInt32TypeInContext(self.context), iter_name_z.ptr);
        _ = c.LLVMBuildStore(self.builder, range_info.start, counter_alloca);

        self.named_values.put(for_stmt.iter_var, .{
            .alloca = counter_alloca,
            .is_mutable = false,
            .type_tag = .i32_type,
        }) catch {};

        const cond_bb = c.LLVMAppendBasicBlockInContext(self.context, function, "for.cond");
        const body_bb = c.LLVMAppendBasicBlockInContext(self.context, function, "for.body");
        const inc_bb = c.LLVMAppendBasicBlockInContext(self.context, function, "for.inc");
        const exit_bb = c.LLVMAppendBasicBlockInContext(self.context, function, "for.exit");

        _ = c.LLVMBuildBr(self.builder, cond_bb);

        c.LLVMPositionBuilderAtEnd(self.builder, cond_bb);
        const current_val = c.LLVMBuildLoad2(self.builder, c.LLVMInt32TypeInContext(self.context), counter_alloca, "i");
        const cond = c.LLVMBuildICmp(self.builder, c.LLVMIntSLT, current_val, range_info.end, "cmp");
        _ = c.LLVMBuildCondBr(self.builder, cond, body_bb, exit_bb);

        try self.loop_stack.append(self.allocator, .{
            .continue_bb = inc_bb,
            .break_bb = exit_bb,
        });
        defer _ = self.loop_stack.pop();

        c.LLVMPositionBuilderAtEnd(self.builder, body_bb);
        try self.genBlock(for_stmt.body, is_main);
        if (c.LLVMGetBasicBlockTerminator(c.LLVMGetInsertBlock(self.builder)) == null) {
            _ = c.LLVMBuildBr(self.builder, inc_bb);
        }

        c.LLVMPositionBuilderAtEnd(self.builder, inc_bb);
        const next_val = c.LLVMBuildAdd(
            self.builder,
            c.LLVMBuildLoad2(self.builder, c.LLVMInt32TypeInContext(self.context), counter_alloca, "i.load"),
            c.LLVMConstInt(c.LLVMInt32TypeInContext(self.context), 1, 0),
            "inc",
        );
        _ = c.LLVMBuildStore(self.builder, next_val, counter_alloca);
        _ = c.LLVMBuildBr(self.builder, cond_bb);

        c.LLVMPositionBuilderAtEnd(self.builder, exit_bb);
    }

    const RangeInfo = struct {
        start: c.LLVMValueRef,
        end: c.LLVMValueRef,
    };

    fn extractRangeArgs(self: *CodeGen, expr_idx: Ast.ExprIndex) GenError!RangeInfo {
        const expr = self.ast.getExpr(expr_idx);
        switch (expr) {
            .call => |call_expr| {
                const callee = self.ast.getExpr(call_expr.callee);
                switch (callee) {
                    .identifier => |name| {
                        if (std.mem.eql(u8, name, "range")) {
                            if (call_expr.args.items.len == 1) {
                                // range(end) -> 0..end
                                const end_val = try self.genExpr(call_expr.args.items[0]);
                                return .{
                                    .start = c.LLVMConstInt(c.LLVMInt32TypeInContext(self.context), 0, 0),
                                    .end = end_val.value,
                                };
                            } else if (call_expr.args.items.len == 2) {
                                // range(start, end)
                                const start_val = try self.genExpr(call_expr.args.items[0]);
                                const end_val = try self.genExpr(call_expr.args.items[1]);
                                return .{
                                    .start = start_val.value,
                                    .end = end_val.value,
                                };
                            }
                        }
                    },
                    else => {},
                }
            },
            else => {},
        }
        self.diagnostics.report(.@"error", 0, "for loop requires range() expression", .{});
        return error.CodeGenError;
    }

    const ExprResult = struct {
        value: c.LLVMValueRef,
        type_tag: TypeTag,
    };

    fn genExpr(self: *CodeGen, expr_idx: Ast.ExprIndex) GenError!ExprResult {
        const expr = self.ast.getExpr(expr_idx);
        switch (expr) {
            .int_literal => |val| {
                return .{
                    .value = c.LLVMConstInt(c.LLVMInt32TypeInContext(self.context), @bitCast(val), 0),
                    .type_tag = .i32_type,
                };
            },
            .float_literal => |val| {
                return .{
                    .value = c.LLVMConstReal(c.LLVMDoubleTypeInContext(self.context), val),
                    .type_tag = .f64_type,
                };
            },
            .bool_literal => |val| {
                return .{
                    .value = c.LLVMConstInt(c.LLVMInt1TypeInContext(self.context), if (val) 1 else 0, 0),
                    .type_tag = .bool_type,
                };
            },
            .string_literal => |val| {
                const str_z = self.allocator.dupeZ(u8, val) catch return error.CodeGenError;
                defer self.allocator.free(str_z);
                return .{
                    .value = c.LLVMBuildGlobalStringPtr(self.builder, str_z.ptr, "str"),
                    .type_tag = .string_type,
                };
            },
            .identifier => |name| {
                if (self.named_values.get(name)) |nv| {
                    const name_z = self.allocator.dupeZ(u8, name) catch return error.CodeGenError;
                    defer self.allocator.free(name_z);
                    const loaded = c.LLVMBuildLoad2(self.builder, self.typeTagToLLVM(nv.type_tag), nv.alloca, name_z.ptr);
                    return .{ .value = loaded, .type_tag = nv.type_tag };
                }
                self.diagnostics.report(.@"error", 0, "undefined variable '{s}'", .{name});
                return error.CodeGenError;
            },
            .binary => |bin| {
                const lhs = try self.genExpr(bin.lhs);
                const rhs = try self.genExpr(bin.rhs);
                return self.genBinaryOp(bin.op, lhs, rhs);
            },
            .unary => |un| {
                const operand = try self.genExpr(un.operand);
                return self.genUnaryOp(un.op, operand);
            },
            .call => |call_expr| {
                return self.genCallExpr(call_expr);
            },
            .grouped => |inner| {
                return self.genExpr(inner);
            },
        }
    }

    fn genBinaryOp(self: *CodeGen, op: Ast.BinaryOp, lhs: ExprResult, rhs: ExprResult) GenError!ExprResult {
        const is_float = lhs.type_tag == .f64_type;
        const value = switch (op) {
            .add => if (is_float) c.LLVMBuildFAdd(self.builder, lhs.value, rhs.value, "fadd") else c.LLVMBuildAdd(self.builder, lhs.value, rhs.value, "add"),
            .sub => if (is_float) c.LLVMBuildFSub(self.builder, lhs.value, rhs.value, "fsub") else c.LLVMBuildSub(self.builder, lhs.value, rhs.value, "sub"),
            .mul => if (is_float) c.LLVMBuildFMul(self.builder, lhs.value, rhs.value, "fmul") else c.LLVMBuildMul(self.builder, lhs.value, rhs.value, "mul"),
            .div => if (is_float) c.LLVMBuildFDiv(self.builder, lhs.value, rhs.value, "fdiv") else c.LLVMBuildSDiv(self.builder, lhs.value, rhs.value, "sdiv"),
            .mod => c.LLVMBuildSRem(self.builder, lhs.value, rhs.value, "mod"),
            .eq => if (is_float) c.LLVMBuildFCmp(self.builder, c.LLVMRealOEQ, lhs.value, rhs.value, "feq") else c.LLVMBuildICmp(self.builder, c.LLVMIntEQ, lhs.value, rhs.value, "eq"),
            .neq => if (is_float) c.LLVMBuildFCmp(self.builder, c.LLVMRealONE, lhs.value, rhs.value, "fne") else c.LLVMBuildICmp(self.builder, c.LLVMIntNE, lhs.value, rhs.value, "ne"),
            .lt => if (is_float) c.LLVMBuildFCmp(self.builder, c.LLVMRealOLT, lhs.value, rhs.value, "flt") else c.LLVMBuildICmp(self.builder, c.LLVMIntSLT, lhs.value, rhs.value, "lt"),
            .le => if (is_float) c.LLVMBuildFCmp(self.builder, c.LLVMRealOLE, lhs.value, rhs.value, "fle") else c.LLVMBuildICmp(self.builder, c.LLVMIntSLE, lhs.value, rhs.value, "le"),
            .gt => if (is_float) c.LLVMBuildFCmp(self.builder, c.LLVMRealOGT, lhs.value, rhs.value, "fgt") else c.LLVMBuildICmp(self.builder, c.LLVMIntSGT, lhs.value, rhs.value, "gt"),
            .ge => if (is_float) c.LLVMBuildFCmp(self.builder, c.LLVMRealOGE, lhs.value, rhs.value, "fge") else c.LLVMBuildICmp(self.builder, c.LLVMIntSGE, lhs.value, rhs.value, "ge"),
            .logic_and => c.LLVMBuildAnd(self.builder, lhs.value, rhs.value, "and"),
            .logic_or => c.LLVMBuildOr(self.builder, lhs.value, rhs.value, "or"),
            .bit_and => c.LLVMBuildAnd(self.builder, lhs.value, rhs.value, "bitand"),
            .bit_or => c.LLVMBuildOr(self.builder, lhs.value, rhs.value, "bitor"),
            .bit_xor => c.LLVMBuildXor(self.builder, lhs.value, rhs.value, "xor"),
            .shift_left => c.LLVMBuildShl(self.builder, lhs.value, rhs.value, "shl"),
            .shift_right => c.LLVMBuildAShr(self.builder, lhs.value, rhs.value, "shr"),
        };

        const result_type: TypeTag = switch (op) {
            .eq, .neq, .lt, .le, .gt, .ge => .bool_type,
            else => lhs.type_tag,
        };

        return .{ .value = value, .type_tag = result_type };
    }

    fn genUnaryOp(self: *CodeGen, op: Ast.UnaryOp, operand: ExprResult) GenError!ExprResult {
        const value = switch (op) {
            .negate => if (operand.type_tag == .f64_type)
                c.LLVMBuildFNeg(self.builder, operand.value, "fneg")
            else
                c.LLVMBuildNeg(self.builder, operand.value, "neg"),
            .logic_not => c.LLVMBuildNot(self.builder, operand.value, "not"),
            .bit_not => c.LLVMBuildNot(self.builder, operand.value, "bitnot"),
        };
        return .{ .value = value, .type_tag = operand.type_tag };
    }

    fn genCallExpr(self: *CodeGen, call_expr: Ast.CallExpr) GenError!ExprResult {
        const callee_expr = self.ast.getExpr(call_expr.callee);
        const name = switch (callee_expr) {
            .identifier => |n| n,
            else => {
                self.diagnostics.report(.@"error", 0, "callee must be an identifier", .{});
                return error.CodeGenError;
            },
        };

        if (std.mem.eql(u8, name, "log")) {
            return self.genLogCall(call_expr);
        }

        if (std.mem.eql(u8, name, "range")) {
            self.diagnostics.report(.@"error", 0, "range() can only be used in for loops", .{});
            return error.CodeGenError;
        }

        const name_z = self.allocator.dupeZ(u8, name) catch return error.CodeGenError;
        defer self.allocator.free(name_z);

        const function = c.LLVMGetNamedFunction(self.module, name_z.ptr);
        if (function == null) {
            self.diagnostics.report(.@"error", 0, "undefined function '{s}'", .{name});
            return error.CodeGenError;
        }

        var args = std.ArrayList(c.LLVMValueRef){};
        defer args.deinit(self.allocator);
        for (call_expr.args.items) |arg_idx| {
            const arg_val = try self.genExpr(arg_idx);
            try args.append(self.allocator, arg_val.value);
        }

        const fn_type = c.LLVMGlobalGetValueType(function);
        const ret_type = c.LLVMGetReturnType(fn_type);
        const is_void = c.LLVMGetTypeKind(ret_type) == c.LLVMVoidTypeKind;

        if (is_void) {
            _ = c.LLVMBuildCall2(self.builder, fn_type, function, args.items.ptr, @intCast(args.items.len), "");
            return .{ .value = c.LLVMGetUndef(c.LLVMVoidTypeInContext(self.context)), .type_tag = .void_type };
        } else {
            const result = c.LLVMBuildCall2(self.builder, fn_type, function, args.items.ptr, @intCast(args.items.len), "call");
            return .{ .value = result, .type_tag = .i32_type };
        }
    }

    fn genLogCall(self: *CodeGen, call_expr: Ast.CallExpr) GenError!ExprResult {
        if (call_expr.args.items.len != 1) {
            self.diagnostics.report(.@"error", 0, "log() takes exactly 1 argument", .{});
            return error.CodeGenError;
        }

        const arg = try self.genExpr(call_expr.args.items[0]);
        const printf_type = c.LLVMGlobalGetValueType(self.printf_fn);

        switch (arg.type_tag) {
            .i32_type, .i64_type => {
                const args_arr = [_]c.LLVMValueRef{ self.fmt_int, arg.value };
                _ = c.LLVMBuildCall2(self.builder, printf_type, self.printf_fn, @constCast(&args_arr), 2, "");
            },
            .f64_type => {
                const args_arr = [_]c.LLVMValueRef{ self.fmt_float, arg.value };
                _ = c.LLVMBuildCall2(self.builder, printf_type, self.printf_fn, @constCast(&args_arr), 2, "");
            },
            .string_type => {
                const args_arr = [_]c.LLVMValueRef{ self.fmt_str, arg.value };
                _ = c.LLVMBuildCall2(self.builder, printf_type, self.printf_fn, @constCast(&args_arr), 2, "");
            },
            .bool_type => {
                const fmt = c.LLVMBuildSelect(self.builder, arg.value, self.fmt_bool_true, self.fmt_bool_false, "bool_fmt");
                const args_arr = [_]c.LLVMValueRef{fmt};
                _ = c.LLVMBuildCall2(self.builder, printf_type, self.printf_fn, @constCast(&args_arr), 1, "");
            },
            .void_type => {
                self.diagnostics.report(.@"error", 0, "cannot log void value", .{});
                return error.CodeGenError;
            },
        }

        return .{ .value = c.LLVMGetUndef(c.LLVMVoidTypeInContext(self.context)), .type_tag = .void_type };
    }

    fn typeTagToLLVM(self: *const CodeGen, tag: TypeTag) c.LLVMTypeRef {
        return switch (tag) {
            .i32_type => c.LLVMInt32TypeInContext(self.context),
            .i64_type => c.LLVMInt64TypeInContext(self.context),
            .f64_type => c.LLVMDoubleTypeInContext(self.context),
            .bool_type => c.LLVMInt1TypeInContext(self.context),
            .void_type => c.LLVMVoidTypeInContext(self.context),
            .string_type => c.LLVMPointerType(c.LLVMInt8TypeInContext(self.context), 0),
        };
    }

    pub fn emitObjectFile(self: *CodeGen, path: []const u8) GenError!void {
        c.LLVMInitializeAllTargetInfos();
        c.LLVMInitializeAllTargets();
        c.LLVMInitializeAllTargetMCs();
        c.LLVMInitializeAllAsmParsers();
        c.LLVMInitializeAllAsmPrinters();

        const triple = c.LLVMGetDefaultTargetTriple();
        defer c.LLVMDisposeMessage(triple);

        var target: c.LLVMTargetRef = null;
        var err_msg: [*c]u8 = null;
        if (c.LLVMGetTargetFromTriple(triple, &target, &err_msg) != 0) {
            self.diagnostics.report(.@"error", 0, "failed to get target: {s}", .{std.mem.span(err_msg)});
            c.LLVMDisposeMessage(err_msg);
            return error.CodeGenError;
        }

        const machine = c.LLVMCreateTargetMachine(target, triple, "generic", "", c.LLVMCodeGenLevelDefault, c.LLVMRelocPIC, c.LLVMCodeModelDefault);
        defer c.LLVMDisposeTargetMachine(machine);

        c.LLVMSetModuleDataLayout(self.module, c.LLVMCreateTargetDataLayout(machine));
        c.LLVMSetTarget(self.module, triple);

        const path_z = self.allocator.dupeZ(u8, path) catch return error.CodeGenError;
        defer self.allocator.free(path_z);

        if (c.LLVMTargetMachineEmitToFile(machine, self.module, path_z.ptr, c.LLVMObjectFile, &err_msg) != 0) {
            self.diagnostics.report(.@"error", 0, "failed to emit object file: {s}", .{std.mem.span(err_msg)});
            c.LLVMDisposeMessage(err_msg);
            return error.CodeGenError;
        }
    }

    pub fn printIR(self: *const CodeGen) void {
        const ir = c.LLVMPrintModuleToString(self.module);
        defer c.LLVMDisposeMessage(ir);
        std.debug.print("{s}\n", .{std.mem.span(ir)});
    }
};

fn builtinToTypeTag(type_expr: Ast.TypeExpr) CodeGen.TypeTag {
    return switch (type_expr) {
        .builtin => |b| switch (b) {
            .void => .void_type,
            .bool_type => .bool_type,
            .i32_type, .int_type => .i32_type,
            .i64_type => .i64_type,
            .f64_type, .float_type => .f64_type,
            .string_type => .string_type,
            else => .i32_type,
        },
        .named => .i32_type,
    };
}
