const std = @import("std");
const Ast = @import("ast.zig").Ast;
const codegen_mod = @import("codegen.zig");
const CodeGen = codegen_mod.CodeGen;
const c = codegen_mod.c;
const codegen_exprs = @import("codegen_exprs.zig");

pub fn genClosureExpr(self: *CodeGen, cl: Ast.ClosureExpr) CodeGen.GenError!CodeGen.ExprResult {
    const codegen_stmts = @import("codegen_stmts.zig");

    // 1. Collect free variables (captured from enclosing scope)
    var captured = std.StringHashMap(CodeGen.NamedValue).init(self.allocator);
    defer captured.deinit();

    var param_names = std.StringHashMap(void).init(self.allocator);
    defer param_names.deinit();
    for (cl.params.items) |param| {
        param_names.put(param.name, {}) catch {};
    }
    collectFreeVars(self, &cl.body, &param_names, &captured);

    // 2. Generate unique closure function name
    const closure_id = self.closure_counter;
    self.closure_counter += 1;
    const fn_name = std.fmt.allocPrint(self.allocator, "__closure_{d}", .{closure_id}) catch return error.CodeGenError;
    self.monomorphized_names.append(self.allocator, fn_name) catch {};

    // 3. Build env struct type from captured variables
    var cap_names = std.ArrayList([]const u8){};
    defer cap_names.deinit(self.allocator);
    var cap_types = std.ArrayList(c.LLVMTypeRef){};
    defer cap_types.deinit(self.allocator);
    var cap_tags = std.ArrayList(CodeGen.TypeTag){};
    defer cap_tags.deinit(self.allocator);

    var cap_iter = captured.iterator();
    while (cap_iter.next()) |entry| {
        cap_names.append(self.allocator, entry.key_ptr.*) catch {};
        cap_types.append(self.allocator, self.typeTagToLLVM(entry.value_ptr.type_tag)) catch {};
        cap_tags.append(self.allocator, entry.value_ptr.type_tag) catch {};
    }

    const has_captures = cap_names.items.len > 0;
    var env_struct_type: c.LLVMTypeRef = undefined;
    if (has_captures) {
        env_struct_type = c.LLVMStructTypeInContext(self.context, cap_types.items.ptr, @intCast(cap_types.items.len), 0);
    }

    // 4. Save codegen state
    const saved_block = c.LLVMGetInsertBlock(self.builder);
    const saved_values = self.named_values;
    self.named_values = std.StringHashMap(CodeGen.NamedValue).init(self.allocator);

    // 5. Build lifted function
    const return_type_tag = self.resolveTypeExpr(cl.return_type);
    const llvm_return_type = self.typeTagToLLVM(return_type_tag);

    var fn_param_types = std.ArrayList(c.LLVMTypeRef){};
    defer fn_param_types.deinit(self.allocator);

    // First param: env pointer (always, for uniform calling convention)
    const i8ptr_ty = c.LLVMPointerType(c.LLVMInt8TypeInContext(self.context), 0);
    if (has_captures) {
        fn_param_types.append(self.allocator, c.LLVMPointerType(env_struct_type, 0)) catch {};
    } else {
        fn_param_types.append(self.allocator, i8ptr_ty) catch {};
    }
    // Then closure declared params
    for (cl.params.items) |param| {
        const pt = self.resolveTypeExpr(param.type_expr);
        fn_param_types.append(self.allocator, self.typeTagToLLVM(pt)) catch {};
    }

    const fn_type = c.LLVMFunctionType(llvm_return_type, fn_param_types.items.ptr, @intCast(fn_param_types.items.len), 0);
    const fn_name_z = self.allocator.dupeZ(u8, fn_name) catch return error.CodeGenError;
    defer self.allocator.free(fn_name_z);
    const function = c.LLVMAddFunction(self.module, fn_name_z.ptr, fn_type);
    const entry_bb = c.LLVMAppendBasicBlockInContext(self.context, function, "entry");
    c.LLVMPositionBuilderAtEnd(self.builder, entry_bb);

    // Load captured vars from env struct (param 0 is always env_ptr)
    const param_offset: u32 = 1;
    if (has_captures) {
        const env_param = c.LLVMGetParam(function, 0);
        for (cap_names.items, 0..) |cap_name, idx| {
            const field_ptr = c.LLVMBuildStructGEP2(self.builder, env_struct_type, env_param, @intCast(idx), "cap_ptr");
            const loaded = c.LLVMBuildLoad2(self.builder, cap_types.items[idx], field_ptr, "cap_val");
            const alloca = c.LLVMBuildAlloca(self.builder, cap_types.items[idx], "cap_alloca");
            _ = c.LLVMBuildStore(self.builder, loaded, alloca);
            self.named_values.put(cap_name, .{
                .alloca = alloca,
                .is_mutable = false,
                .type_tag = cap_tags.items[idx],
            }) catch {};
        }
    }

    // Register closure params
    for (cl.params.items, 0..) |param, i| {
        const llvm_param = c.LLVMGetParam(function, @intCast(i + param_offset));
        const pt = self.resolveTypeExpr(param.type_expr);
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

    // Generate closure body
    codegen_stmts.genBlock(self, &cl.body, false) catch {};

    // Add terminator if missing
    const current_bb = c.LLVMGetInsertBlock(self.builder);
    if (c.LLVMGetBasicBlockTerminator(current_bb) == null) {
        if (CodeGen.isTypeTag(return_type_tag, .void_type)) {
            _ = c.LLVMBuildRetVoid(self.builder);
        }
    }

    // 6. Restore codegen state
    self.named_values.deinit();
    self.named_values = saved_values;
    c.LLVMPositionBuilderAtEnd(self.builder, saved_block);

    // 7. Create env struct and build closure descriptor { fn_ptr, env_ptr }
    var env_ptr_val: c.LLVMValueRef = undefined;
    if (has_captures) {
        const env_alloca = c.LLVMBuildAlloca(self.builder, env_struct_type, "closure_env");
        for (cap_names.items, 0..) |cap_name, idx| {
            const nv = saved_values.get(cap_name).?;
            const cap_val = c.LLVMBuildLoad2(self.builder, cap_types.items[idx], nv.alloca, "cap_load");
            const field_ptr = c.LLVMBuildStructGEP2(self.builder, env_struct_type, env_alloca, @intCast(idx), "env_field");
            _ = c.LLVMBuildStore(self.builder, cap_val, field_ptr);
        }
        env_ptr_val = c.LLVMBuildBitCast(self.builder, env_alloca, i8ptr_ty, "env_ptr");
    } else {
        env_ptr_val = c.LLVMConstNull(i8ptr_ty);
    }

    // Build closure descriptor struct: { fn_ptr: ptr, env_ptr: ptr }
    var desc_field_types = [_]c.LLVMTypeRef{ i8ptr_ty, i8ptr_ty };
    const desc_struct_type = c.LLVMStructTypeInContext(self.context, &desc_field_types, 2, 0);
    const desc_alloca = c.LLVMBuildAlloca(self.builder, desc_struct_type, "closure_desc");

    // Store fn_ptr
    const fn_ptr_raw = c.LLVMBuildBitCast(self.builder, function, i8ptr_ty, "fn_ptr");
    const fn_ptr_field = c.LLVMBuildStructGEP2(self.builder, desc_struct_type, desc_alloca, 0, "fn_ptr_store");
    _ = c.LLVMBuildStore(self.builder, fn_ptr_raw, fn_ptr_field);

    // Store env_ptr
    const env_ptr_field = c.LLVMBuildStructGEP2(self.builder, desc_struct_type, desc_alloca, 1, "env_ptr_store");
    _ = c.LLVMBuildStore(self.builder, env_ptr_val, env_ptr_field);

    // Load descriptor value
    const desc_val = c.LLVMBuildLoad2(self.builder, desc_struct_type, desc_alloca, "closure_desc_val");

    // Build fn_type TypeTag
    const param_tags = self.allocator.alloc(CodeGen.TypeTag, cl.params.items.len) catch return error.CodeGenError;
    for (cl.params.items, 0..) |param, i| {
        param_tags[i] = self.resolveTypeExpr(param.type_expr);
    }
    const ret_tag_ptr = self.allocator.create(CodeGen.TypeTag) catch return error.CodeGenError;
    ret_tag_ptr.* = return_type_tag;

    return .{
        .value = desc_val,
        .type_tag = .{ .fn_type = .{ .param_types = param_tags, .return_type = ret_tag_ptr } },
    };
}

pub fn collectFreeVars(self: *CodeGen, block: *const Ast.Block, param_names: *const std.StringHashMap(void), captured: *std.StringHashMap(CodeGen.NamedValue)) void {
    for (block.stmts.items) |stmt| {
        switch (stmt) {
            .return_stmt => |ret| {
                collectFreeVarsExpr(self, ret.expr, param_names, captured);
            },
            .expr_stmt => |expr_idx| collectFreeVarsExpr(self, expr_idx, param_names, captured),
            .var_decl => |vd| {
                collectFreeVarsExpr(self, vd.init_expr, param_names, captured);
            },
            .assign => |a| {
                collectFreeVarsExpr(self, a.value, param_names, captured);
            },
            .if_stmt => |ifs| {
                collectFreeVarsExpr(self, ifs.condition, param_names, captured);
                collectFreeVars(self, ifs.then_block, param_names, captured);
                if (ifs.else_block) |eb| collectFreeVars(self, eb, param_names, captured);
            },
            .for_stmt => |fs| {
                collectFreeVarsExpr(self, fs.iterable, param_names, captured);
                collectFreeVars(self, fs.body, param_names, captured);
            },
            .while_stmt => |ws| {
                collectFreeVarsExpr(self, ws.condition, param_names, captured);
                collectFreeVars(self, ws.body, param_names, captured);
            },
            else => {},
        }
    }
}

pub fn collectFreeVarsExpr(self: *CodeGen, expr_idx: Ast.ExprIndex, param_names: *const std.StringHashMap(void), captured: *std.StringHashMap(CodeGen.NamedValue)) void {
    if (expr_idx >= self.ast.exprs.items.len) return;
    const expr = self.ast.getExpr(expr_idx);
    switch (expr) {
        .identifier => |name| {
            if (param_names.contains(name)) return;
            if (captured.contains(name)) return;
            if (self.named_values.get(name)) |nv| {
                captured.put(name, nv) catch {};
            }
        },
        .binary => |bin| {
            collectFreeVarsExpr(self, bin.lhs, param_names, captured);
            collectFreeVarsExpr(self, bin.rhs, param_names, captured);
        },
        .unary => |un| {
            collectFreeVarsExpr(self, un.operand, param_names, captured);
        },
        .call => |call| {
            collectFreeVarsExpr(self, call.callee, param_names, captured);
            for (call.args.items) |arg| {
                collectFreeVarsExpr(self, arg, param_names, captured);
            }
        },
        .method_call => |mc| {
            collectFreeVarsExpr(self, mc.object, param_names, captured);
            for (mc.args.items) |arg| {
                collectFreeVarsExpr(self, arg, param_names, captured);
            }
        },
        .field_access => |fa| {
            collectFreeVarsExpr(self, fa.object, param_names, captured);
        },
        .index_access => |ia| {
            collectFreeVarsExpr(self, ia.object, param_names, captured);
            collectFreeVarsExpr(self, ia.index, param_names, captured);
        },
        .grouped => |inner| {
            collectFreeVarsExpr(self, inner, param_names, captured);
        },
        .address_of => |inner| {
            collectFreeVarsExpr(self, inner, param_names, captured);
        },
        .deref => |inner| {
            collectFreeVarsExpr(self, inner, param_names, captured);
        },
        else => {},
    }
}

pub fn genFnTypeCall(self: *CodeGen, nv: CodeGen.NamedValue, ft: CodeGen.FnTypeTag, args: *const std.ArrayList(Ast.ExprIndex)) CodeGen.GenError!CodeGen.ExprResult {
    const i8ptr_ty = c.LLVMPointerType(c.LLVMInt8TypeInContext(self.context), 0);
    var desc_field_types = [_]c.LLVMTypeRef{ i8ptr_ty, i8ptr_ty };
    const desc_struct_type = c.LLVMStructTypeInContext(self.context, &desc_field_types, 2, 0);

    // Extract fn_ptr (field 0) and env_ptr (field 1) from descriptor
    const fn_ptr_field = c.LLVMBuildStructGEP2(self.builder, desc_struct_type, nv.alloca, 0, "fn_ptr_field");
    const fn_ptr_raw = c.LLVMBuildLoad2(self.builder, i8ptr_ty, fn_ptr_field, "fn_ptr");
    const env_ptr_field = c.LLVMBuildStructGEP2(self.builder, desc_struct_type, nv.alloca, 1, "env_ptr_field");
    const env_ptr = c.LLVMBuildLoad2(self.builder, i8ptr_ty, env_ptr_field, "env_ptr");

    // Build the expected LLVM function type: ret(ptr, T1, T2, ...)
    const ret_llvm = self.typeTagToLLVM(ft.return_type.*);
    var fn_param_types = std.ArrayList(c.LLVMTypeRef){};
    defer fn_param_types.deinit(self.allocator);
    fn_param_types.append(self.allocator, i8ptr_ty) catch {}; // env_ptr
    for (ft.param_types) |pt| {
        fn_param_types.append(self.allocator, self.typeTagToLLVM(pt)) catch {};
    }
    const callee_fn_type = c.LLVMFunctionType(ret_llvm, fn_param_types.items.ptr, @intCast(fn_param_types.items.len), 0);

    // Bitcast raw ptr to typed function pointer
    const fn_ptr_typed = c.LLVMBuildBitCast(self.builder, fn_ptr_raw, c.LLVMPointerType(callee_fn_type, 0), "fn_ptr_typed");

    // Build call args: env_ptr, arg1, arg2, ...
    var call_args = std.ArrayList(c.LLVMValueRef){};
    defer call_args.deinit(self.allocator);
    call_args.append(self.allocator, env_ptr) catch {};
    for (args.items) |arg_idx| {
        const arg_val = try codegen_exprs.genExpr(self, arg_idx);
        call_args.append(self.allocator, arg_val.value) catch {};
    }

    const is_void = c.LLVMGetTypeKind(ret_llvm) == c.LLVMVoidTypeKind;
    if (is_void) {
        _ = c.LLVMBuildCall2(self.builder, callee_fn_type, fn_ptr_typed, call_args.items.ptr, @intCast(call_args.items.len), "");
        return .{ .value = c.LLVMGetUndef(c.LLVMVoidTypeInContext(self.context)), .type_tag = .void_type };
    } else {
        const result = c.LLVMBuildCall2(self.builder, callee_fn_type, fn_ptr_typed, call_args.items.ptr, @intCast(call_args.items.len), "fn_call");
        return .{ .value = result, .type_tag = ft.return_type.* };
    }
}

pub fn genClosureCall(self: *CodeGen, ct: CodeGen.ClosureTypeTag, args: *const std.ArrayList(Ast.ExprIndex)) CodeGen.GenError!CodeGen.ExprResult {
    const fn_name_z = self.allocator.dupeZ(u8, ct.fn_name) catch return error.CodeGenError;
    defer self.allocator.free(fn_name_z);
    const function = c.LLVMGetNamedFunction(self.module, fn_name_z.ptr);
    if (function == null) {
        self.diagnostics.report(.@"error", 0, "undefined closure function '{s}'", .{ct.fn_name});
        return error.CodeGenError;
    }

    var call_args = std.ArrayList(c.LLVMValueRef){};
    defer call_args.deinit(self.allocator);

    // Pass env pointer as first arg (if closure has captures)
    const fn_type = c.LLVMGlobalGetValueType(function);
    const fn_param_count = c.LLVMCountParamTypes(fn_type);
    if (fn_param_count > args.items.len) {
        // Has env param
        call_args.append(self.allocator, ct.env_alloca) catch {};
    }

    // Pass user arguments
    for (args.items) |arg_idx| {
        const arg_val = try codegen_exprs.genExpr(self, arg_idx);
        call_args.append(self.allocator, arg_val.value) catch {};
    }

    const ret_llvm = c.LLVMGetReturnType(fn_type);
    const is_void = c.LLVMGetTypeKind(ret_llvm) == c.LLVMVoidTypeKind;

    if (is_void) {
        _ = c.LLVMBuildCall2(self.builder, fn_type, function, call_args.items.ptr, @intCast(call_args.items.len), "");
        return .{ .value = c.LLVMGetUndef(c.LLVMVoidTypeInContext(self.context)), .type_tag = .void_type };
    } else {
        const result = c.LLVMBuildCall2(self.builder, fn_type, function, call_args.items.ptr, @intCast(call_args.items.len), "closure_call");
        return .{ .value = result, .type_tag = ct.return_type.* };
    }
}

/// Helper: call a closure descriptor with LLVM values (not AST ExprIndex)
pub fn callClosureWithValues(self: *CodeGen, desc_alloca: c.LLVMValueRef, ft: CodeGen.FnTypeTag, values: []const c.LLVMValueRef) CodeGen.GenError!c.LLVMValueRef {
    const i8ptr_ty = c.LLVMPointerType(c.LLVMInt8TypeInContext(self.context), 0);
    var desc_field_types = [_]c.LLVMTypeRef{ i8ptr_ty, i8ptr_ty };
    const desc_struct_type = c.LLVMStructTypeInContext(self.context, &desc_field_types, 2, 0);

    const fn_ptr_field = c.LLVMBuildStructGEP2(self.builder, desc_struct_type, desc_alloca, 0, "hof_fn_ptr_f");
    const fn_ptr_raw = c.LLVMBuildLoad2(self.builder, i8ptr_ty, fn_ptr_field, "hof_fn_ptr");
    const env_ptr_field = c.LLVMBuildStructGEP2(self.builder, desc_struct_type, desc_alloca, 1, "hof_env_ptr_f");
    const env_ptr = c.LLVMBuildLoad2(self.builder, i8ptr_ty, env_ptr_field, "hof_env_ptr");

    const ret_llvm = self.typeTagToLLVM(ft.return_type.*);
    var fn_param_types = std.ArrayList(c.LLVMTypeRef){};
    defer fn_param_types.deinit(self.allocator);
    fn_param_types.append(self.allocator, i8ptr_ty) catch {};
    for (ft.param_types) |pt| {
        fn_param_types.append(self.allocator, self.typeTagToLLVM(pt)) catch {};
    }
    const callee_fn_type = c.LLVMFunctionType(ret_llvm, fn_param_types.items.ptr, @intCast(fn_param_types.items.len), 0);
    const fn_ptr_typed = c.LLVMBuildBitCast(self.builder, fn_ptr_raw, c.LLVMPointerType(callee_fn_type, 0), "hof_fn_typed");

    var call_args = std.ArrayList(c.LLVMValueRef){};
    defer call_args.deinit(self.allocator);
    call_args.append(self.allocator, env_ptr) catch {};
    for (values) |v| {
        call_args.append(self.allocator, v) catch {};
    }

    const ret_llvm_type = c.LLVMGetReturnType(callee_fn_type);
    const is_void_ret = c.LLVMGetTypeKind(ret_llvm_type) == c.LLVMVoidTypeKind;
    const call_name: [*c]const u8 = if (is_void_ret) "" else "hof_call";
    return c.LLVMBuildCall2(self.builder, callee_fn_type, fn_ptr_typed, call_args.items.ptr, @intCast(call_args.items.len), call_name);
}
