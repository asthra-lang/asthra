const std = @import("std");
const Ast = @import("ast.zig").Ast;
const codegen_mod = @import("codegen.zig");
const CodeGen = codegen_mod.CodeGen;
const c = codegen_mod.c;

pub fn genFunction(self: *CodeGen, fn_decl: *const Ast.FnDecl) CodeGen.GenError!void {
    const is_main = std.mem.eql(u8, fn_decl.name, "main");
    const return_type_tag = self.resolveTypeExpr(fn_decl.return_type);
    const llvm_return_type = if (is_main)
        c.LLVMInt32TypeInContext(self.context)
    else
        self.typeTagToLLVM(return_type_tag);

    var param_types = std.ArrayList(c.LLVMTypeRef){};
    defer param_types.deinit(self.allocator);
    for (fn_decl.params.items) |param| {
        const pt = self.resolveTypeExpr(param.type_expr);
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
        .if_stmt => |if_s| try genIfStmt(self, if_s, is_main),
        .if_let_stmt => |il_s| try genIfLetStmt(self, il_s, is_main),
        .for_stmt => |for_s| try genForStmt(self, for_s, is_main),
        .while_stmt => |while_s| try genWhileStmt(self, while_s, is_main),
        .expr_stmt => |expr_idx| {
            _ = try self.genExpr(expr_idx);
        },
        .match_stmt => |match_s| try genMatchStmt(self, match_s, is_main),
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

pub fn genIfStmt(self: *CodeGen, if_stmt: Ast.IfStmt, is_main: bool) CodeGen.GenError!void {
    const cond = try self.genExpr(if_stmt.condition);
    const function = c.LLVMGetBasicBlockParent(c.LLVMGetInsertBlock(self.builder));

    const then_bb = c.LLVMAppendBasicBlockInContext(self.context, function, "then");
    const else_bb = c.LLVMAppendBasicBlockInContext(self.context, function, "else");
    const merge_bb = c.LLVMAppendBasicBlockInContext(self.context, function, "merge");

    _ = c.LLVMBuildCondBr(self.builder, cond.value, then_bb, if (if_stmt.else_block != null) else_bb else merge_bb);

    c.LLVMPositionBuilderAtEnd(self.builder, then_bb);
    try genBlock(self, if_stmt.then_block, is_main);
    if (c.LLVMGetBasicBlockTerminator(c.LLVMGetInsertBlock(self.builder)) == null) {
        _ = c.LLVMBuildBr(self.builder, merge_bb);
    }

    c.LLVMPositionBuilderAtEnd(self.builder, else_bb);
    if (if_stmt.else_block) |eb| {
        try genBlock(self, eb, is_main);
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

pub fn genIfLetStmt(self: *CodeGen, il_stmt: Ast.IfLetStmt, is_main: bool) CodeGen.GenError!void {
    // Evaluate the expression (should be an enum type like Option or Result)
    const match_val = try self.genExpr(il_stmt.expr);
    const function = c.LLVMGetBasicBlockParent(c.LLVMGetInsertBlock(self.builder));

    // Get enum info
    const enum_name = switch (match_val.type_tag) {
        .enum_type => |name| name,
        else => {
            self.diagnostics.report(.@"error", 0, "if let requires enum type", .{});
            return error.CodeGenError;
        },
    };

    const info = self.enum_types.get(enum_name) orelse {
        self.diagnostics.report(.@"error", 0, "undefined enum type", .{});
        return error.CodeGenError;
    };

    // Store the enum value to access tag and data
    const alloca = c.LLVMBuildAlloca(self.builder, info.llvm_type, "if_let_val");
    _ = c.LLVMBuildStore(self.builder, match_val.value, alloca);

    // Load the tag
    const tag_ptr = c.LLVMBuildStructGEP2(self.builder, info.llvm_type, alloca, 0, "tag_ptr");
    const tag_val = c.LLVMBuildLoad2(self.builder, c.LLVMInt32TypeInContext(self.context), tag_ptr, "tag");

    // The pattern must be an enum_pattern
    switch (il_stmt.pattern) {
        .enum_pattern => |ep| {
            // Find variant index
            var found_idx: ?usize = null;
            for (info.variant_names, 0..) |vn, i| {
                if (std.mem.eql(u8, vn, ep.variant_name)) {
                    found_idx = i;
                    break;
                }
            }
            const vidx = found_idx orelse {
                self.diagnostics.report(.@"error", 0, "undefined variant '{s}'", .{ep.variant_name});
                return error.CodeGenError;
            };

            // Compare tag with variant index
            const cmp = c.LLVMBuildICmp(self.builder, c.LLVMIntEQ, tag_val, c.LLVMConstInt(c.LLVMInt32TypeInContext(self.context), @intCast(vidx), 0), "if_let_cmp");

            const then_bb = c.LLVMAppendBasicBlockInContext(self.context, function, "if_let.then");
            const else_bb = c.LLVMAppendBasicBlockInContext(self.context, function, "if_let.else");
            const merge_bb = c.LLVMAppendBasicBlockInContext(self.context, function, "if_let.merge");

            _ = c.LLVMBuildCondBr(self.builder, cmp, then_bb, if (il_stmt.else_block != null) else_bb else merge_bb);

            // Then block: extract bindings and execute body
            c.LLVMPositionBuilderAtEnd(self.builder, then_bb);

            const data_types = info.variant_data_types[vidx];
            if (ep.bindings.items.len > 0 and data_types.len > 0) {
                const data_ptr = c.LLVMBuildStructGEP2(self.builder, info.llvm_type, alloca, 1, "data_ptr");
                const data_byte_ptr = c.LLVMBuildBitCast(self.builder, data_ptr, c.LLVMPointerType(c.LLVMInt8TypeInContext(self.context), 0), "data_bytes");

                var offset: u64 = 0;
                for (ep.bindings.items, 0..) |binding_name, bi| {
                    if (bi >= data_types.len) break;
                    const field_type = self.typeTagToLLVM(data_types[bi]);
                    var gep_idx = [_]c.LLVMValueRef{c.LLVMConstInt(c.LLVMInt64TypeInContext(self.context), offset, 0)};
                    const field_ptr = c.LLVMBuildGEP2(
                        self.builder,
                        c.LLVMInt8TypeInContext(self.context),
                        data_byte_ptr,
                        &gep_idx,
                        1,
                        "bind_byte_ptr",
                    );
                    const typed_ptr = c.LLVMBuildBitCast(self.builder, field_ptr, c.LLVMPointerType(field_type, 0), "bind_typed_ptr");
                    const loaded = c.LLVMBuildLoad2(self.builder, field_type, typed_ptr, "bind_val");

                    const bind_name_z = self.allocator.dupeZ(u8, binding_name) catch return error.CodeGenError;
                    defer self.allocator.free(bind_name_z);

                    const bind_alloca = c.LLVMBuildAlloca(self.builder, field_type, bind_name_z.ptr);
                    _ = c.LLVMBuildStore(self.builder, loaded, bind_alloca);
                    self.named_values.put(binding_name, .{
                        .alloca = bind_alloca,
                        .is_mutable = false,
                        .type_tag = data_types[bi],
                    }) catch {};

                    offset += 8;
                }
            }

            try genBlock(self, il_stmt.then_block, is_main);
            if (c.LLVMGetBasicBlockTerminator(c.LLVMGetInsertBlock(self.builder)) == null) {
                _ = c.LLVMBuildBr(self.builder, merge_bb);
            }

            // Else block
            c.LLVMPositionBuilderAtEnd(self.builder, else_bb);
            if (il_stmt.else_block) |eb| {
                try genBlock(self, eb, is_main);
            }
            if (c.LLVMGetBasicBlockTerminator(c.LLVMGetInsertBlock(self.builder)) == null) {
                _ = c.LLVMBuildBr(self.builder, merge_bb);
            }

            c.LLVMPositionBuilderAtEnd(self.builder, merge_bb);

            // If merge block has no predecessors, add unreachable
            if (c.LLVMGetFirstUse(c.LLVMBasicBlockAsValue(merge_bb)) == null) {
                _ = c.LLVMBuildUnreachable(self.builder);
            }
        },
        .identifier, .wildcard => {
            self.diagnostics.report(.@"error", 0, "if let requires enum pattern", .{});
            return error.CodeGenError;
        },
        .tuple_pattern => {
            self.diagnostics.report(.@"error", 0, "if let requires enum pattern, not tuple pattern", .{});
            return error.CodeGenError;
        },
    }
}

pub fn genForStmt(self: *CodeGen, for_stmt: Ast.ForStmt, is_main: bool) CodeGen.GenError!void {
    const function = c.LLVMGetBasicBlockParent(c.LLVMGetInsertBlock(self.builder));

    // Check if the iterable is an array or slice variable
    const iter_expr = self.ast.getExpr(for_stmt.iterable);
    switch (iter_expr) {
        .identifier => |name| {
            if (self.named_values.get(name)) |nv| {
                switch (nv.type_tag) {
                    .array_type => |arr| {
                        return genForArray(self, for_stmt, arr, nv.alloca, function, is_main);
                    },
                    .slice_type => |sl| {
                        return genForSlice(self, for_stmt, sl, nv.alloca, function, is_main);
                    },
                    else => {},
                }
            }
        },
        else => {},
    }

    // Fall back to range() iteration
    const range_info = try extractRangeArgs(self, for_stmt.iterable);

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
    try genBlock(self, for_stmt.body, is_main);
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

pub fn genWhileStmt(self: *CodeGen, while_stmt: Ast.WhileStmt, is_main: bool) CodeGen.GenError!void {
    const function = c.LLVMGetBasicBlockParent(c.LLVMGetInsertBlock(self.builder));

    const cond_bb = c.LLVMAppendBasicBlockInContext(self.context, function, "while.cond");
    const body_bb = c.LLVMAppendBasicBlockInContext(self.context, function, "while.body");
    const exit_bb = c.LLVMAppendBasicBlockInContext(self.context, function, "while.exit");

    _ = c.LLVMBuildBr(self.builder, cond_bb);

    // Condition block
    c.LLVMPositionBuilderAtEnd(self.builder, cond_bb);
    const cond = try self.genExpr(while_stmt.condition);
    _ = c.LLVMBuildCondBr(self.builder, cond.value, body_bb, exit_bb);

    // Push loop context for break/continue
    try self.loop_stack.append(self.allocator, .{
        .continue_bb = cond_bb,
        .break_bb = exit_bb,
    });
    defer _ = self.loop_stack.pop();

    // Body block
    c.LLVMPositionBuilderAtEnd(self.builder, body_bb);
    try genBlock(self, while_stmt.body, is_main);
    if (c.LLVMGetBasicBlockTerminator(c.LLVMGetInsertBlock(self.builder)) == null) {
        _ = c.LLVMBuildBr(self.builder, cond_bb);
    }

    c.LLVMPositionBuilderAtEnd(self.builder, exit_bb);
}

pub fn genForArray(self: *CodeGen, for_stmt: Ast.ForStmt, arr: CodeGen.ArrayTypeTag, array_alloca: c.LLVMValueRef, function: c.LLVMValueRef, is_main: bool) CodeGen.GenError!void {
    const elem_type = arr.element_type.*;
    const elem_llvm = self.typeTagToLLVM(elem_type);
    const array_llvm = self.typeTagToLLVM(.{ .array_type = arr });
    const count = arr.count;

    // Counter variable (hidden index)
    const counter_alloca = c.LLVMBuildAlloca(self.builder, c.LLVMInt32TypeInContext(self.context), "for_idx");
    _ = c.LLVMBuildStore(self.builder, c.LLVMConstInt(c.LLVMInt32TypeInContext(self.context), 0, 0), counter_alloca);

    // Iteration variable (the element)
    const iter_name_z = self.allocator.dupeZ(u8, for_stmt.iter_var) catch return error.CodeGenError;
    defer self.allocator.free(iter_name_z);
    const iter_alloca = c.LLVMBuildAlloca(self.builder, elem_llvm, iter_name_z.ptr);

    self.named_values.put(for_stmt.iter_var, .{
        .alloca = iter_alloca,
        .is_mutable = false,
        .type_tag = elem_type,
    }) catch {};

    const cond_bb = c.LLVMAppendBasicBlockInContext(self.context, function, "forarr.cond");
    const body_bb = c.LLVMAppendBasicBlockInContext(self.context, function, "forarr.body");
    const inc_bb = c.LLVMAppendBasicBlockInContext(self.context, function, "forarr.inc");
    const exit_bb = c.LLVMAppendBasicBlockInContext(self.context, function, "forarr.exit");

    _ = c.LLVMBuildBr(self.builder, cond_bb);

    c.LLVMPositionBuilderAtEnd(self.builder, cond_bb);
    const idx_val = c.LLVMBuildLoad2(self.builder, c.LLVMInt32TypeInContext(self.context), counter_alloca, "idx");
    const cond = c.LLVMBuildICmp(self.builder, c.LLVMIntSLT, idx_val, c.LLVMConstInt(c.LLVMInt32TypeInContext(self.context), count, 0), "arr_cmp");
    _ = c.LLVMBuildCondBr(self.builder, cond, body_bb, exit_bb);

    try self.loop_stack.append(self.allocator, .{
        .continue_bb = inc_bb,
        .break_bb = exit_bb,
    });
    defer _ = self.loop_stack.pop();

    c.LLVMPositionBuilderAtEnd(self.builder, body_bb);
    // Load the current element from the array
    const cur_idx = c.LLVMBuildLoad2(self.builder, c.LLVMInt32TypeInContext(self.context), counter_alloca, "cur_idx");
    var gep_idx = [_]c.LLVMValueRef{
        c.LLVMConstInt(c.LLVMInt64TypeInContext(self.context), 0, 0),
        cur_idx,
    };
    const elem_ptr = c.LLVMBuildGEP2(self.builder, array_llvm, array_alloca, &gep_idx, 2, "arr_elem_ptr");
    const elem_val = c.LLVMBuildLoad2(self.builder, elem_llvm, elem_ptr, "arr_elem");
    _ = c.LLVMBuildStore(self.builder, elem_val, iter_alloca);

    try genBlock(self, for_stmt.body, is_main);
    if (c.LLVMGetBasicBlockTerminator(c.LLVMGetInsertBlock(self.builder)) == null) {
        _ = c.LLVMBuildBr(self.builder, inc_bb);
    }

    c.LLVMPositionBuilderAtEnd(self.builder, inc_bb);
    const next_val = c.LLVMBuildAdd(
        self.builder,
        c.LLVMBuildLoad2(self.builder, c.LLVMInt32TypeInContext(self.context), counter_alloca, "idx.load"),
        c.LLVMConstInt(c.LLVMInt32TypeInContext(self.context), 1, 0),
        "arr_inc",
    );
    _ = c.LLVMBuildStore(self.builder, next_val, counter_alloca);
    _ = c.LLVMBuildBr(self.builder, cond_bb);

    c.LLVMPositionBuilderAtEnd(self.builder, exit_bb);
}

pub fn genForSlice(self: *CodeGen, for_stmt: Ast.ForStmt, sl: CodeGen.SliceTypeTag, slice_alloca: c.LLVMValueRef, function: c.LLVMValueRef, is_main: bool) CodeGen.GenError!void {
    const elem_type = sl.element_type.*;
    const elem_llvm = self.typeTagToLLVM(elem_type);
    const slice_llvm = self.sliceLLVMType();

    // Counter variable (hidden index)
    const counter_alloca = c.LLVMBuildAlloca(self.builder, c.LLVMInt32TypeInContext(self.context), "for_idx");
    _ = c.LLVMBuildStore(self.builder, c.LLVMConstInt(c.LLVMInt32TypeInContext(self.context), 0, 0), counter_alloca);

    // Load the slice length
    const len_ptr = c.LLVMBuildStructGEP2(self.builder, slice_llvm, slice_alloca, 1, "slice_len_ptr");
    const len_val = c.LLVMBuildLoad2(self.builder, c.LLVMInt32TypeInContext(self.context), len_ptr, "slice_len");

    // Load the slice data pointer
    const ptr_field = c.LLVMBuildStructGEP2(self.builder, slice_llvm, slice_alloca, 0, "slice_ptr_field");
    const data_ptr = c.LLVMBuildLoad2(self.builder, c.LLVMPointerType(c.LLVMInt8TypeInContext(self.context), 0), ptr_field, "slice_ptr");
    const typed_data_ptr = c.LLVMBuildBitCast(self.builder, data_ptr, c.LLVMPointerType(elem_llvm, 0), "typed_data_ptr");

    // Iteration variable (the element)
    const iter_name_z = self.allocator.dupeZ(u8, for_stmt.iter_var) catch return error.CodeGenError;
    defer self.allocator.free(iter_name_z);
    const iter_alloca = c.LLVMBuildAlloca(self.builder, elem_llvm, iter_name_z.ptr);

    self.named_values.put(for_stmt.iter_var, .{
        .alloca = iter_alloca,
        .is_mutable = false,
        .type_tag = elem_type,
    }) catch {};

    const cond_bb = c.LLVMAppendBasicBlockInContext(self.context, function, "forslice.cond");
    const body_bb = c.LLVMAppendBasicBlockInContext(self.context, function, "forslice.body");
    const inc_bb = c.LLVMAppendBasicBlockInContext(self.context, function, "forslice.inc");
    const exit_bb = c.LLVMAppendBasicBlockInContext(self.context, function, "forslice.exit");

    _ = c.LLVMBuildBr(self.builder, cond_bb);

    c.LLVMPositionBuilderAtEnd(self.builder, cond_bb);
    const idx_val = c.LLVMBuildLoad2(self.builder, c.LLVMInt32TypeInContext(self.context), counter_alloca, "idx");
    const cond = c.LLVMBuildICmp(self.builder, c.LLVMIntSLT, idx_val, len_val, "slice_cmp");
    _ = c.LLVMBuildCondBr(self.builder, cond, body_bb, exit_bb);

    try self.loop_stack.append(self.allocator, .{
        .continue_bb = inc_bb,
        .break_bb = exit_bb,
    });
    defer _ = self.loop_stack.pop();

    c.LLVMPositionBuilderAtEnd(self.builder, body_bb);
    // Load the current element from the slice
    const cur_idx = c.LLVMBuildLoad2(self.builder, c.LLVMInt32TypeInContext(self.context), counter_alloca, "cur_idx");
    var gep_idx = [_]c.LLVMValueRef{cur_idx};
    const elem_ptr = c.LLVMBuildGEP2(self.builder, elem_llvm, typed_data_ptr, &gep_idx, 1, "slice_elem_ptr");
    const elem_val = c.LLVMBuildLoad2(self.builder, elem_llvm, elem_ptr, "slice_elem");
    _ = c.LLVMBuildStore(self.builder, elem_val, iter_alloca);

    try genBlock(self, for_stmt.body, is_main);
    if (c.LLVMGetBasicBlockTerminator(c.LLVMGetInsertBlock(self.builder)) == null) {
        _ = c.LLVMBuildBr(self.builder, inc_bb);
    }

    c.LLVMPositionBuilderAtEnd(self.builder, inc_bb);
    const next_val = c.LLVMBuildAdd(
        self.builder,
        c.LLVMBuildLoad2(self.builder, c.LLVMInt32TypeInContext(self.context), counter_alloca, "idx.load"),
        c.LLVMConstInt(c.LLVMInt32TypeInContext(self.context), 1, 0),
        "slice_inc",
    );
    _ = c.LLVMBuildStore(self.builder, next_val, counter_alloca);
    _ = c.LLVMBuildBr(self.builder, cond_bb);

    c.LLVMPositionBuilderAtEnd(self.builder, exit_bb);
}

pub fn genMatchStmt(self: *CodeGen, match_stmt: Ast.MatchStmt, is_main: bool) CodeGen.GenError!void {
    const match_val = try self.genExpr(match_stmt.expr);
    const function = c.LLVMGetBasicBlockParent(c.LLVMGetInsertBlock(self.builder));

    // Handle tuple match separately
    switch (match_val.type_tag) {
        .tuple_type => return genTupleMatch(self, match_stmt, match_val, function, is_main),
        else => {},
    }

    // Get enum info
    const enum_name = switch (match_val.type_tag) {
        .enum_type => |name| name,
        else => {
            self.diagnostics.report(.@"error", 0, "match requires enum or tuple type", .{});
            return error.CodeGenError;
        },
    };

    const info = self.enum_types.get(enum_name) orelse {
        self.diagnostics.report(.@"error", 0, "undefined enum type", .{});
        return error.CodeGenError;
    };

    // Store the enum value to access tag and data
    const alloca = c.LLVMBuildAlloca(self.builder, info.llvm_type, "match_val");
    _ = c.LLVMBuildStore(self.builder, match_val.value, alloca);

    // Load the tag
    const tag_ptr = c.LLVMBuildStructGEP2(self.builder, info.llvm_type, alloca, 0, "tag_ptr");
    const tag_val = c.LLVMBuildLoad2(self.builder, c.LLVMInt32TypeInContext(self.context), tag_ptr, "tag");

    const merge_bb = c.LLVMAppendBasicBlockInContext(self.context, function, "match.end");

    // Check if there's a wildcard or identifier catch-all arm
    var wildcard_arm: ?Ast.MatchArm = null;
    for (match_stmt.arms.items) |arm| {
        switch (arm.pattern) {
            .wildcard, .identifier => {
                wildcard_arm = arm;
                break;
            },
            else => {},
        }
    }

    // Build switch — default goes to wildcard arm body if present, otherwise to merge
    const default_bb = c.LLVMAppendBasicBlockInContext(self.context, function, "match.default");
    const switch_inst = c.LLVMBuildSwitch(self.builder, tag_val, default_bb, @intCast(match_stmt.arms.items.len));

    // Generate default block
    c.LLVMPositionBuilderAtEnd(self.builder, default_bb);
    if (wildcard_arm) |wa| {
        try genBlock(self, wa.body, is_main);
        if (c.LLVMGetBasicBlockTerminator(c.LLVMGetInsertBlock(self.builder)) == null) {
            _ = c.LLVMBuildBr(self.builder, merge_bb);
        }
    } else {
        _ = c.LLVMBuildBr(self.builder, merge_bb);
    }

    for (match_stmt.arms.items) |arm| {
        switch (arm.pattern) {
            .enum_pattern => |ep| {
                // Find variant index
                var found_idx: ?usize = null;
                for (info.variant_names, 0..) |vn, i| {
                    if (std.mem.eql(u8, vn, ep.variant_name)) {
                        found_idx = i;
                        break;
                    }
                }
                const vidx = found_idx orelse {
                    self.diagnostics.report(.@"error", 0, "undefined variant '{s}'", .{ep.variant_name});
                    return error.CodeGenError;
                };

                const arm_bb = c.LLVMAppendBasicBlockInContext(self.context, function, "match.arm");
                c.LLVMAddCase(switch_inst, c.LLVMConstInt(c.LLVMInt32TypeInContext(self.context), @intCast(vidx), 0), arm_bb);

                c.LLVMPositionBuilderAtEnd(self.builder, arm_bb);

                // Extract bindings if any
                const data_types = info.variant_data_types[vidx];
                if (ep.bindings.items.len > 0 and data_types.len > 0) {
                    const data_ptr = c.LLVMBuildStructGEP2(self.builder, info.llvm_type, alloca, 1, "data_ptr");
                    const data_byte_ptr = c.LLVMBuildBitCast(self.builder, data_ptr, c.LLVMPointerType(c.LLVMInt8TypeInContext(self.context), 0), "data_bytes");

                    var offset: u64 = 0;
                    for (ep.bindings.items, 0..) |binding_name, bi| {
                        if (bi >= data_types.len) break;
                        const field_type = self.typeTagToLLVM(data_types[bi]);
                        var gep_idx = [_]c.LLVMValueRef{c.LLVMConstInt(c.LLVMInt64TypeInContext(self.context), offset, 0)};
                        const field_ptr = c.LLVMBuildGEP2(
                            self.builder,
                            c.LLVMInt8TypeInContext(self.context),
                            data_byte_ptr,
                            &gep_idx,
                            1,
                            "bind_byte_ptr",
                        );
                        const typed_ptr = c.LLVMBuildBitCast(self.builder, field_ptr, c.LLVMPointerType(field_type, 0), "bind_typed_ptr");
                        const loaded = c.LLVMBuildLoad2(self.builder, field_type, typed_ptr, "bind_val");

                        // Create local alloca for the binding
                        const bind_name_z = self.allocator.dupeZ(u8, binding_name) catch return error.CodeGenError;
                        defer self.allocator.free(bind_name_z);

                        const bind_alloca = c.LLVMBuildAlloca(self.builder, field_type, bind_name_z.ptr);
                        _ = c.LLVMBuildStore(self.builder, loaded, bind_alloca);
                        self.named_values.put(binding_name, .{
                            .alloca = bind_alloca,
                            .is_mutable = false,
                            .type_tag = data_types[bi],
                        }) catch {};

                        offset += 8;
                    }
                }

                try genBlock(self, arm.body, is_main);
                if (c.LLVMGetBasicBlockTerminator(c.LLVMGetInsertBlock(self.builder)) == null) {
                    _ = c.LLVMBuildBr(self.builder, merge_bb);
                }
            },
            .identifier, .wildcard => {
                // Catch-all patterns are handled as the default arm above
                continue;
            },
            .tuple_pattern => {
                self.diagnostics.report(.@"error", 0, "tuple pattern not valid in enum match", .{});
                return error.CodeGenError;
            },
        }
    }

    c.LLVMPositionBuilderAtEnd(self.builder, merge_bb);
}

pub fn genTupleMatch(self: *CodeGen, match_stmt: Ast.MatchStmt, match_val: CodeGen.ExprResult, function: c.LLVMValueRef, is_main: bool) CodeGen.GenError!void {
    const tt = match_val.type_tag.tuple_type;

    // Store the tuple value so we can GEP into it
    const tuple_alloca = c.LLVMBuildAlloca(self.builder, tt.llvm_type, "match_tuple");
    _ = c.LLVMBuildStore(self.builder, match_val.value, tuple_alloca);

    const merge_bb = c.LLVMAppendBasicBlockInContext(self.context, function, "tuple_match.end");

    // Tuple patterns with only identifiers/wildcards always match,
    // so we process arms sequentially — the first matching arm wins.
    // Currently only identifier and wildcard sub-patterns are supported
    // (no literal comparison), so the first arm always matches.
    for (match_stmt.arms.items) |arm| {
        switch (arm.pattern) {
            .tuple_pattern => |tp| {
                if (tp.elements.items.len != tt.element_types.len) {
                    self.diagnostics.report(.@"error", 0, "tuple pattern has {d} elements but tuple has {d}", .{ tp.elements.items.len, tt.element_types.len });
                    return error.CodeGenError;
                }

                // Bind each element
                for (tp.elements.items, 0..) |elem_pat, i| {
                    switch (elem_pat) {
                        .identifier => |name| {
                            const elem_type = tt.element_types[i];
                            const elem_llvm = self.typeTagToLLVM(elem_type);
                            const field_ptr = c.LLVMBuildStructGEP2(self.builder, tt.llvm_type, tuple_alloca, @intCast(i), "tup_pat_ptr");
                            const loaded = c.LLVMBuildLoad2(self.builder, elem_llvm, field_ptr, "tup_pat_val");

                            const name_z = self.allocator.dupeZ(u8, name) catch return error.CodeGenError;
                            defer self.allocator.free(name_z);

                            const bind_alloca = c.LLVMBuildAlloca(self.builder, elem_llvm, name_z.ptr);
                            _ = c.LLVMBuildStore(self.builder, loaded, bind_alloca);
                            self.named_values.put(name, .{
                                .alloca = bind_alloca,
                                .is_mutable = false,
                                .type_tag = elem_type,
                            }) catch {};
                        },
                        .wildcard => {
                            // Wildcard: don't bind anything
                        },
                        else => {
                            self.diagnostics.report(.@"error", 0, "unsupported sub-pattern in tuple pattern", .{});
                            return error.CodeGenError;
                        },
                    }
                }

                try genBlock(self, arm.body, is_main);
                if (c.LLVMGetBasicBlockTerminator(c.LLVMGetInsertBlock(self.builder)) == null) {
                    _ = c.LLVMBuildBr(self.builder, merge_bb);
                }
                // First matching tuple pattern wins — skip remaining arms
                c.LLVMPositionBuilderAtEnd(self.builder, merge_bb);
                return;
            },
            .wildcard, .identifier => {
                // Catch-all: always matches
                try genBlock(self, arm.body, is_main);
                if (c.LLVMGetBasicBlockTerminator(c.LLVMGetInsertBlock(self.builder)) == null) {
                    _ = c.LLVMBuildBr(self.builder, merge_bb);
                }
                c.LLVMPositionBuilderAtEnd(self.builder, merge_bb);
                return;
            },
            .enum_pattern => {
                self.diagnostics.report(.@"error", 0, "enum pattern not valid in tuple match", .{});
                return error.CodeGenError;
            },
        }
    }

    c.LLVMPositionBuilderAtEnd(self.builder, merge_bb);
}

pub fn extractRangeArgs(self: *CodeGen, expr_idx: Ast.ExprIndex) CodeGen.GenError!CodeGen.RangeInfo {
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
