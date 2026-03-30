const std = @import("std");
const Ast = @import("ast.zig").Ast;
const codegen_mod = @import("codegen.zig");
const CodeGen = codegen_mod.CodeGen;
const c = codegen_mod.c;
const codegen_stmts = @import("codegen_stmts.zig");

pub fn genIfStmt(self: *CodeGen, if_stmt: Ast.IfStmt, is_main: bool) CodeGen.GenError!void {
    const cond = try self.genExpr(if_stmt.condition);
    const function = c.LLVMGetBasicBlockParent(c.LLVMGetInsertBlock(self.builder));

    const then_bb = c.LLVMAppendBasicBlockInContext(self.context, function, "then");
    const else_bb = c.LLVMAppendBasicBlockInContext(self.context, function, "else");
    const merge_bb = c.LLVMAppendBasicBlockInContext(self.context, function, "merge");

    _ = c.LLVMBuildCondBr(self.builder, cond.value, then_bb, if (if_stmt.else_block != null) else_bb else merge_bb);

    c.LLVMPositionBuilderAtEnd(self.builder, then_bb);
    try codegen_stmts.genBlock(self, if_stmt.then_block, is_main);
    if (c.LLVMGetBasicBlockTerminator(c.LLVMGetInsertBlock(self.builder)) == null) {
        _ = c.LLVMBuildBr(self.builder, merge_bb);
    }

    c.LLVMPositionBuilderAtEnd(self.builder, else_bb);
    if (if_stmt.else_block) |eb| {
        try codegen_stmts.genBlock(self, eb, is_main);
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

            try codegen_stmts.genBlock(self, il_stmt.then_block, is_main);
            if (c.LLVMGetBasicBlockTerminator(c.LLVMGetInsertBlock(self.builder)) == null) {
                _ = c.LLVMBuildBr(self.builder, merge_bb);
            }

            // Else block
            c.LLVMPositionBuilderAtEnd(self.builder, else_bb);
            if (il_stmt.else_block) |eb| {
                try codegen_stmts.genBlock(self, eb, is_main);
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
    try codegen_stmts.genBlock(self, for_stmt.body, is_main);
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
    try codegen_stmts.genBlock(self, while_stmt.body, is_main);
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

    try codegen_stmts.genBlock(self, for_stmt.body, is_main);
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

    try codegen_stmts.genBlock(self, for_stmt.body, is_main);
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
        try codegen_stmts.genBlock(self, wa.body, is_main);
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

                try codegen_stmts.genBlock(self, arm.body, is_main);
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

                try codegen_stmts.genBlock(self, arm.body, is_main);
                if (c.LLVMGetBasicBlockTerminator(c.LLVMGetInsertBlock(self.builder)) == null) {
                    _ = c.LLVMBuildBr(self.builder, merge_bb);
                }
                // First matching tuple pattern wins — skip remaining arms
                c.LLVMPositionBuilderAtEnd(self.builder, merge_bb);
                return;
            },
            .wildcard, .identifier => {
                // Catch-all: always matches
                try codegen_stmts.genBlock(self, arm.body, is_main);
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
