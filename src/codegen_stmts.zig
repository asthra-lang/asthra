const std = @import("std");
const Ast = @import("ast.zig").Ast;
const codegen_mod = @import("codegen.zig");
const CodeGen = codegen_mod.CodeGen;
const c = codegen_mod.c;
const codegen_control = @import("codegen_control.zig");

pub fn genFunction(self: *CodeGen, fn_decl: *const Ast.FnDecl) CodeGen.GenError!void {
    return genFunctionWithName(self, fn_decl, fn_decl.name);
}

pub fn genFunctionWithName(self: *CodeGen, fn_decl: *const Ast.FnDecl, llvm_name: []const u8) CodeGen.GenError!void {
    const is_main = std.mem.eql(u8, llvm_name, "main");
    const return_type_tag = self.resolveTypeExpr(fn_decl.return_type);
    const llvm_return_type = if (is_main)
        c.LLVMInt32TypeInContext(self.context)
    else
        self.typeTagToLLVM(return_type_tag);

    var param_types = std.ArrayList(c.LLVMTypeRef){};
    defer param_types.deinit(self.allocator);

    // Main gets argc/argv params for C ABI
    if (is_main) {
        try param_types.append(self.allocator, c.LLVMInt32TypeInContext(self.context)); // argc
        const i8ptr = c.LLVMPointerType(c.LLVMInt8TypeInContext(self.context), 0);
        try param_types.append(self.allocator, c.LLVMPointerType(i8ptr, 0)); // argv: char**
    }

    for (fn_decl.params.items) |param| {
        const pt = self.resolveTypeExpr(param.type_expr);
        try param_types.append(self.allocator, self.typeTagToLLVM(pt));
    }

    const fn_type = c.LLVMFunctionType(llvm_return_type, param_types.items.ptr, @intCast(param_types.items.len), 0);

    // Null-terminate function name
    const name_z = self.allocator.dupeZ(u8, llvm_name) catch return error.CodeGenError;
    defer self.allocator.free(name_z);

    const function = c.LLVMAddFunction(self.module, name_z.ptr, fn_type);
    const entry = c.LLVMAppendBasicBlockInContext(self.context, function, "entry");
    c.LLVMPositionBuilderAtEnd(self.builder, entry);

    // Store argc/argv to globals for os.args()
    if (is_main) {
        _ = c.LLVMBuildStore(self.builder, c.LLVMGetParam(function, 0), self.argc_global);
        _ = c.LLVMBuildStore(self.builder, c.LLVMGetParam(function, 1), self.argv_global);
    }

    self.named_values.clearRetainingCapacity();

    for (fn_decl.params.items, 0..) |param, i| {
        const llvm_param = c.LLVMGetParam(function, @intCast(i));
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

    try genBlock(self, &fn_decl.body, is_main);

    const current_bb = c.LLVMGetInsertBlock(self.builder);
    if (c.LLVMGetBasicBlockTerminator(current_bb) == null) {
        if (is_main) {
            _ = c.LLVMBuildRet(self.builder, c.LLVMConstInt(c.LLVMInt32TypeInContext(self.context), 0, 0));
        } else if (CodeGen.isTypeTag(return_type_tag, .void_type)) {
            _ = c.LLVMBuildRetVoid(self.builder);
        }
    }
}

pub fn genBlock(self: *CodeGen, block: *const Ast.Block, is_main: bool) CodeGen.GenError!void {
    for (block.stmts.items) |stmt| {
        if (c.LLVMGetBasicBlockTerminator(c.LLVMGetInsertBlock(self.builder)) != null) {
            break;
        }
        try genStmt(self, stmt, is_main);
    }
}

pub fn genStmt(self: *CodeGen, stmt: Ast.Stmt, is_main: bool) CodeGen.GenError!void {
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
            if (vd.type_expr == .inferred) {
                // Type inference: evaluate init first to determine type
                const init_val = try self.genExpr(vd.init_expr);
                const type_tag = init_val.type_tag;
                const llvm_type = self.typeTagToLLVM(type_tag);
                const name_z = self.allocator.dupeZ(u8, vd.name) catch return error.CodeGenError;
                defer self.allocator.free(name_z);
                const alloca = c.LLVMBuildAlloca(self.builder, llvm_type, name_z.ptr);
                _ = c.LLVMBuildStore(self.builder, init_val.value, alloca);
                self.named_values.put(vd.name, .{
                    .alloca = alloca,
                    .is_mutable = vd.is_mutable,
                    .type_tag = type_tag,
                }) catch {};
            } else {
                // Explicit type: resolve type first (may register Option/Result)
                const type_tag = self.resolveTypeExpr(vd.type_expr);
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
            }
        },
        .assign => |assign| {
            const val = try self.genExpr(assign.value);
            if (self.named_values.get(assign.target)) |nv| {
                // Mutability check: prevent assignment to immutable variables
                if (!nv.is_mutable) {
                    self.diagnostics.report(.@"error", 0, "cannot assign to immutable variable '{s}'", .{assign.target});
                    return error.CodeGenError;
                }
                if (assign.target_index) |idx_expr| {
                    // Indexed assignment: arr[i] = value or slice[i] = value
                    switch (nv.type_tag) {
                        .array_type => {
                            const index_val = try self.genExpr(idx_expr);
                            const array_llvm = self.typeTagToLLVM(nv.type_tag);
                            var gep_idx = [_]c.LLVMValueRef{
                                c.LLVMConstInt(c.LLVMInt64TypeInContext(self.context), 0, 0),
                                index_val.value,
                            };
                            const elem_ptr = c.LLVMBuildGEP2(self.builder, array_llvm, nv.alloca, &gep_idx, 2, "idx_assign_ptr");
                            _ = c.LLVMBuildStore(self.builder, val.value, elem_ptr);
                        },
                        .slice_type => |sl| {
                            const index_val = try self.genExpr(idx_expr);
                            const slice_llvm = self.sliceLLVMType();
                            const ptr_field = c.LLVMBuildStructGEP2(self.builder, slice_llvm, nv.alloca, 0, "slice_ptr_field");
                            const data_ptr = c.LLVMBuildLoad2(self.builder, c.LLVMPointerType(c.LLVMInt8TypeInContext(self.context), 0), ptr_field, "slice_ptr");
                            const elem_llvm = self.typeTagToLLVM(sl.element_type.*);
                            const typed_ptr = c.LLVMBuildBitCast(self.builder, data_ptr, c.LLVMPointerType(elem_llvm, 0), "typed_ptr");
                            var gep_idx = [_]c.LLVMValueRef{index_val.value};
                            const elem_ptr = c.LLVMBuildGEP2(self.builder, elem_llvm, typed_ptr, &gep_idx, 1, "slice_idx_assign_ptr");
                            _ = c.LLVMBuildStore(self.builder, val.value, elem_ptr);
                        },
                        else => {
                            self.diagnostics.report(.@"error", 0, "indexed assignment requires an array or slice", .{});
                            return error.CodeGenError;
                        },
                    }
                } else if (assign.target_fields.items.len > 0) {
                    // Field assignment: obj.field = value
                    var current_ptr = nv.alloca;
                    var current_type_tag = nv.type_tag;

                    for (assign.target_fields.items) |field_name| {
                        switch (current_type_tag) {
                            .struct_type => |struct_name| {
                                if (self.struct_types.get(struct_name)) |info| {
                                    const field_idx = self.findFieldIndex(info, field_name);
                                    if (field_idx) |idx| {
                                        current_ptr = c.LLVMBuildStructGEP2(self.builder, info.llvm_type, current_ptr, @intCast(idx), "field_ptr");
                                        current_type_tag = info.field_types[idx];
                                    } else {
                                        self.diagnostics.report(.@"error", 0, "no field '{s}' on struct '{s}'", .{ field_name, struct_name });
                                        return error.CodeGenError;
                                    }
                                }
                            },
                            else => {
                                self.diagnostics.report(.@"error", 0, "cannot access field on non-struct type", .{});
                                return error.CodeGenError;
                            },
                        }
                    }
                    _ = c.LLVMBuildStore(self.builder, val.value, current_ptr);
                } else {
                    _ = c.LLVMBuildStore(self.builder, val.value, nv.alloca);
                }
            } else {
                self.diagnostics.report(.@"error", 0, "undefined variable '{s}'", .{assign.target});
                return error.CodeGenError;
            }
        },
        .if_stmt => |if_s| try codegen_control.genIfStmt(self, if_s, is_main),
        .if_let_stmt => |il_s| try codegen_control.genIfLetStmt(self, il_s, is_main),
        .for_stmt => |for_s| try codegen_control.genForStmt(self, for_s, is_main),
        .while_stmt => |while_s| try codegen_control.genWhileStmt(self, while_s, is_main),
        .expr_stmt => |expr_idx| {
            _ = try self.genExpr(expr_idx);
        },
        .match_stmt => |match_s| try codegen_control.genMatchStmt(self, match_s, is_main),
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
        .unsafe_block => |block| {
            // For now, unsafe blocks just execute their statements
            // (no safety checks to disable yet)
            try genBlock(self, block, is_main);
        },
        .spawn_stmt => |spawn_s| {
            // Synchronous spawn: just execute the expression (call the function)
            _ = try self.genExpr(spawn_s.expr);
        },
        .deref_assign => |da| {
            const val = try self.genExpr(da.value);
            const target = try self.genExpr(da.target);
            _ = c.LLVMBuildStore(self.builder, val.value, target.value);
        },
        .spawn_handle_stmt => |spawn_h| {
            // Synchronous spawn_with_handle: execute the call and store result in a local
            const result = try self.genExpr(spawn_h.expr);
            const llvm_type = self.typeTagToLLVM(result.type_tag);
            const name_z = self.allocator.dupeZ(u8, spawn_h.handle_name) catch return error.CodeGenError;
            defer self.allocator.free(name_z);
            const alloca = c.LLVMBuildAlloca(self.builder, llvm_type, name_z.ptr);
            _ = c.LLVMBuildStore(self.builder, result.value, alloca);
            self.named_values.put(spawn_h.handle_name, .{
                .alloca = alloca,
                .is_mutable = false,
                .type_tag = result.type_tag,
            }) catch {};
        },
    }
}

pub fn genImplDecl(self: *CodeGen, impl_decl: *const Ast.ImplDecl) CodeGen.GenError!void {
    // If this impl is for a generic struct, store it for lazy monomorphization
    if (self.generic_struct_decls.contains(impl_decl.type_name)) {
        self.generic_impl_decls.put(impl_decl.type_name, impl_decl) catch {};
        // Also generate methods for any already-monomorphized instances
        var it = self.monomorphized_origins.iterator();
        while (it.next()) |entry| {
            if (std.mem.eql(u8, entry.value_ptr.generic_name, impl_decl.type_name)) {
                const sd = self.generic_struct_decls.get(impl_decl.type_name) orelse continue;
                self.genGenericImplMethods(entry.key_ptr.*, impl_decl, sd, entry.value_ptr.type_args) catch {};
            }
        }
        return;
    }
    for (impl_decl.methods.items) |method| {
        try genMethod(self, impl_decl.type_name, &method);
    }
}

pub fn genMethod(self: *CodeGen, type_name: []const u8, method: *const Ast.MethodDecl) CodeGen.GenError!void {
    const return_type_tag = self.resolveTypeExpr(method.return_type);
    const llvm_return_type = self.typeTagToLLVM(return_type_tag);

    var param_types = std.ArrayList(c.LLVMTypeRef){};
    defer param_types.deinit(self.allocator);

    // If method has self, first param is the struct type
    if (method.has_self) {
        const self_type = CodeGen.TypeTag{ .struct_type = type_name };
        try param_types.append(self.allocator, self.typeTagToLLVM(self_type));
    }

    for (method.params.items) |param| {
        const pt = self.resolveTypeExpr(param.type_expr);
        try param_types.append(self.allocator, self.typeTagToLLVM(pt));
    }

    const fn_type = c.LLVMFunctionType(llvm_return_type, param_types.items.ptr, @intCast(param_types.items.len), 0);

    // Mangled name: TypeName_methodName
    const mangled_slice = std.fmt.allocPrint(self.allocator, "{s}_{s}", .{ type_name, method.name }) catch return error.CodeGenError;
    defer self.allocator.free(mangled_slice);
    const mangled = self.allocator.dupeZ(u8, mangled_slice) catch return error.CodeGenError;
    defer self.allocator.free(mangled);

    const function = c.LLVMAddFunction(self.module, mangled.ptr, fn_type);
    const entry = c.LLVMAppendBasicBlockInContext(self.context, function, "entry");
    c.LLVMPositionBuilderAtEnd(self.builder, entry);

    self.named_values.clearRetainingCapacity();

    var param_idx: u32 = 0;
    if (method.has_self) {
        const llvm_param = c.LLVMGetParam(function, 0);
        const self_type = CodeGen.TypeTag{ .struct_type = type_name };
        const alloca = c.LLVMBuildAlloca(self.builder, self.typeTagToLLVM(self_type), "self");
        _ = c.LLVMBuildStore(self.builder, llvm_param, alloca);
        self.named_values.put("self", .{
            .alloca = alloca,
            .is_mutable = false,
            .type_tag = self_type,
        }) catch {};
        param_idx = 1;
    }

    for (method.params.items) |param| {
        const llvm_param = c.LLVMGetParam(function, param_idx);
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
        param_idx += 1;
    }

    try genBlock(self, &method.body, false);

    const current_bb = c.LLVMGetInsertBlock(self.builder);
    if (c.LLVMGetBasicBlockTerminator(current_bb) == null) {
        if (CodeGen.isTypeTag(return_type_tag, .void_type)) {
            _ = c.LLVMBuildRetVoid(self.builder);
        }
    }
}

pub fn genExternDecl(self: *CodeGen, ed: *const Ast.ExternDecl) CodeGen.GenError!void {
    const name_z = self.allocator.dupeZ(u8, ed.name) catch return error.CodeGenError;
    defer self.allocator.free(name_z);

    // Skip if function is already declared (e.g., compiler builtins like strlen, malloc, sprintf)
    if (c.LLVMGetNamedFunction(self.module, name_z.ptr) != null) {
        return;
    }

    const return_type_tag = self.resolveTypeExpr(ed.return_type);
    const llvm_return_type = self.typeTagToLLVM(return_type_tag);

    var param_types = std.ArrayList(c.LLVMTypeRef){};
    defer param_types.deinit(self.allocator);
    for (ed.params.items) |param| {
        const pt = self.resolveTypeExpr(param.type_expr);
        try param_types.append(self.allocator, self.typeTagToLLVM(pt));
    }

    const fn_type = c.LLVMFunctionType(llvm_return_type, param_types.items.ptr, @intCast(param_types.items.len), 0);

    _ = c.LLVMAddFunction(self.module, name_z.ptr, fn_type);
}

pub fn genConstDecl(self: *CodeGen, cd: *const Ast.ConstDecl) CodeGen.GenError!void {
    const type_tag = self.resolveTypeExpr(cd.type_expr);
    const llvm_type = self.typeTagToLLVM(type_tag);

    const name_z = self.allocator.dupeZ(u8, cd.name) catch return error.CodeGenError;
    defer self.allocator.free(name_z);

    // Evaluate the constant initializer to an LLVM constant
    const const_val = evalConstExpr(self, cd.init_expr, type_tag) orelse {
        self.diagnostics.report(.@"error", 0, "constant '{s}' requires a compile-time constant expression", .{cd.name});
        return error.CodeGenError;
    };

    // Create a global constant
    const global = c.LLVMAddGlobal(self.module, llvm_type, name_z.ptr);
    c.LLVMSetInitializer(global, const_val);
    c.LLVMSetGlobalConstant(global, 1);
    c.LLVMSetLinkage(global, c.LLVMPrivateLinkage);

    // Register it in const_values so functions can access it
    self.const_values.put(cd.name, .{
        .alloca = global,
        .is_mutable = false,
        .type_tag = type_tag,
    }) catch {};
}

pub fn evalConstExpr(self: *CodeGen, expr_idx: Ast.ExprIndex, type_tag: CodeGen.TypeTag) ?c.LLVMValueRef {
    const expr = self.ast.getExpr(expr_idx);
    switch (expr) {
        .int_literal => |val| {
            return c.LLVMConstInt(self.typeTagToLLVM(type_tag), @bitCast(val), 0);
        },
        .float_literal => |val| {
            return c.LLVMConstReal(c.LLVMDoubleTypeInContext(self.context), val);
        },
        .bool_literal => |val| {
            return c.LLVMConstInt(c.LLVMInt1TypeInContext(self.context), if (val) 1 else 0, 0);
        },
        .string_literal => |val| {
            // Create a global string constant (doesn't need builder positioned)
            const str_val = c.LLVMConstStringInContext(self.context, val.ptr, @intCast(val.len), 0);
            const str_global = c.LLVMAddGlobal(self.module, c.LLVMTypeOf(str_val), "const_str_data");
            c.LLVMSetInitializer(str_global, str_val);
            c.LLVMSetGlobalConstant(str_global, 1);
            c.LLVMSetLinkage(str_global, c.LLVMPrivateLinkage);
            // GEP to get i8* from [N x i8]*
            var indices = [_]c.LLVMValueRef{
                c.LLVMConstInt(c.LLVMInt32TypeInContext(self.context), 0, 0),
                c.LLVMConstInt(c.LLVMInt32TypeInContext(self.context), 0, 0),
            };
            return c.LLVMConstGEP2(c.LLVMTypeOf(str_val), str_global, &indices, 2);
        },
        .unary => |un| {
            if (un.op == .negate) {
                const inner_expr = self.ast.getExpr(un.operand);
                switch (inner_expr) {
                    .int_literal => |val| {
                        return c.LLVMConstInt(self.typeTagToLLVM(type_tag), @bitCast(-val), 0);
                    },
                    .float_literal => |val| {
                        return c.LLVMConstReal(c.LLVMDoubleTypeInContext(self.context), -val);
                    },
                    else => return null,
                }
            }
            return null;
        },
        .identifier => |name| {
            // Look up previously defined constants
            if (self.const_values.get(name)) |cv| {
                const loaded_type = self.typeTagToLLVM(cv.type_tag);
                // Global constants have an initializer we can use directly
                const init = c.LLVMGetInitializer(cv.alloca);
                if (init != null) return init;
                // Fallback: load from the global (shouldn't happen for constants)
                _ = loaded_type;
            }
            return null;
        },
        .binary => |bin| {
            const lhs_val = evalConstExpr(self, bin.lhs, type_tag) orelse return null;
            const rhs_val = evalConstExpr(self, bin.rhs, type_tag) orelse return null;
            const is_float = CodeGen.isTypeTag(type_tag, .f64_type);
            if (is_float) {
                // Extract float values, compute in Zig, create new constant
                var loses_info: c.LLVMBool = 0;
                const lhs_f = c.LLVMConstRealGetDouble(lhs_val, &loses_info);
                const rhs_f = c.LLVMConstRealGetDouble(rhs_val, &loses_info);
                const result_f: f64 = switch (bin.op) {
                    .add => lhs_f + rhs_f,
                    .sub => lhs_f - rhs_f,
                    .mul => lhs_f * rhs_f,
                    .div => lhs_f / rhs_f,
                    else => return null,
                };
                return c.LLVMConstReal(c.LLVMDoubleTypeInContext(self.context), result_f);
            }
            // Integer constant operations
            return switch (bin.op) {
                .add => c.LLVMConstAdd(lhs_val, rhs_val),
                .sub => c.LLVMConstSub(lhs_val, rhs_val),
                .mul => c.LLVMConstMul(lhs_val, rhs_val),
                .bit_xor => c.LLVMConstXor(lhs_val, rhs_val),
                .div, .mod, .bit_and, .bit_or, .shift_left, .shift_right => {
                    // Extract integer values, compute in Zig, create new constant
                    const lhs_i = c.LLVMConstIntGetSExtValue(lhs_val);
                    const rhs_i = c.LLVMConstIntGetSExtValue(rhs_val);
                    const result_i: i64 = switch (bin.op) {
                        .div => @divTrunc(lhs_i, rhs_i),
                        .mod => @rem(lhs_i, rhs_i),
                        .bit_and => lhs_i & rhs_i,
                        .bit_or => lhs_i | rhs_i,
                        .shift_left => lhs_i << @intCast(rhs_i),
                        .shift_right => lhs_i >> @intCast(rhs_i),
                        else => unreachable,
                    };
                    return c.LLVMConstInt(self.typeTagToLLVM(type_tag), @bitCast(result_i), 0);
                },
                else => null,
            };
        },
        else => return null,
    }
}
