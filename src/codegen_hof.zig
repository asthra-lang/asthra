const std = @import("std");
const Ast = @import("ast.zig").Ast;
const codegen_mod = @import("codegen.zig");
const CodeGen = codegen_mod.CodeGen;
const c = codegen_mod.c;
const codegen_exprs = @import("codegen_exprs.zig");
const codegen_closures = @import("codegen_closures.zig");

// ── Generic Function Monomorphization ─────────────────────────────────────

pub fn monomorphizeGenericFn(self: *CodeGen, gen_fn: *const Ast.FnDecl, call_args: *const std.ArrayList(Ast.ExprIndex)) CodeGen.GenError!c.LLVMValueRef {
    const codegen_stmts = @import("codegen_stmts.zig");
    const codegen_types = @import("codegen_types.zig");

    // Infer type arguments from call arguments by peeking at arg expressions
    // (don't generate code — just determine types)
    var type_map = std.StringHashMap(CodeGen.TypeTag).init(self.allocator);
    defer type_map.deinit();

    var arg_types = std.ArrayList(CodeGen.TypeTag){};
    defer arg_types.deinit(self.allocator);
    for (call_args.items) |arg_idx| {
        const arg_type = inferExprType(self, arg_idx);
        arg_types.append(self.allocator, arg_type) catch {};
    }

    // Match parameter type expressions against actual argument types to infer T
    for (gen_fn.params.items, 0..) |param, i| {
        if (i >= arg_types.items.len) break;
        switch (param.type_expr) {
            .named => |param_type_name| {
                // Check if this is a type parameter
                for (gen_fn.type_params.items) |tp| {
                    if (std.mem.eql(u8, param_type_name, tp.name)) {
                        type_map.put(tp.name, arg_types.items[i]) catch {};
                        break;
                    }
                }
            },
            else => {},
        }
    }

    // Validate trait bounds
    for (gen_fn.type_params.items) |tp| {
        if (tp.bound) |trait_name| {
            const concrete = type_map.get(tp.name) orelse continue;
            const struct_name = switch (concrete) {
                .struct_type => |sn| sn,
                else => {
                    self.diagnostics.report(.@"error", 0, "type does not implement trait '{s}'", .{trait_name});
                    return error.CodeGenError;
                },
            };
            const vtable_key = std.fmt.allocPrint(self.allocator, "{s}_{s}", .{ trait_name, struct_name }) catch return error.CodeGenError;
            defer self.allocator.free(vtable_key);
            if (!self.vtable_globals.contains(vtable_key)) {
                self.diagnostics.report(.@"error", 0, "'{s}' does not implement trait '{s}'", .{ struct_name, trait_name });
                return error.CodeGenError;
            }
        }
    }

    // Build mangled name: fn_name_Type1_Type2
    var mangled_parts = std.ArrayList(u8){};
    defer mangled_parts.deinit(self.allocator);
    mangled_parts.appendSlice(self.allocator, gen_fn.name) catch {};
    for (gen_fn.type_params.items) |tp| {
        mangled_parts.append(self.allocator, '_') catch {};
        const resolved = type_map.get(tp.name) orelse CodeGen.TypeTag.i32_type;
        const type_str = typeTagName(resolved);
        mangled_parts.appendSlice(self.allocator, type_str) catch {};
    }

    const mangled_name = mangled_parts.items;
    const mangled_z = self.allocator.dupeZ(u8, mangled_name) catch return error.CodeGenError;
    defer self.allocator.free(mangled_z);

    // Check if already monomorphized
    const existing = c.LLVMGetNamedFunction(self.module, mangled_z.ptr);
    if (existing != null) return existing;

    // Generate monomorphized function
    const saved_block = c.LLVMGetInsertBlock(self.builder);
    const saved_values = self.named_values;
    self.named_values = std.StringHashMap(CodeGen.NamedValue).init(self.allocator);

    // Resolve return type with substitution
    const ret_tag = codegen_types.resolveTypeExprWithSubst(self, gen_fn.return_type, &type_map);
    const ret_llvm = self.typeTagToLLVM(ret_tag);

    // Build param types with substitution
    var param_types = std.ArrayList(c.LLVMTypeRef){};
    defer param_types.deinit(self.allocator);
    var param_tags = std.ArrayList(CodeGen.TypeTag){};
    defer param_tags.deinit(self.allocator);
    for (gen_fn.params.items) |param| {
        const pt = codegen_types.resolveTypeExprWithSubst(self, param.type_expr, &type_map);
        param_types.append(self.allocator, self.typeTagToLLVM(pt)) catch {};
        param_tags.append(self.allocator, pt) catch {};
    }

    const fn_type = c.LLVMFunctionType(ret_llvm, param_types.items.ptr, @intCast(param_types.items.len), 0);
    const mangled_name_duped = self.allocator.dupe(u8, mangled_name) catch return error.CodeGenError;
    self.monomorphized_names.append(self.allocator, mangled_name_duped) catch {};
    const mangled_name_z = self.allocator.dupeZ(u8, mangled_name) catch return error.CodeGenError;
    defer self.allocator.free(mangled_name_z);
    const function = c.LLVMAddFunction(self.module, mangled_name_z.ptr, fn_type);

    const entry_bb = c.LLVMAppendBasicBlockInContext(self.context, function, "entry");
    c.LLVMPositionBuilderAtEnd(self.builder, entry_bb);

    // Register params
    for (gen_fn.params.items, 0..) |param, i| {
        const llvm_param = c.LLVMGetParam(function, @intCast(i));
        const pt = param_tags.items[i];
        const pname_z = self.allocator.dupeZ(u8, param.name) catch return error.CodeGenError;
        defer self.allocator.free(pname_z);
        const alloca = c.LLVMBuildAlloca(self.builder, param_types.items[i], pname_z.ptr);
        _ = c.LLVMBuildStore(self.builder, llvm_param, alloca);
        self.named_values.put(param.name, .{
            .alloca = alloca,
            .is_mutable = false,
            .type_tag = pt,
        }) catch {};
    }

    // Generate body
    codegen_stmts.genBlock(self, &gen_fn.body, false) catch {};

    // Add terminator if missing
    const current_bb = c.LLVMGetInsertBlock(self.builder);
    if (c.LLVMGetBasicBlockTerminator(current_bb) == null) {
        if (CodeGen.isTypeTag(ret_tag, .void_type)) {
            _ = c.LLVMBuildRetVoid(self.builder);
        }
    }

    // Restore state
    self.named_values.deinit();
    self.named_values = saved_values;
    c.LLVMPositionBuilderAtEnd(self.builder, saved_block);

    // Register return type for the monomorphized function so call sites can look it up
    const mangled_key = self.allocator.dupe(u8, mangled_name) catch return error.CodeGenError;
    self.monomorphized_names.append(self.allocator, mangled_key) catch {};
    self.imported_fn_return_types.put(mangled_key, ret_tag) catch {};

    return function;
}

pub fn inferExprType(self: *CodeGen, expr_idx: Ast.ExprIndex) CodeGen.TypeTag {
    if (expr_idx >= self.ast.exprs.items.len) return .i32_type;
    const expr = self.ast.getExpr(expr_idx);
    return switch (expr) {
        .int_literal => .i32_type,
        .float_literal => .f64_type,
        .bool_literal => .bool_type,
        .char_literal => .char_type,
        .string_literal => .string_type,
        .identifier => |name| blk: {
            if (self.named_values.get(name)) |nv| break :blk nv.type_tag;
            if (self.const_values.get(name)) |cv| break :blk cv.type_tag;
            break :blk .i32_type;
        },
        .call => |call| blk: {
            const callee = self.ast.getExpr(call.callee);
            switch (callee) {
                .identifier => |fn_name| {
                    break :blk codegen_exprs.lookupFunctionReturnType(self, fn_name);
                },
                else => break :blk .i32_type,
            }
        },
        else => .i32_type,
    };
}

pub fn typeTagName(tag: CodeGen.TypeTag) []const u8 {
    return switch (tag) {
        .i8_type => "i8",
        .i16_type => "i16",
        .i32_type => "i32",
        .i64_type => "i64",
        .i128_type => "i128",
        .u8_type => "u8",
        .u16_type => "u16",
        .u32_type => "u32",
        .u64_type => "u64",
        .u128_type => "u128",
        .f64_type => "f64",
        .bool_type => "bool",
        .char_type => "char",
        .string_type => "string",
        .struct_type => |name| name,
        .enum_type => |name| name,
        else => "unknown",
    };
}

// ── Dynamic Dispatch (dyn Trait) ──────────────────────────────────────────

pub fn buildTraitUpcast(self: *CodeGen, arg_val: CodeGen.ExprResult, arg_idx: Ast.ExprIndex, trait_name: []const u8) CodeGen.GenError!c.LLVMValueRef {
    const struct_name = switch (arg_val.type_tag) {
        .struct_type => |sn| sn,
        else => {
            self.diagnostics.report(.@"error", 0, "cannot upcast non-struct to dyn {s}", .{trait_name});
            return error.CodeGenError;
        },
    };

    const i8ptr_ty = c.LLVMPointerType(c.LLVMInt8TypeInContext(self.context), 0);

    // Get the struct alloca for data_ptr
    const arg_expr = self.ast.getExpr(arg_idx);
    const data_ptr = switch (arg_expr) {
        .identifier => |id_name| blk: {
            const nv = self.named_values.get(id_name) orelse {
                self.diagnostics.report(.@"error", 0, "undefined variable for upcast", .{});
                return error.CodeGenError;
            };
            break :blk c.LLVMBuildBitCast(self.builder, nv.alloca, i8ptr_ty, "data_ptr");
        },
        else => blk: {
            // Store value to temp alloca
            const struct_info = self.struct_types.get(struct_name) orelse return error.CodeGenError;
            const tmp = c.LLVMBuildAlloca(self.builder, struct_info.llvm_type, "upcast_tmp");
            _ = c.LLVMBuildStore(self.builder, arg_val.value, tmp);
            break :blk c.LLVMBuildBitCast(self.builder, tmp, i8ptr_ty, "data_ptr");
        },
    };

    // Look up vtable global
    const vtable_key = std.fmt.allocPrint(self.allocator, "{s}_{s}", .{ trait_name, struct_name }) catch return error.CodeGenError;
    defer self.allocator.free(vtable_key);
    const vtable_global = self.vtable_globals.get(vtable_key) orelse {
        self.diagnostics.report(.@"error", 0, "no vtable for {s} implementing {s}", .{ struct_name, trait_name });
        return error.CodeGenError;
    };
    const vtable_ptr = c.LLVMBuildBitCast(self.builder, vtable_global, i8ptr_ty, "vtable_ptr");

    // Build fat pointer { data_ptr, vtable_ptr }
    var fat_fields = [_]c.LLVMTypeRef{ i8ptr_ty, i8ptr_ty };
    const fat_type = c.LLVMStructTypeInContext(self.context, &fat_fields, 2, 0);
    const fat_alloca = c.LLVMBuildAlloca(self.builder, fat_type, "trait_obj");
    const data_field = c.LLVMBuildStructGEP2(self.builder, fat_type, fat_alloca, 0, "data_f");
    _ = c.LLVMBuildStore(self.builder, data_ptr, data_field);
    const vtable_field = c.LLVMBuildStructGEP2(self.builder, fat_type, fat_alloca, 1, "vtable_f");
    _ = c.LLVMBuildStore(self.builder, vtable_ptr, vtable_field);

    return c.LLVMBuildLoad2(self.builder, fat_type, fat_alloca, "trait_obj_val");
}

pub fn genTraitMethodCall(self: *CodeGen, obj: CodeGen.ExprResult, trait_name: []const u8, method_name: []const u8, call_args: *const std.ArrayList(Ast.ExprIndex)) CodeGen.GenError!CodeGen.ExprResult {
    const i8ptr_ty = c.LLVMPointerType(c.LLVMInt8TypeInContext(self.context), 0);

    // Look up trait declaration for method index and signature
    const td = self.trait_decls.get(trait_name) orelse {
        self.diagnostics.report(.@"error", 0, "undefined trait '{s}'", .{trait_name});
        return error.CodeGenError;
    };

    var method_idx: ?usize = null;
    var method_sig: ?Ast.TraitMethodSig = null;
    for (td.methods.items, 0..) |m, i| {
        if (std.mem.eql(u8, m.name, method_name)) {
            method_idx = i;
            method_sig = m;
            break;
        }
    }
    const sig = method_sig orelse {
        self.diagnostics.report(.@"error", 0, "method '{s}' not in trait '{s}'", .{ method_name, trait_name });
        return error.CodeGenError;
    };

    // Fat pointer is stored on stack — get its alloca
    var fat_fields = [_]c.LLVMTypeRef{ i8ptr_ty, i8ptr_ty };
    const fat_type = c.LLVMStructTypeInContext(self.context, &fat_fields, 2, 0);

    // Store the fat pointer value to access via GEP
    const fat_alloca = c.LLVMBuildAlloca(self.builder, fat_type, "dyn_obj");
    _ = c.LLVMBuildStore(self.builder, obj.value, fat_alloca);

    // Extract data_ptr and vtable_ptr
    const data_field = c.LLVMBuildStructGEP2(self.builder, fat_type, fat_alloca, 0, "dyn_data_f");
    const data_ptr = c.LLVMBuildLoad2(self.builder, i8ptr_ty, data_field, "dyn_data");
    const vtable_field = c.LLVMBuildStructGEP2(self.builder, fat_type, fat_alloca, 1, "dyn_vtable_f");
    const vtable_raw = c.LLVMBuildLoad2(self.builder, i8ptr_ty, vtable_field, "dyn_vtable");

    // Build the wrapper function type for this method: ret(i8*, params...)
    const ret_tag = self.resolveTypeExpr(sig.return_type);
    const ret_llvm = self.typeTagToLLVM(ret_tag);

    var fn_param_types = std.ArrayList(c.LLVMTypeRef){};
    defer fn_param_types.deinit(self.allocator);
    fn_param_types.append(self.allocator, i8ptr_ty) catch {}; // self
    for (sig.params.items) |param| {
        const pt = self.resolveTypeExpr(param.type_expr);
        fn_param_types.append(self.allocator, self.typeTagToLLVM(pt)) catch {};
    }
    const wrapper_fn_type = c.LLVMFunctionType(ret_llvm, fn_param_types.items.ptr, @intCast(fn_param_types.items.len), 0);

    // Build vtable struct type (array of function pointers)
    var vtable_field_types = std.ArrayList(c.LLVMTypeRef){};
    defer vtable_field_types.deinit(self.allocator);
    for (td.methods.items) |_| {
        vtable_field_types.append(self.allocator, c.LLVMPointerType(wrapper_fn_type, 0)) catch {};
    }
    const vtable_struct_type = c.LLVMStructTypeInContext(self.context, vtable_field_types.items.ptr, @intCast(td.methods.items.len), 0);

    // Bitcast vtable_raw to vtable struct pointer
    const vtable_typed = c.LLVMBuildBitCast(self.builder, vtable_raw, c.LLVMPointerType(vtable_struct_type, 0), "vtable_typed");

    // GEP to get the function pointer at method_idx
    const fn_ptr_field = c.LLVMBuildStructGEP2(self.builder, vtable_struct_type, vtable_typed, @intCast(method_idx.?), "fn_ptr_field");
    const fn_ptr = c.LLVMBuildLoad2(self.builder, c.LLVMPointerType(wrapper_fn_type, 0), fn_ptr_field, "fn_ptr");

    // Build call args: data_ptr + user args
    var fn_call_args = std.ArrayList(c.LLVMValueRef){};
    defer fn_call_args.deinit(self.allocator);
    fn_call_args.append(self.allocator, data_ptr) catch {};
    for (call_args.items) |arg_idx| {
        const arg_val = try codegen_exprs.genExpr(self, arg_idx);
        fn_call_args.append(self.allocator, arg_val.value) catch {};
    }

    const is_void = c.LLVMGetTypeKind(ret_llvm) == c.LLVMVoidTypeKind;
    if (is_void) {
        _ = c.LLVMBuildCall2(self.builder, wrapper_fn_type, fn_ptr, fn_call_args.items.ptr, @intCast(fn_call_args.items.len), "");
        return .{ .value = c.LLVMGetUndef(c.LLVMVoidTypeInContext(self.context)), .type_tag = .void_type };
    } else {
        const result = c.LLVMBuildCall2(self.builder, wrapper_fn_type, fn_ptr, fn_call_args.items.ptr, @intCast(fn_call_args.items.len), "dyn_call");
        return .{ .value = result, .type_tag = ret_tag };
    }
}

// ── Try Expression ────────────────────────────────────────────────────────

pub fn genTryExpr(self: *CodeGen, inner_idx: Ast.ExprIndex) CodeGen.GenError!CodeGen.ExprResult {
    // Evaluate the inner expression (should return Result<T,E> or Option<T>)
    const inner = try codegen_exprs.genExpr(self, inner_idx);

    const enum_name = switch (inner.type_tag) {
        .enum_type => |name| name,
        else => {
            self.diagnostics.report(.@"error", 0, "try requires Result or Option type", .{});
            return error.CodeGenError;
        },
    };

    const info = self.enum_types.get(enum_name) orelse {
        self.diagnostics.report(.@"error", 0, "undefined enum type for try", .{});
        return error.CodeGenError;
    };

    // Store enum value to access tag and data
    const alloca = c.LLVMBuildAlloca(self.builder, info.llvm_type, "try_val");
    _ = c.LLVMBuildStore(self.builder, inner.value, alloca);

    // Load the tag (field 0): 0 = Ok/Some, 1 = Err/None
    const tag_ptr = c.LLVMBuildStructGEP2(self.builder, info.llvm_type, alloca, 0, "try_tag_ptr");
    const tag_val = c.LLVMBuildLoad2(self.builder, c.LLVMInt32TypeInContext(self.context), tag_ptr, "try_tag");
    const is_ok = c.LLVMBuildICmp(self.builder, c.LLVMIntEQ, tag_val, c.LLVMConstInt(c.LLVMInt32TypeInContext(self.context), 0, 0), "is_ok");

    const current_fn = c.LLVMGetBasicBlockParent(c.LLVMGetInsertBlock(self.builder));
    const ok_bb = c.LLVMAppendBasicBlockInContext(self.context, current_fn, "try.ok");
    const err_bb = c.LLVMAppendBasicBlockInContext(self.context, current_fn, "try.err");
    const cont_bb = c.LLVMAppendBasicBlockInContext(self.context, current_fn, "try.cont");

    _ = c.LLVMBuildCondBr(self.builder, is_ok, ok_bb, err_bb);

    // Error/None branch: early return from enclosing function
    c.LLVMPositionBuilderAtEnd(self.builder, err_bb);
    // Return the entire enum value as-is (propagates error)
    _ = c.LLVMBuildRet(self.builder, inner.value);

    // Ok/Some branch: extract the data (variant 0's data at offset 0)
    c.LLVMPositionBuilderAtEnd(self.builder, ok_bb);
    const data_types = info.variant_data_types[0]; // Ok/Some is variant 0
    if (data_types.len == 0) {
        _ = c.LLVMBuildBr(self.builder, cont_bb);
        c.LLVMPositionBuilderAtEnd(self.builder, cont_bb);
        return .{ .value = c.LLVMGetUndef(c.LLVMVoidTypeInContext(self.context)), .type_tag = .void_type };
    }

    const unwrapped_type = data_types[0];
    const unwrapped_llvm = self.typeTagToLLVM(unwrapped_type);

    // Extract data using byte-offset pattern (same as match arm codegen)
    const data_ptr = c.LLVMBuildStructGEP2(self.builder, info.llvm_type, alloca, 1, "try_data_ptr");
    const data_byte_ptr = c.LLVMBuildBitCast(self.builder, data_ptr, c.LLVMPointerType(c.LLVMInt8TypeInContext(self.context), 0), "try_data_bytes");
    var gep_zero = [_]c.LLVMValueRef{c.LLVMConstInt(c.LLVMInt64TypeInContext(self.context), 0, 0)};
    const field_ptr = c.LLVMBuildGEP2(self.builder, c.LLVMInt8TypeInContext(self.context), data_byte_ptr, &gep_zero, 1, "try_field_ptr");
    const typed_ptr = c.LLVMBuildBitCast(self.builder, field_ptr, c.LLVMPointerType(unwrapped_llvm, 0), "try_typed_ptr");
    const unwrapped_val = c.LLVMBuildLoad2(self.builder, unwrapped_llvm, typed_ptr, "try_unwrapped");

    _ = c.LLVMBuildBr(self.builder, cont_bb);

    // Continue with the unwrapped value
    c.LLVMPositionBuilderAtEnd(self.builder, cont_bb);
    // Use phi to carry the value from ok_bb
    const result_phi = c.LLVMBuildPhi(self.builder, unwrapped_llvm, "try_result");
    c.LLVMAddIncoming(result_phi, @constCast(&[_]c.LLVMValueRef{unwrapped_val}), @constCast(&[_]c.LLVMBasicBlockRef{ok_bb}), 1);

    return .{ .value = result_phi, .type_tag = unwrapped_type };
}

// ── Higher-Order Array Functions ──────────────────────────────────────────

pub fn getArrayAllocaFromArg(self: *CodeGen, arg_idx: Ast.ExprIndex) ?struct { alloca: c.LLVMValueRef, arr: CodeGen.ArrayTypeTag } {
    const expr = self.ast.getExpr(arg_idx);
    switch (expr) {
        .identifier => |name| {
            if (self.named_values.get(name)) |nv| {
                switch (nv.type_tag) {
                    .array_type => |arr| return .{ .alloca = nv.alloca, .arr = arr },
                    else => {},
                }
            }
        },
        else => {},
    }
    return null;
}

pub fn genMapCall(self: *CodeGen, call_expr: Ast.CallExpr) CodeGen.GenError!CodeGen.ExprResult {
    if (call_expr.args.items.len != 2) { self.diagnostics.report(.@"error", 0, "map() expects 2 arguments (array, function)", .{}); return error.CodeGenError; }

    // Get array alloca and type info
    const arr_info = getArrayAllocaFromArg(self, call_expr.args.items[0]) orelse {
        self.diagnostics.report(.@"error", 0, "map() first argument must be an array variable", .{});
        return error.CodeGenError;
    };
    const count = arr_info.arr.count;
    const elem_type = arr_info.arr.element_type.*;
    const elem_llvm = self.typeTagToLLVM(elem_type);
    const input_array_llvm = self.typeTagToLLVM(.{ .array_type = arr_info.arr });

    // Get closure descriptor
    const fn_result = try codegen_exprs.genExpr(self, call_expr.args.items[1]);
    const ft = switch (fn_result.type_tag) {
        .fn_type => |f| f,
        else => { self.diagnostics.report(.@"error", 0, "map() second argument must be a function", .{}); return error.CodeGenError; },
    };
    const out_elem_type = ft.return_type.*;
    const out_elem_llvm = self.typeTagToLLVM(out_elem_type);

    // Store closure descriptor to alloca for GEP access
    const i8ptr_ty = c.LLVMPointerType(c.LLVMInt8TypeInContext(self.context), 0);
    var desc_ft = [_]c.LLVMTypeRef{ i8ptr_ty, i8ptr_ty };
    const desc_type = c.LLVMStructTypeInContext(self.context, &desc_ft, 2, 0);
    const fn_alloca = c.LLVMBuildAlloca(self.builder, desc_type, "map_fn");
    _ = c.LLVMBuildStore(self.builder, fn_result.value, fn_alloca);

    // Allocate output array
    const out_array_llvm = c.LLVMArrayType(out_elem_llvm, count);
    const out_alloca = c.LLVMBuildAlloca(self.builder, out_array_llvm, "map_out");

    const i32_llvm = c.LLVMInt32TypeInContext(self.context);
    const count_val = c.LLVMConstInt(i32_llvm, count, 0);
    const zero = c.LLVMConstInt(i32_llvm, 0, 0);
    const one = c.LLVMConstInt(i32_llvm, 1, 0);

    const current_fn = c.LLVMGetBasicBlockParent(c.LLVMGetInsertBlock(self.builder));
    const loop_header = c.LLVMAppendBasicBlockInContext(self.context, current_fn, "map_loop");
    const loop_body = c.LLVMAppendBasicBlockInContext(self.context, current_fn, "map_body");
    const loop_end = c.LLVMAppendBasicBlockInContext(self.context, current_fn, "map_end");

    _ = c.LLVMBuildBr(self.builder, loop_header);

    // Header: i = phi(0, entry)(i_next, body)
    c.LLVMPositionBuilderAtEnd(self.builder, loop_header);
    const i_phi = c.LLVMBuildPhi(self.builder, i32_llvm, "map_i");
    const done = c.LLVMBuildICmp(self.builder, c.LLVMIntUGE, i_phi, count_val, "map_done");
    _ = c.LLVMBuildCondBr(self.builder, done, loop_end, loop_body);

    // Body: load input[i], call fn, store to output[i]
    c.LLVMPositionBuilderAtEnd(self.builder, loop_body);
    var in_gep = [_]c.LLVMValueRef{ zero, i_phi };
    const in_ptr = c.LLVMBuildGEP2(self.builder, input_array_llvm, arr_info.alloca, &in_gep, 2, "map_in_ptr");
    const in_val = c.LLVMBuildLoad2(self.builder, elem_llvm, in_ptr, "map_in_val");

    const mapped_val = try codegen_closures.callClosureWithValues(self, fn_alloca, ft, &[_]c.LLVMValueRef{in_val});

    var out_gep = [_]c.LLVMValueRef{ zero, i_phi };
    const out_ptr = c.LLVMBuildGEP2(self.builder, out_array_llvm, out_alloca, &out_gep, 2, "map_out_ptr");
    _ = c.LLVMBuildStore(self.builder, mapped_val, out_ptr);

    const i_next = c.LLVMBuildAdd(self.builder, i_phi, one, "map_i_next");
    _ = c.LLVMBuildBr(self.builder, loop_header);

    // Wire phi
    const entry_bb = c.LLVMGetPreviousBasicBlock(loop_header);
    c.LLVMAddIncoming(i_phi, @constCast(&[_]c.LLVMValueRef{zero}), @constCast(&[_]c.LLVMBasicBlockRef{entry_bb}), 1);
    c.LLVMAddIncoming(i_phi, @constCast(&[_]c.LLVMValueRef{i_next}), @constCast(&[_]c.LLVMBasicBlockRef{loop_body}), 1);

    c.LLVMPositionBuilderAtEnd(self.builder, loop_end);
    const result = c.LLVMBuildLoad2(self.builder, out_array_llvm, out_alloca, "map_result");

    const out_elem_ptr = self.type_tag_arena.allocator().create(CodeGen.TypeTag) catch return error.CodeGenError;
    out_elem_ptr.* = out_elem_type;
    return .{ .value = result, .type_tag = .{ .array_type = .{ .element_type = out_elem_ptr, .count = count } } };
}

pub fn genReduceCall(self: *CodeGen, call_expr: Ast.CallExpr) CodeGen.GenError!CodeGen.ExprResult {
    if (call_expr.args.items.len != 3) { self.diagnostics.report(.@"error", 0, "reduce() expects 3 arguments (array, init, function)", .{}); return error.CodeGenError; }

    const arr_info = getArrayAllocaFromArg(self, call_expr.args.items[0]) orelse {
        self.diagnostics.report(.@"error", 0, "reduce() first argument must be an array variable", .{});
        return error.CodeGenError;
    };
    const count = arr_info.arr.count;
    const elem_type = arr_info.arr.element_type.*;
    const elem_llvm = self.typeTagToLLVM(elem_type);
    const input_array_llvm = self.typeTagToLLVM(.{ .array_type = arr_info.arr });

    const init_val = try codegen_exprs.genExpr(self, call_expr.args.items[1]);

    const fn_result = try codegen_exprs.genExpr(self, call_expr.args.items[2]);
    const ft = switch (fn_result.type_tag) {
        .fn_type => |f| f,
        else => { self.diagnostics.report(.@"error", 0, "reduce() third argument must be a function", .{}); return error.CodeGenError; },
    };

    // Store closure descriptor
    const i8ptr_ty = c.LLVMPointerType(c.LLVMInt8TypeInContext(self.context), 0);
    var desc_ft = [_]c.LLVMTypeRef{ i8ptr_ty, i8ptr_ty };
    const desc_type = c.LLVMStructTypeInContext(self.context, &desc_ft, 2, 0);
    const fn_alloca = c.LLVMBuildAlloca(self.builder, desc_type, "reduce_fn");
    _ = c.LLVMBuildStore(self.builder, fn_result.value, fn_alloca);

    const i32_llvm = c.LLVMInt32TypeInContext(self.context);
    const count_val = c.LLVMConstInt(i32_llvm, count, 0);
    const zero = c.LLVMConstInt(i32_llvm, 0, 0);
    const one = c.LLVMConstInt(i32_llvm, 1, 0);

    const current_fn = c.LLVMGetBasicBlockParent(c.LLVMGetInsertBlock(self.builder));
    const loop_header = c.LLVMAppendBasicBlockInContext(self.context, current_fn, "reduce_loop");
    const loop_body = c.LLVMAppendBasicBlockInContext(self.context, current_fn, "reduce_body");
    const loop_end = c.LLVMAppendBasicBlockInContext(self.context, current_fn, "reduce_end");

    _ = c.LLVMBuildBr(self.builder, loop_header);

    // Header: i = phi, acc = phi
    c.LLVMPositionBuilderAtEnd(self.builder, loop_header);
    const i_phi = c.LLVMBuildPhi(self.builder, i32_llvm, "reduce_i");
    const acc_phi = c.LLVMBuildPhi(self.builder, self.typeTagToLLVM(init_val.type_tag), "reduce_acc");
    const done = c.LLVMBuildICmp(self.builder, c.LLVMIntUGE, i_phi, count_val, "reduce_done");
    _ = c.LLVMBuildCondBr(self.builder, done, loop_end, loop_body);

    // Body: load input[i], call fn(acc, elem)
    c.LLVMPositionBuilderAtEnd(self.builder, loop_body);
    var in_gep = [_]c.LLVMValueRef{ zero, i_phi };
    const in_ptr = c.LLVMBuildGEP2(self.builder, input_array_llvm, arr_info.alloca, &in_gep, 2, "reduce_in_ptr");
    const in_val = c.LLVMBuildLoad2(self.builder, elem_llvm, in_ptr, "reduce_in_val");

    const new_acc = try codegen_closures.callClosureWithValues(self, fn_alloca, ft, &[_]c.LLVMValueRef{ acc_phi, in_val });

    const i_next = c.LLVMBuildAdd(self.builder, i_phi, one, "reduce_i_next");
    _ = c.LLVMBuildBr(self.builder, loop_header);

    // Wire phis
    const entry_bb = c.LLVMGetPreviousBasicBlock(loop_header);
    c.LLVMAddIncoming(i_phi, @constCast(&[_]c.LLVMValueRef{zero}), @constCast(&[_]c.LLVMBasicBlockRef{entry_bb}), 1);
    c.LLVMAddIncoming(i_phi, @constCast(&[_]c.LLVMValueRef{i_next}), @constCast(&[_]c.LLVMBasicBlockRef{loop_body}), 1);
    c.LLVMAddIncoming(acc_phi, @constCast(&[_]c.LLVMValueRef{init_val.value}), @constCast(&[_]c.LLVMBasicBlockRef{entry_bb}), 1);
    c.LLVMAddIncoming(acc_phi, @constCast(&[_]c.LLVMValueRef{new_acc}), @constCast(&[_]c.LLVMBasicBlockRef{loop_body}), 1);

    c.LLVMPositionBuilderAtEnd(self.builder, loop_end);
    return .{ .value = acc_phi, .type_tag = init_val.type_tag };
}

pub fn genFilterCall(self: *CodeGen, call_expr: Ast.CallExpr) CodeGen.GenError!CodeGen.ExprResult {
    if (call_expr.args.items.len != 2) { self.diagnostics.report(.@"error", 0, "filter() expects 2 arguments (array, predicate)", .{}); return error.CodeGenError; }

    const arr_info = getArrayAllocaFromArg(self, call_expr.args.items[0]) orelse {
        self.diagnostics.report(.@"error", 0, "filter() first argument must be an array variable", .{});
        return error.CodeGenError;
    };
    const count = arr_info.arr.count;
    const elem_type = arr_info.arr.element_type.*;
    const elem_llvm = self.typeTagToLLVM(elem_type);
    const input_array_llvm = self.typeTagToLLVM(.{ .array_type = arr_info.arr });
    const elem_size = c.LLVMABISizeOfType(c.LLVMGetModuleDataLayout(self.module), elem_llvm);

    const fn_result = try codegen_exprs.genExpr(self, call_expr.args.items[1]);
    const ft = switch (fn_result.type_tag) {
        .fn_type => |f| f,
        else => { self.diagnostics.report(.@"error", 0, "filter() second argument must be a function", .{}); return error.CodeGenError; },
    };

    // Store closure descriptor
    const i8ptr_ty = c.LLVMPointerType(c.LLVMInt8TypeInContext(self.context), 0);
    var desc_ft = [_]c.LLVMTypeRef{ i8ptr_ty, i8ptr_ty };
    const desc_type = c.LLVMStructTypeInContext(self.context, &desc_ft, 2, 0);
    const fn_alloca = c.LLVMBuildAlloca(self.builder, desc_type, "filter_fn");
    _ = c.LLVMBuildStore(self.builder, fn_result.value, fn_alloca);

    // Allocate max-size buffer via malloc
    const i32_llvm = c.LLVMInt32TypeInContext(self.context);
    const i64_llvm = c.LLVMInt64TypeInContext(self.context);
    const buf_size = c.LLVMConstInt(i64_llvm, @as(u64, count) * elem_size, 0);
    const malloc_fn_type = c.LLVMGlobalGetValueType(self.malloc_fn);
    var malloc_args = [_]c.LLVMValueRef{buf_size};
    const buf = c.LLVMBuildCall2(self.builder, malloc_fn_type, self.malloc_fn, &malloc_args, 1, "filter_buf");
    const typed_buf = c.LLVMBuildBitCast(self.builder, buf, c.LLVMPointerType(elem_llvm, 0), "filter_typed_buf");

    const count_val = c.LLVMConstInt(i32_llvm, count, 0);
    const zero = c.LLVMConstInt(i32_llvm, 0, 0);
    const one = c.LLVMConstInt(i32_llvm, 1, 0);

    const current_fn = c.LLVMGetBasicBlockParent(c.LLVMGetInsertBlock(self.builder));
    const loop_header = c.LLVMAppendBasicBlockInContext(self.context, current_fn, "filter_loop");
    const loop_body = c.LLVMAppendBasicBlockInContext(self.context, current_fn, "filter_body");
    const store_bb = c.LLVMAppendBasicBlockInContext(self.context, current_fn, "filter_store");
    const skip_bb = c.LLVMAppendBasicBlockInContext(self.context, current_fn, "filter_skip");
    const loop_end = c.LLVMAppendBasicBlockInContext(self.context, current_fn, "filter_end");

    _ = c.LLVMBuildBr(self.builder, loop_header);

    // Header: i = phi, out_count = phi
    c.LLVMPositionBuilderAtEnd(self.builder, loop_header);
    const i_phi = c.LLVMBuildPhi(self.builder, i32_llvm, "filter_i");
    const j_phi = c.LLVMBuildPhi(self.builder, i32_llvm, "filter_j");
    const done = c.LLVMBuildICmp(self.builder, c.LLVMIntUGE, i_phi, count_val, "filter_done");
    _ = c.LLVMBuildCondBr(self.builder, done, loop_end, loop_body);

    // Body: load input[i], call predicate
    c.LLVMPositionBuilderAtEnd(self.builder, loop_body);
    var in_gep = [_]c.LLVMValueRef{ c.LLVMConstInt(i32_llvm, 0, 0), i_phi };
    const in_ptr = c.LLVMBuildGEP2(self.builder, input_array_llvm, arr_info.alloca, &in_gep, 2, "filter_in_ptr");
    const in_val = c.LLVMBuildLoad2(self.builder, elem_llvm, in_ptr, "filter_in_val");

    const pred_result = try codegen_closures.callClosureWithValues(self, fn_alloca, ft, &[_]c.LLVMValueRef{in_val});
    _ = c.LLVMBuildCondBr(self.builder, pred_result, store_bb, skip_bb);

    // Store: buf[j] = in_val, j_next = j + 1
    c.LLVMPositionBuilderAtEnd(self.builder, store_bb);
    var out_gep = [_]c.LLVMValueRef{j_phi};
    const out_ptr = c.LLVMBuildGEP2(self.builder, elem_llvm, typed_buf, &out_gep, 1, "filter_out_ptr");
    _ = c.LLVMBuildStore(self.builder, in_val, out_ptr);
    const j_inc = c.LLVMBuildAdd(self.builder, j_phi, one, "filter_j_inc");
    _ = c.LLVMBuildBr(self.builder, skip_bb);

    // Skip: merge j values
    c.LLVMPositionBuilderAtEnd(self.builder, skip_bb);
    const j_merged = c.LLVMBuildPhi(self.builder, i32_llvm, "filter_j_merged");
    c.LLVMAddIncoming(j_merged, @constCast(&[_]c.LLVMValueRef{ j_inc, j_phi }), @constCast(&[_]c.LLVMBasicBlockRef{ store_bb, loop_body }), 2);
    const i_next = c.LLVMBuildAdd(self.builder, i_phi, one, "filter_i_next");
    _ = c.LLVMBuildBr(self.builder, loop_header);

    // Wire header phis
    const entry_bb = c.LLVMGetPreviousBasicBlock(loop_header);
    c.LLVMAddIncoming(i_phi, @constCast(&[_]c.LLVMValueRef{ zero, i_next }), @constCast(&[_]c.LLVMBasicBlockRef{ entry_bb, skip_bb }), 2);
    c.LLVMAddIncoming(j_phi, @constCast(&[_]c.LLVMValueRef{ zero, j_merged }), @constCast(&[_]c.LLVMBasicBlockRef{ entry_bb, skip_bb }), 2);

    // Build slice struct { ptr, len }
    c.LLVMPositionBuilderAtEnd(self.builder, loop_end);
    const slice_llvm = self.sliceLLVMType();
    const slice_alloca = c.LLVMBuildAlloca(self.builder, slice_llvm, "filter_slice");
    const ptr_field = c.LLVMBuildStructGEP2(self.builder, slice_llvm, slice_alloca, 0, "filter_ptr_f");
    _ = c.LLVMBuildStore(self.builder, buf, ptr_field);
    const len_field = c.LLVMBuildStructGEP2(self.builder, slice_llvm, slice_alloca, 1, "filter_len_f");
    _ = c.LLVMBuildStore(self.builder, j_phi, len_field);
    const slice_val = c.LLVMBuildLoad2(self.builder, slice_llvm, slice_alloca, "filter_result");

    const elem_ptr = self.type_tag_arena.allocator().create(CodeGen.TypeTag) catch return error.CodeGenError;
    elem_ptr.* = elem_type;
    return .{ .value = slice_val, .type_tag = .{ .slice_type = .{ .element_type = elem_ptr } } };
}

pub fn genForEachCall(self: *CodeGen, call_expr: Ast.CallExpr) CodeGen.GenError!CodeGen.ExprResult {
    if (call_expr.args.items.len != 2) { self.diagnostics.report(.@"error", 0, "forEach() expects 2 arguments (array, function)", .{}); return error.CodeGenError; }

    const arr_info = getArrayAllocaFromArg(self, call_expr.args.items[0]) orelse {
        self.diagnostics.report(.@"error", 0, "forEach() first argument must be an array variable", .{});
        return error.CodeGenError;
    };
    const count = arr_info.arr.count;
    const elem_type = arr_info.arr.element_type.*;
    const elem_llvm = self.typeTagToLLVM(elem_type);
    const input_array_llvm = self.typeTagToLLVM(.{ .array_type = arr_info.arr });

    const fn_result = try codegen_exprs.genExpr(self, call_expr.args.items[1]);
    const ft = switch (fn_result.type_tag) {
        .fn_type => |f| f,
        else => { self.diagnostics.report(.@"error", 0, "forEach() second argument must be a function", .{}); return error.CodeGenError; },
    };

    const i8ptr_ty = c.LLVMPointerType(c.LLVMInt8TypeInContext(self.context), 0);
    var desc_ft = [_]c.LLVMTypeRef{ i8ptr_ty, i8ptr_ty };
    const desc_type = c.LLVMStructTypeInContext(self.context, &desc_ft, 2, 0);
    const fn_alloca = c.LLVMBuildAlloca(self.builder, desc_type, "forEach_fn");
    _ = c.LLVMBuildStore(self.builder, fn_result.value, fn_alloca);

    const i32_llvm = c.LLVMInt32TypeInContext(self.context);
    const count_val = c.LLVMConstInt(i32_llvm, count, 0);
    const zero = c.LLVMConstInt(i32_llvm, 0, 0);
    const one = c.LLVMConstInt(i32_llvm, 1, 0);

    const current_fn = c.LLVMGetBasicBlockParent(c.LLVMGetInsertBlock(self.builder));
    const loop_header = c.LLVMAppendBasicBlockInContext(self.context, current_fn, "forEach_loop");
    const loop_body = c.LLVMAppendBasicBlockInContext(self.context, current_fn, "forEach_body");
    const loop_end = c.LLVMAppendBasicBlockInContext(self.context, current_fn, "forEach_end");

    _ = c.LLVMBuildBr(self.builder, loop_header);

    c.LLVMPositionBuilderAtEnd(self.builder, loop_header);
    const i_phi = c.LLVMBuildPhi(self.builder, i32_llvm, "forEach_i");
    const done = c.LLVMBuildICmp(self.builder, c.LLVMIntUGE, i_phi, count_val, "forEach_done");
    _ = c.LLVMBuildCondBr(self.builder, done, loop_end, loop_body);

    c.LLVMPositionBuilderAtEnd(self.builder, loop_body);
    var in_gep = [_]c.LLVMValueRef{ zero, i_phi };
    const in_ptr = c.LLVMBuildGEP2(self.builder, input_array_llvm, arr_info.alloca, &in_gep, 2, "forEach_in_ptr");
    const in_val = c.LLVMBuildLoad2(self.builder, elem_llvm, in_ptr, "forEach_in_val");

    _ = try codegen_closures.callClosureWithValues(self, fn_alloca, ft, &[_]c.LLVMValueRef{in_val});

    const i_next = c.LLVMBuildAdd(self.builder, i_phi, one, "forEach_i_next");
    _ = c.LLVMBuildBr(self.builder, loop_header);

    const entry_bb = c.LLVMGetPreviousBasicBlock(loop_header);
    c.LLVMAddIncoming(i_phi, @constCast(&[_]c.LLVMValueRef{zero}), @constCast(&[_]c.LLVMBasicBlockRef{entry_bb}), 1);
    c.LLVMAddIncoming(i_phi, @constCast(&[_]c.LLVMValueRef{i_next}), @constCast(&[_]c.LLVMBasicBlockRef{loop_body}), 1);

    c.LLVMPositionBuilderAtEnd(self.builder, loop_end);
    return .{ .value = c.LLVMGetUndef(c.LLVMVoidTypeInContext(self.context)), .type_tag = .void_type };
}
