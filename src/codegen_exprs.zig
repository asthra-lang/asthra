const std = @import("std");
const Ast = @import("ast.zig").Ast;
const codegen_mod = @import("codegen.zig");
const CodeGen = codegen_mod.CodeGen;
const c = codegen_mod.c;
const codegen_stdlib = @import("codegen_stdlib.zig");
const codegen_closures = @import("codegen_closures.zig");
const codegen_hof = @import("codegen_hof.zig");

pub fn genExpr(self: *CodeGen, expr_idx: Ast.ExprIndex) CodeGen.GenError!CodeGen.ExprResult {
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
        .char_literal => |val| {
            return .{
                .value = c.LLVMConstInt(c.LLVMInt8TypeInContext(self.context), val, 0),
                .type_tag = .char_type,
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
            const nv = self.named_values.get(name) orelse self.const_values.get(name);
            if (nv) |v| {
                const name_z = self.allocator.dupeZ(u8, name) catch return error.CodeGenError;
                defer self.allocator.free(name_z);
                const loaded = c.LLVMBuildLoad2(self.builder, self.typeTagToLLVM(v.type_tag), v.alloca, name_z.ptr);
                return .{ .value = loaded, .type_tag = v.type_tag };
            }
            self.diagnostics.report(.@"error", 0, "undefined variable '{s}'", .{name});
            return error.CodeGenError;
        },
        .binary => |bin| {
            const lhs = try genExpr(self, bin.lhs);
            const rhs = try genExpr(self, bin.rhs);
            return genBinaryOp(self, bin.op, lhs, rhs);
        },
        .unary => |un| {
            const operand = try genExpr(self, un.operand);
            return genUnaryOp(self, un.op, operand);
        },
        .call => |call_expr| {
            return genCallExpr(self, call_expr);
        },
        .grouped => |inner| {
            return genExpr(self, inner);
        },
        .field_access => |fa| {
            return genFieldAccess(self, fa);
        },
        .struct_literal => |sl| {
            return genStructLiteral(self, sl);
        },
        .method_call => |mc| {
            return genMethodCall(self, mc);
        },
        .associated_call => |ac| {
            return genAssociatedCall(self, ac);
        },
        .enum_constructor => |ec| {
            return self.genEnumConstructor(ec.enum_name, ec.variant_name, &ec.args);
        },
        .array_literal => |al| {
            return genArrayLiteral(self, al);
        },
        .repeated_array => |ra| {
            return genRepeatedArray(self, ra);
        },
        .index_access => |ia| {
            return genIndexAccess(self, ia);
        },
        .slice_expr => |se| {
            return genSliceExpr(self, se);
        },
        .tuple_literal => |tl| {
            return genTupleLiteral(self, tl);
        },
        .sizeof_expr => |type_expr| {
            return genSizeOf(self, type_expr);
        },
        .await_expr => |inner_idx| {
            // Synchronous await: just load the handle variable (it already has the value)
            return genExpr(self, inner_idx);
        },
        .address_of => |operand_idx| {
            return genAddressOf(self, operand_idx);
        },
        .deref => |operand_idx| {
            return genDeref(self, operand_idx);
        },
        .closure => |cl| {
            return codegen_closures.genClosureExpr(self, cl);
        },
        .try_expr => |inner_idx| {
            return codegen_hof.genTryExpr(self, inner_idx);
        },
    }
}

pub fn genAddressOf(self: *CodeGen, operand_idx: Ast.ExprIndex) CodeGen.GenError!CodeGen.ExprResult {
    // The operand should be an lvalue (a variable). Return its alloca pointer.
    const expr = self.ast.getExpr(operand_idx);
    switch (expr) {
        .identifier => |name| {
            const nv = self.named_values.get(name) orelse {
                self.diagnostics.report(.@"error", 0, "undefined variable '{s}'", .{name});
                return error.CodeGenError;
            };
            // Return the alloca as a pointer value
            const pointee_ptr = self.type_tag_arena.allocator().create(CodeGen.TypeTag) catch return error.CodeGenError;
            pointee_ptr.* = nv.type_tag;
            return .{
                .value = nv.alloca,
                .type_tag = .{ .ptr_type = .{ .pointee = pointee_ptr, .is_mutable = nv.is_mutable } },
            };
        },
        else => {
            self.diagnostics.report(.@"error", 0, "cannot take address of this expression", .{});
            return error.CodeGenError;
        },
    }
}

pub fn genDeref(self: *CodeGen, operand_idx: Ast.ExprIndex) CodeGen.GenError!CodeGen.ExprResult {
    // Evaluate the operand (should be a pointer)
    const ptr_val = try genExpr(self, operand_idx);
    switch (ptr_val.type_tag) {
        .ptr_type => |pt| {
            const pointee_llvm = self.typeTagToLLVM(pt.pointee.*);
            const loaded = c.LLVMBuildLoad2(self.builder, pointee_llvm, ptr_val.value, "deref");
            return .{ .value = loaded, .type_tag = pt.pointee.* };
        },
        else => {
            self.diagnostics.report(.@"error", 0, "cannot dereference non-pointer type", .{});
            return error.CodeGenError;
        },
    }
}

pub fn genArrayLiteral(self: *CodeGen, al: Ast.ArrayLiteralExpr) CodeGen.GenError!CodeGen.ExprResult {
    if (al.elements.items.len == 0) {
        self.diagnostics.report(.@"error", 0, "empty array literal", .{});
        return error.CodeGenError;
    }

    // Generate first element to determine type
    const first = try genExpr(self, al.elements.items[0]);
    const elem_type_tag = first.type_tag;
    const elem_llvm = self.typeTagToLLVM(elem_type_tag);
    const count: u32 = @intCast(al.elements.items.len);
    const array_llvm = c.LLVMArrayType(elem_llvm, count);

    const alloca = c.LLVMBuildAlloca(self.builder, array_llvm, "arr_lit");

    // Store first element
    var gep0 = [_]c.LLVMValueRef{
        c.LLVMConstInt(c.LLVMInt64TypeInContext(self.context), 0, 0),
        c.LLVMConstInt(c.LLVMInt64TypeInContext(self.context), 0, 0),
    };
    const ptr0 = c.LLVMBuildGEP2(self.builder, array_llvm, alloca, &gep0, 2, "elem_ptr");
    _ = c.LLVMBuildStore(self.builder, first.value, ptr0);

    // Store remaining elements
    for (al.elements.items[1..], 1..) |elem_idx, i| {
        const elem_val = try genExpr(self, elem_idx);
        var gep_idx = [_]c.LLVMValueRef{
            c.LLVMConstInt(c.LLVMInt64TypeInContext(self.context), 0, 0),
            c.LLVMConstInt(c.LLVMInt64TypeInContext(self.context), @intCast(i), 0),
        };
        const elem_ptr = c.LLVMBuildGEP2(self.builder, array_llvm, alloca, &gep_idx, 2, "elem_ptr");
        _ = c.LLVMBuildStore(self.builder, elem_val.value, elem_ptr);
    }

    // Load the full array value
    const loaded = c.LLVMBuildLoad2(self.builder, array_llvm, alloca, "arr_val");

    const elem_ptr = self.type_tag_arena.allocator().create(CodeGen.TypeTag) catch return error.CodeGenError;
    elem_ptr.* = elem_type_tag;
    return .{ .value = loaded, .type_tag = .{ .array_type = .{ .element_type = elem_ptr, .count = count } } };
}

pub fn genRepeatedArray(self: *CodeGen, ra: Ast.RepeatedArrayExpr) CodeGen.GenError!CodeGen.ExprResult {
    // Count must be a compile-time integer literal
    const count_expr = self.ast.getExpr(ra.count);
    const count: u32 = switch (count_expr) {
        .int_literal => |v| @intCast(v),
        else => {
            self.diagnostics.report(.@"error", 0, "repeated array count must be a compile-time integer literal", .{});
            return error.CodeGenError;
        },
    };

    // Generate the value expression
    const val = try genExpr(self, ra.value);
    const elem_type_tag = val.type_tag;
    const elem_llvm = self.typeTagToLLVM(elem_type_tag);
    const array_llvm = c.LLVMArrayType(elem_llvm, count);

    const alloca = c.LLVMBuildAlloca(self.builder, array_llvm, "rep_arr");

    // Store the value at each index
    for (0..count) |i| {
        var gep_idx = [_]c.LLVMValueRef{
            c.LLVMConstInt(c.LLVMInt64TypeInContext(self.context), 0, 0),
            c.LLVMConstInt(c.LLVMInt64TypeInContext(self.context), @intCast(i), 0),
        };
        const elem_ptr = c.LLVMBuildGEP2(self.builder, array_llvm, alloca, &gep_idx, 2, "elem_ptr");
        _ = c.LLVMBuildStore(self.builder, val.value, elem_ptr);
    }

    // Load the full array value
    const loaded = c.LLVMBuildLoad2(self.builder, array_llvm, alloca, "rep_arr_val");

    const elem_type_ptr = self.type_tag_arena.allocator().create(CodeGen.TypeTag) catch return error.CodeGenError;
    elem_type_ptr.* = elem_type_tag;
    return .{ .value = loaded, .type_tag = .{ .array_type = .{ .element_type = elem_type_ptr, .count = count } } };
}

pub fn genTupleLiteral(self: *CodeGen, tl: Ast.TupleLiteralExpr) CodeGen.GenError!CodeGen.ExprResult {
    if (tl.elements.items.len < 2) {
        self.diagnostics.report(.@"error", 0, "tuple requires at least 2 elements", .{});
        return error.CodeGenError;
    }

    const count = tl.elements.items.len;

    // Generate all element values and collect types
    var elem_vals = self.allocator.alloc(CodeGen.ExprResult, count) catch return error.CodeGenError;
    defer self.allocator.free(elem_vals);
    var elem_tags = self.type_tag_arena.allocator().alloc(CodeGen.TypeTag, count) catch return error.CodeGenError;
    var elem_llvm_types = self.allocator.alloc(c.LLVMTypeRef, count) catch return error.CodeGenError;
    defer self.allocator.free(elem_llvm_types);

    for (tl.elements.items, 0..) |elem_idx, i| {
        elem_vals[i] = try genExpr(self, elem_idx);
        elem_tags[i] = elem_vals[i].type_tag;
        elem_llvm_types[i] = self.typeTagToLLVM(elem_tags[i]);
    }

    // Create anonymous struct type for the tuple
    const tuple_llvm = c.LLVMStructTypeInContext(self.context, elem_llvm_types.ptr, @intCast(count), 0);
    const alloca = c.LLVMBuildAlloca(self.builder, tuple_llvm, "tuple_lit");

    // Store each element via GEP
    for (elem_vals, 0..) |ev, i| {
        const field_ptr = c.LLVMBuildStructGEP2(self.builder, tuple_llvm, alloca, @intCast(i), "tup_elem_ptr");
        _ = c.LLVMBuildStore(self.builder, ev.value, field_ptr);
    }

    const loaded = c.LLVMBuildLoad2(self.builder, tuple_llvm, alloca, "tuple_val");
    return .{ .value = loaded, .type_tag = .{ .tuple_type = .{ .element_types = elem_tags, .llvm_type = tuple_llvm } } };
}

pub fn genIndexAccess(self: *CodeGen, ia: Ast.IndexAccessExpr) CodeGen.GenError!CodeGen.ExprResult {
    // We need the alloca (pointer) of the array or slice, not a loaded value
    const obj_expr = self.ast.getExpr(ia.object);
    switch (obj_expr) {
        .identifier => |name| {
            if (self.named_values.get(name)) |nv| {
                switch (nv.type_tag) {
                    .array_type => |arr| {
                        const index_val = try genExpr(self, ia.index);
                        const array_llvm = self.typeTagToLLVM(nv.type_tag);
                        var gep_idx = [_]c.LLVMValueRef{
                            c.LLVMConstInt(c.LLVMInt64TypeInContext(self.context), 0, 0),
                            index_val.value,
                        };
                        const elem_ptr = c.LLVMBuildGEP2(self.builder, array_llvm, nv.alloca, &gep_idx, 2, "idx_ptr");
                        const elem_llvm = self.typeTagToLLVM(arr.element_type.*);
                        const loaded = c.LLVMBuildLoad2(self.builder, elem_llvm, elem_ptr, "idx_val");
                        return .{ .value = loaded, .type_tag = arr.element_type.* };
                    },
                    .slice_type => |sl| {
                        return genSliceIndexAccess(self, nv.alloca, sl, ia.index);
                    },
                    else => {},
                }
            }
        },
        else => {},
    }
    self.diagnostics.report(.@"error", 0, "index access requires an array or slice", .{});
    return error.CodeGenError;
}

pub fn genSliceIndexAccess(self: *CodeGen, slice_alloca: c.LLVMValueRef, sl: CodeGen.SliceTypeTag, index_expr_idx: Ast.ExprIndex) CodeGen.GenError!CodeGen.ExprResult {
    const slice_llvm = self.sliceLLVMType();
    // Extract pointer (field 0)
    const ptr_field = c.LLVMBuildStructGEP2(self.builder, slice_llvm, slice_alloca, 0, "slice_ptr_field");
    const data_ptr = c.LLVMBuildLoad2(self.builder, c.LLVMPointerType(c.LLVMInt8TypeInContext(self.context), 0), ptr_field, "slice_ptr");

    const index_val = try genExpr(self, index_expr_idx);
    const elem_llvm = self.typeTagToLLVM(sl.element_type.*);

    // GEP on the data pointer with element type
    var gep_idx = [_]c.LLVMValueRef{index_val.value};
    const elem_ptr = c.LLVMBuildGEP2(self.builder, elem_llvm, c.LLVMBuildBitCast(self.builder, data_ptr, c.LLVMPointerType(elem_llvm, 0), "typed_ptr"), &gep_idx, 1, "slice_elem_ptr");
    const loaded = c.LLVMBuildLoad2(self.builder, elem_llvm, elem_ptr, "slice_elem");
    return .{ .value = loaded, .type_tag = sl.element_type.* };
}

pub fn genSliceExpr(self: *CodeGen, se: Ast.SliceExprNode) CodeGen.GenError!CodeGen.ExprResult {
    const obj_expr = self.ast.getExpr(se.object);
    switch (obj_expr) {
        .identifier => |name| {
            if (self.named_values.get(name)) |nv| {
                switch (nv.type_tag) {
                    .array_type => |arr| {
                        return genSliceFromArray(self, nv.alloca, arr, se);
                    },
                    else => {},
                }
            }
        },
        else => {},
    }
    self.diagnostics.report(.@"error", 0, "slicing requires an array", .{});
    return error.CodeGenError;
}

pub fn genSliceFromArray(self: *CodeGen, array_alloca: c.LLVMValueRef, arr: CodeGen.ArrayTypeTag, se: Ast.SliceExprNode) CodeGen.GenError!CodeGen.ExprResult {
    const elem_llvm = self.typeTagToLLVM(arr.element_type.*);
    const array_llvm = self.typeTagToLLVM(.{ .array_type = arr });

    // Compute start index
    const start_val = if (se.start) |start_idx|
        (try genExpr(self, start_idx)).value
    else
        c.LLVMConstInt(c.LLVMInt32TypeInContext(self.context), 0, 0);

    // Compute end index
    const end_val = if (se.end) |end_idx|
        (try genExpr(self, end_idx)).value
    else
        c.LLVMConstInt(c.LLVMInt32TypeInContext(self.context), arr.count, 0);

    // GEP to get pointer to arr[start]
    var gep_idx = [_]c.LLVMValueRef{
        c.LLVMConstInt(c.LLVMInt64TypeInContext(self.context), 0, 0),
        start_val,
    };
    const elem_ptr = c.LLVMBuildGEP2(self.builder, array_llvm, array_alloca, &gep_idx, 2, "slice_start_ptr");

    // Cast to i8*
    const byte_ptr = c.LLVMBuildBitCast(self.builder, elem_ptr, c.LLVMPointerType(c.LLVMInt8TypeInContext(self.context), 0), "byte_ptr");

    // Compute length = end - start
    const length = c.LLVMBuildSub(self.builder, end_val, start_val, "slice_len");

    // Build the slice struct { i8*, i32 }
    const slice_llvm = self.sliceLLVMType();
    const alloca = c.LLVMBuildAlloca(self.builder, slice_llvm, "slice_lit");

    // Store pointer (field 0)
    const ptr_field = c.LLVMBuildStructGEP2(self.builder, slice_llvm, alloca, 0, "slice_ptr_store");
    _ = c.LLVMBuildStore(self.builder, byte_ptr, ptr_field);

    // Store length (field 1)
    const len_field = c.LLVMBuildStructGEP2(self.builder, slice_llvm, alloca, 1, "slice_len_store");
    _ = c.LLVMBuildStore(self.builder, length, len_field);

    const loaded = c.LLVMBuildLoad2(self.builder, slice_llvm, alloca, "slice_val");

    // Create slice type tag with element type
    const elem_ptr_tag = self.type_tag_arena.allocator().create(CodeGen.TypeTag) catch return error.CodeGenError;
    elem_ptr_tag.* = arr.element_type.*;
    _ = elem_llvm;

    return .{ .value = loaded, .type_tag = .{ .slice_type = .{ .element_type = elem_ptr_tag } } };
}

pub fn genFieldAccess(self: *CodeGen, fa: Ast.FieldAccessExpr) CodeGen.GenError!CodeGen.ExprResult {
    // Check if this is an enum constructor: EnumName.VariantName
    const obj_expr = self.ast.getExpr(fa.object);
    switch (obj_expr) {
        .identifier => |name| {
            // Check stdlib constants: math.PI, math.E
            if (std.mem.eql(u8, name, "math")) {
                return codegen_stdlib.genMathConstant(self, fa.field);
            }
            if (self.enum_types.contains(name) or self.generic_enum_decls.contains(name)) {
                // This is an enum constructor without args: EnumName.VariantName
                return self.genEnumConstructor(name, fa.field, &.{});
            }
            if (self.named_values.get(name)) |nv| {
                switch (nv.type_tag) {
                    .struct_type => |struct_name| {
                        if (self.struct_types.get(struct_name)) |info| {
                            if (self.findFieldIndex(info, fa.field)) |idx| {
                                const field_ptr = c.LLVMBuildStructGEP2(self.builder, info.llvm_type, nv.alloca, @intCast(idx), "field_ptr");
                                const field_type = info.field_types[idx];
                                const loaded = c.LLVMBuildLoad2(self.builder, self.typeTagToLLVM(field_type), field_ptr, "field");
                                return .{ .value = loaded, .type_tag = field_type };
                            }
                            self.diagnostics.report(.@"error", 0, "no field '{s}' on struct '{s}'", .{ fa.field, struct_name });
                            return error.CodeGenError;
                        }
                    },
                    .tuple_type => |tt| {
                        // Tuple element access: tuple.0, tuple.1, etc.
                        const idx = std.fmt.parseInt(u32, fa.field, 10) catch {
                            self.diagnostics.report(.@"error", 0, "invalid tuple index '{s}'", .{fa.field});
                            return error.CodeGenError;
                        };
                        if (idx >= tt.element_types.len) {
                            self.diagnostics.report(.@"error", 0, "tuple index {d} out of bounds (tuple has {d} elements)", .{ idx, tt.element_types.len });
                            return error.CodeGenError;
                        }
                        const field_ptr = c.LLVMBuildStructGEP2(self.builder, tt.llvm_type, nv.alloca, idx, "tup_idx_ptr");
                        const elem_type = tt.element_types[idx];
                        const loaded = c.LLVMBuildLoad2(self.builder, self.typeTagToLLVM(elem_type), field_ptr, "tup_elem");
                        return .{ .value = loaded, .type_tag = elem_type };
                    },
                    else => {},
                }
            }
        },
        .field_access => {
            // Nested field access: a.b.c — we need to chain GEPs
            const parent = try genFieldAccessPtr(self, fa.object);
            switch (parent.type_tag) {
                .struct_type => |struct_name| {
                    if (self.struct_types.get(struct_name)) |info| {
                        if (self.findFieldIndex(info, fa.field)) |idx| {
                            const field_ptr = c.LLVMBuildStructGEP2(self.builder, info.llvm_type, parent.value, @intCast(idx), "field_ptr");
                            const field_type = info.field_types[idx];
                            const loaded = c.LLVMBuildLoad2(self.builder, self.typeTagToLLVM(field_type), field_ptr, "field");
                            return .{ .value = loaded, .type_tag = field_type };
                        }
                    }
                },
                else => {},
            }
        },
        else => {},
    }
    self.diagnostics.report(.@"error", 0, "cannot access field on this expression", .{});
    return error.CodeGenError;
}

pub fn genFieldAccessPtr(self: *CodeGen, expr_idx: Ast.ExprIndex) CodeGen.GenError!CodeGen.ExprResult {
    const expr = self.ast.getExpr(expr_idx);
    switch (expr) {
        .identifier => |name| {
            if (self.named_values.get(name)) |nv| {
                return .{ .value = nv.alloca, .type_tag = nv.type_tag };
            }
        },
        .field_access => |fa| {
            const parent = try genFieldAccessPtr(self, fa.object);
            switch (parent.type_tag) {
                .struct_type => |struct_name| {
                    if (self.struct_types.get(struct_name)) |info| {
                        if (self.findFieldIndex(info, fa.field)) |idx| {
                            const field_ptr = c.LLVMBuildStructGEP2(self.builder, info.llvm_type, parent.value, @intCast(idx), "field_ptr");
                            return .{ .value = field_ptr, .type_tag = info.field_types[idx] };
                        }
                    }
                },
                else => {},
            }
        },
        else => {},
    }
    self.diagnostics.report(.@"error", 0, "cannot get pointer to field", .{});
    return error.CodeGenError;
}

pub fn genStructLiteral(self: *CodeGen, sl: Ast.StructLiteralExpr) CodeGen.GenError!CodeGen.ExprResult {
    // Determine the concrete struct name (handle generic type args via monomorphization)
    const struct_name = if (sl.type_args.items.len > 0)
        try self.monomorphizeStruct(sl.name, sl.type_args.items)
    else
        sl.name;

    const info = self.struct_types.get(struct_name) orelse {
        self.diagnostics.report(.@"error", 0, "undefined struct '{s}'", .{sl.name});
        return error.CodeGenError;
    };

    // Allocate the struct on the stack
    const alloca = c.LLVMBuildAlloca(self.builder, info.llvm_type, "struct_lit");

    // Initialize each field
    for (sl.field_inits.items) |fi| {
        if (self.findFieldIndex(info, fi.name)) |idx| {
            const val = try genExpr(self, fi.value);
            const field_ptr = c.LLVMBuildStructGEP2(self.builder, info.llvm_type, alloca, @intCast(idx), "field_init");
            _ = c.LLVMBuildStore(self.builder, val.value, field_ptr);
        } else {
            self.diagnostics.report(.@"error", 0, "no field '{s}' on struct '{s}'", .{ fi.name, struct_name });
            return error.CodeGenError;
        }
    }

    // Load the full struct value
    const loaded = c.LLVMBuildLoad2(self.builder, info.llvm_type, alloca, "struct_val");
    return .{ .value = loaded, .type_tag = .{ .struct_type = struct_name } };
}

pub fn genMethodCall(self: *CodeGen, mc: Ast.MethodCallExpr) CodeGen.GenError!CodeGen.ExprResult {
    // Check if this is an enum constructor: EnumName.VariantName(args)
    const mc_obj_expr = self.ast.getExpr(mc.object);
    switch (mc_obj_expr) {
        .identifier => |name| {
            if (self.enum_types.contains(name) or self.generic_enum_decls.contains(name)) {
                return self.genEnumConstructor(name, mc.method, &mc.args);
            }
            // Check if this is an import alias call: alias.function(args)
            if (self.getImportPackageName(name)) |pkg_name| {
                const mangled_slice = std.fmt.allocPrint(self.allocator, "__pkg_{s}_{s}", .{ pkg_name, mc.method }) catch return error.CodeGenError;
                defer self.allocator.free(mangled_slice);
                return genImportCall(self, mangled_slice, &mc.args);
            }
            // Check stdlib namespace calls: math.sqrt(), str.upper(), etc.
            if (CodeGen.isStdlibNamespace(name)) {
                return codegen_stdlib.genStdlibCall(self, name, mc.method, &mc.args);
            }
        },
        else => {},
    }

    // Get the object value
    const obj = try genExpr(self, mc.object);

    // Handle trait object method calls (dynamic dispatch)
    switch (obj.type_tag) {
        .trait_type => |trait_name| {
            return codegen_hof.genTraitMethodCall(self, obj, trait_name, mc.method, &mc.args);
        },
        else => {},
    }

    // Determine the struct type
    const struct_name = switch (obj.type_tag) {
        .struct_type => |name| name,
        else => {
            self.diagnostics.report(.@"error", 0, "method call on non-struct type", .{});
            return error.CodeGenError;
        },
    };

    // Look up the mangled function name
    const mangled_slice = std.fmt.allocPrint(self.allocator, "{s}_{s}", .{ struct_name, mc.method }) catch return error.CodeGenError;
    defer self.allocator.free(mangled_slice);
    const mangled = self.allocator.dupeZ(u8, mangled_slice) catch return error.CodeGenError;
    defer self.allocator.free(mangled);

    const function = c.LLVMGetNamedFunction(self.module, mangled.ptr);
    if (function == null) {
        self.diagnostics.report(.@"error", 0, "undefined method '{s}' on '{s}'", .{ mc.method, struct_name });
        return error.CodeGenError;
    }

    // Build args: self (the object) + method args
    var args = std.ArrayList(c.LLVMValueRef){};
    defer args.deinit(self.allocator);
    try args.append(self.allocator, obj.value); // self parameter

    for (mc.args.items) |arg_idx| {
        const arg_val = try genExpr(self, arg_idx);
        try args.append(self.allocator, arg_val.value);
    }

    const fn_type = c.LLVMGlobalGetValueType(function);
    const ret_type = c.LLVMGetReturnType(fn_type);
    const is_void = c.LLVMGetTypeKind(ret_type) == c.LLVMVoidTypeKind;

    if (is_void) {
        _ = c.LLVMBuildCall2(self.builder, fn_type, function, args.items.ptr, @intCast(args.items.len), "");
        return .{ .value = c.LLVMGetUndef(c.LLVMVoidTypeInContext(self.context)), .type_tag = .void_type };
    } else {
        const result = c.LLVMBuildCall2(self.builder, fn_type, function, args.items.ptr, @intCast(args.items.len), "mcall");
        // Look up method return type from AST
        const ret_tag = lookupMethodReturnType(self, struct_name, mc.method);
        return .{ .value = result, .type_tag = ret_tag };
    }
}

pub fn genImportCall(self: *CodeGen, func_name: []const u8, call_args: *const std.ArrayList(Ast.ExprIndex)) CodeGen.GenError!CodeGen.ExprResult {
    const name_z = self.allocator.dupeZ(u8, func_name) catch return error.CodeGenError;
    defer self.allocator.free(name_z);

    const function = c.LLVMGetNamedFunction(self.module, name_z.ptr);
    if (function == null) {
        self.diagnostics.report(.@"error", 0, "undefined imported function '{s}'", .{func_name});
        return error.CodeGenError;
    }

    var args = std.ArrayList(c.LLVMValueRef){};
    defer args.deinit(self.allocator);
    for (call_args.items) |arg_idx| {
        const arg_val = try genExpr(self, arg_idx);
        try args.append(self.allocator, arg_val.value);
    }

    const fn_type = c.LLVMGlobalGetValueType(function);
    const ret_type = c.LLVMGetReturnType(fn_type);
    const is_void = c.LLVMGetTypeKind(ret_type) == c.LLVMVoidTypeKind;

    if (is_void) {
        _ = c.LLVMBuildCall2(self.builder, fn_type, function, args.items.ptr, @intCast(args.items.len), "");
        return .{ .value = c.LLVMGetUndef(c.LLVMVoidTypeInContext(self.context)), .type_tag = .void_type };
    } else {
        const result = c.LLVMBuildCall2(self.builder, fn_type, function, args.items.ptr, @intCast(args.items.len), "icall");
        // Look up return type from imported function map, fallback to i32
        const ret_tag = self.imported_fn_return_types.get(func_name) orelse lookupFunctionReturnType(self, func_name);
        return .{ .value = result, .type_tag = ret_tag };
    }
}

pub fn genAssociatedCall(self: *CodeGen, ac: Ast.AssociatedCallExpr) CodeGen.GenError!CodeGen.ExprResult {
    // Look up the mangled function name: TypeName_funcName
    const mangled_slice = std.fmt.allocPrint(self.allocator, "{s}_{s}", .{ ac.type_name, ac.func_name }) catch return error.CodeGenError;
    defer self.allocator.free(mangled_slice);
    const mangled = self.allocator.dupeZ(u8, mangled_slice) catch return error.CodeGenError;
    defer self.allocator.free(mangled);

    const function = c.LLVMGetNamedFunction(self.module, mangled.ptr);
    if (function == null) {
        self.diagnostics.report(.@"error", 0, "undefined associated function '{s}::{s}'", .{ ac.type_name, ac.func_name });
        return error.CodeGenError;
    }

    // Build args (no self parameter for associated functions)
    var args = std.ArrayList(c.LLVMValueRef){};
    defer args.deinit(self.allocator);

    for (ac.args.items) |arg_idx| {
        const arg_val = try genExpr(self, arg_idx);
        try args.append(self.allocator, arg_val.value);
    }

    const fn_type = c.LLVMGlobalGetValueType(function);
    const ret_type = c.LLVMGetReturnType(fn_type);
    const is_void = c.LLVMGetTypeKind(ret_type) == c.LLVMVoidTypeKind;

    if (is_void) {
        _ = c.LLVMBuildCall2(self.builder, fn_type, function, args.items.ptr, @intCast(args.items.len), "");
        return .{ .value = c.LLVMGetUndef(c.LLVMVoidTypeInContext(self.context)), .type_tag = .void_type };
    } else {
        const result = c.LLVMBuildCall2(self.builder, fn_type, function, args.items.ptr, @intCast(args.items.len), "acall");
        const ret_tag = lookupMethodReturnType(self, ac.type_name, ac.func_name);
        return .{ .value = result, .type_tag = ret_tag };
    }
}

pub fn lookupMethodReturnType(self: *CodeGen, type_name: []const u8, method_name: []const u8) CodeGen.TypeTag {
    for (self.ast.program.decls.items) |decl| {
        switch (decl.decl) {
            .impl_decl => |impl_decl| {
                if (std.mem.eql(u8, impl_decl.type_name, type_name)) {
                    for (impl_decl.methods.items) |method| {
                        if (std.mem.eql(u8, method.name, method_name)) {
                            return self.resolveTypeExpr(method.return_type);
                        }
                    }
                }
            },
            else => {},
        }
    }
    // Check if this is a monomorphized generic struct
    if (self.monomorphized_origins.get(type_name)) |origin| {
        // Find the generic impl and resolve return type with substitution
        if (self.generic_impl_decls.get(origin.generic_name)) |impl_decl| {
            if (self.generic_struct_decls.get(origin.generic_name)) |sd| {
                var type_map = std.StringHashMap(CodeGen.TypeTag).init(self.allocator);
                defer type_map.deinit();
                for (sd.type_params.items, 0..) |tp, i| {
                    if (i < origin.type_args.len) {
                        const resolved = self.resolveTypeExpr(origin.type_args[i]);
                        type_map.put(tp.name, resolved) catch {};
                    }
                }
                for (impl_decl.methods.items) |method| {
                    if (std.mem.eql(u8, method.name, method_name)) {
                        return self.resolveTypeExprWithSubst(method.return_type, &type_map);
                    }
                }
            }
        }
    }
    // Fallback: check imported method return types
    const mangled = std.fmt.allocPrint(self.allocator, "{s}_{s}", .{ type_name, method_name }) catch return .i32_type;
    defer self.allocator.free(mangled);
    if (self.imported_fn_return_types.get(mangled)) |ret_tag| return ret_tag;
    return .i32_type;
}

pub fn genBinaryOp(self: *CodeGen, op: Ast.BinaryOp, lhs: CodeGen.ExprResult, rhs: CodeGen.ExprResult) CodeGen.GenError!CodeGen.ExprResult {
    // String concatenation: string + string
    if (op == .add and CodeGen.isTypeTag(lhs.type_tag, .string_type) and CodeGen.isTypeTag(rhs.type_tag, .string_type)) {
        return genStringConcat(self, lhs.value, rhs.value);
    }

    const is_float = CodeGen.isTypeTag(lhs.type_tag, .f64_type);
    const is_unsigned = CodeGen.isUnsignedInt(lhs.type_tag);
    const value = switch (op) {
        .add => if (is_float) c.LLVMBuildFAdd(self.builder, lhs.value, rhs.value, "fadd") else c.LLVMBuildAdd(self.builder, lhs.value, rhs.value, "add"),
        .sub => if (is_float) c.LLVMBuildFSub(self.builder, lhs.value, rhs.value, "fsub") else c.LLVMBuildSub(self.builder, lhs.value, rhs.value, "sub"),
        .mul => if (is_float) c.LLVMBuildFMul(self.builder, lhs.value, rhs.value, "fmul") else c.LLVMBuildMul(self.builder, lhs.value, rhs.value, "mul"),
        .div => if (is_float) c.LLVMBuildFDiv(self.builder, lhs.value, rhs.value, "fdiv") else if (is_unsigned) c.LLVMBuildUDiv(self.builder, lhs.value, rhs.value, "udiv") else c.LLVMBuildSDiv(self.builder, lhs.value, rhs.value, "sdiv"),
        .mod => if (is_unsigned) c.LLVMBuildURem(self.builder, lhs.value, rhs.value, "umod") else c.LLVMBuildSRem(self.builder, lhs.value, rhs.value, "mod"),
        .eq => if (is_float) c.LLVMBuildFCmp(self.builder, c.LLVMRealOEQ, lhs.value, rhs.value, "feq") else c.LLVMBuildICmp(self.builder, c.LLVMIntEQ, lhs.value, rhs.value, "eq"),
        .neq => if (is_float) c.LLVMBuildFCmp(self.builder, c.LLVMRealONE, lhs.value, rhs.value, "fne") else c.LLVMBuildICmp(self.builder, c.LLVMIntNE, lhs.value, rhs.value, "ne"),
        .lt => if (is_float) c.LLVMBuildFCmp(self.builder, c.LLVMRealOLT, lhs.value, rhs.value, "flt") else c.LLVMBuildICmp(self.builder, if (is_unsigned) c.LLVMIntULT else c.LLVMIntSLT, lhs.value, rhs.value, "lt"),
        .le => if (is_float) c.LLVMBuildFCmp(self.builder, c.LLVMRealOLE, lhs.value, rhs.value, "fle") else c.LLVMBuildICmp(self.builder, if (is_unsigned) c.LLVMIntULE else c.LLVMIntSLE, lhs.value, rhs.value, "le"),
        .gt => if (is_float) c.LLVMBuildFCmp(self.builder, c.LLVMRealOGT, lhs.value, rhs.value, "fgt") else c.LLVMBuildICmp(self.builder, if (is_unsigned) c.LLVMIntUGT else c.LLVMIntSGT, lhs.value, rhs.value, "gt"),
        .ge => if (is_float) c.LLVMBuildFCmp(self.builder, c.LLVMRealOGE, lhs.value, rhs.value, "fge") else c.LLVMBuildICmp(self.builder, if (is_unsigned) c.LLVMIntUGE else c.LLVMIntSGE, lhs.value, rhs.value, "ge"),
        .logic_and => c.LLVMBuildAnd(self.builder, lhs.value, rhs.value, "and"),
        .logic_or => c.LLVMBuildOr(self.builder, lhs.value, rhs.value, "or"),
        .bit_and => c.LLVMBuildAnd(self.builder, lhs.value, rhs.value, "bitand"),
        .bit_or => c.LLVMBuildOr(self.builder, lhs.value, rhs.value, "bitor"),
        .bit_xor => c.LLVMBuildXor(self.builder, lhs.value, rhs.value, "xor"),
        .shift_left => c.LLVMBuildShl(self.builder, lhs.value, rhs.value, "shl"),
        .shift_right => if (is_unsigned) c.LLVMBuildLShr(self.builder, lhs.value, rhs.value, "lshr") else c.LLVMBuildAShr(self.builder, lhs.value, rhs.value, "shr"),
    };

    const result_type: CodeGen.TypeTag = switch (op) {
        .eq, .neq, .lt, .le, .gt, .ge => .bool_type,
        else => lhs.type_tag,
    };

    return .{ .value = value, .type_tag = result_type };
}

pub fn genUnaryOp(self: *CodeGen, op: Ast.UnaryOp, operand: CodeGen.ExprResult) CodeGen.GenError!CodeGen.ExprResult {
    const value = switch (op) {
        .negate => if (CodeGen.isTypeTag(operand.type_tag, .f64_type))
            c.LLVMBuildFNeg(self.builder, operand.value, "fneg")
        else
            c.LLVMBuildNeg(self.builder, operand.value, "neg"),
        .logic_not => c.LLVMBuildNot(self.builder, operand.value, "not"),
        .bit_not => c.LLVMBuildNot(self.builder, operand.value, "bitnot"),
    };
    return .{ .value = value, .type_tag = operand.type_tag };
}

pub fn genCallExpr(self: *CodeGen, call_expr: Ast.CallExpr) CodeGen.GenError!CodeGen.ExprResult {
    const callee_expr = self.ast.getExpr(call_expr.callee);
    const name = switch (callee_expr) {
        .identifier => |n| n,
        else => {
            self.diagnostics.report(.@"error", 0, "callee must be an identifier", .{});
            return error.CodeGenError;
        },
    };

    // Check if callee is a closure/fn_type variable
    if (self.named_values.get(name)) |nv| {
        switch (nv.type_tag) {
            .closure_type => |ct| return codegen_closures.genClosureCall(self, ct, &call_expr.args),
            .fn_type => |ft| return codegen_closures.genFnTypeCall(self, nv, ft, &call_expr.args),
            else => {},
        }
    }

    if (std.mem.eql(u8, name, "log")) {
        return genLogCall(self, call_expr);
    }

    if (std.mem.eql(u8, name, "len")) {
        return genLenCall(self, call_expr);
    }

    if (std.mem.eql(u8, name, "panic")) {
        return genPanicCall(self, call_expr);
    }

    if (std.mem.eql(u8, name, "exit")) {
        return genExitCall(self, call_expr);
    }

    if (std.mem.eql(u8, name, "range")) {
        self.diagnostics.report(.@"error", 0, "range() can only be used in for loops", .{});
        return error.CodeGenError;
    }

    // Higher-order array functions
    if (std.mem.eql(u8, name, "map")) return codegen_hof.genMapCall(self, call_expr);
    if (std.mem.eql(u8, name, "filter")) return codegen_hof.genFilterCall(self, call_expr);
    if (std.mem.eql(u8, name, "reduce")) return codegen_hof.genReduceCall(self, call_expr);
    if (std.mem.eql(u8, name, "forEach")) return codegen_hof.genForEachCall(self, call_expr);

    // Type conversion calls: i32(expr), f64(expr), etc.
    if (self.getTypeConversion(name)) |target_type| {
        if (call_expr.args.items.len != 1) {
            self.diagnostics.report(.@"error", 0, "type conversion takes exactly 1 argument", .{});
            return error.CodeGenError;
        }
        const arg = try genExpr(self, call_expr.args.items[0]);
        return self.genTypeConversion(arg, target_type);
    }

    const name_z = self.allocator.dupeZ(u8, name) catch return error.CodeGenError;
    defer self.allocator.free(name_z);

    var function = c.LLVMGetNamedFunction(self.module, name_z.ptr);
    if (function == null) {
        // Check for generic function — monomorphize on demand
        if (self.generic_fn_decls.get(name)) |gen_fn| {
            function = try codegen_hof.monomorphizeGenericFn(self, gen_fn, &call_expr.args);
        }
        if (function == null) {
            self.diagnostics.report(.@"error", 0, "undefined function '{s}'", .{name});
            return error.CodeGenError;
        }
    }

    // Look up function parameter types from AST for auto-upcast
    var param_type_tags = std.ArrayList(CodeGen.TypeTag){};
    defer param_type_tags.deinit(self.allocator);
    for (self.ast.program.decls.items) |decl| {
        switch (decl.decl) {
            .function => |fn_decl| {
                if (std.mem.eql(u8, fn_decl.name, name)) {
                    for (fn_decl.params.items) |param| {
                        param_type_tags.append(self.allocator, self.resolveTypeExpr(param.type_expr)) catch {};
                    }
                    break;
                }
            },
            else => {},
        }
    }

    var args = std.ArrayList(c.LLVMValueRef){};
    defer args.deinit(self.allocator);
    for (call_expr.args.items, 0..) |arg_idx, arg_i| {
        const arg_val = try genExpr(self, arg_idx);
        // Auto-upcast: if param expects trait_type and arg is struct_type, build fat pointer
        if (arg_i < param_type_tags.items.len) {
            switch (param_type_tags.items[arg_i]) {
                .trait_type => |trait_name| {
                    const upcast_val = try codegen_hof.buildTraitUpcast(self, arg_val, arg_idx, trait_name);
                    try args.append(self.allocator, upcast_val);
                    continue;
                },
                else => {},
            }
        }
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
        // Look up return type: check registered return types first, then AST
        const fn_llvm_name = std.mem.span(c.LLVMGetValueName(function));
        const ret_tag = self.imported_fn_return_types.get(fn_llvm_name) orelse lookupFunctionReturnType(self, name);
        return .{ .value = result, .type_tag = ret_tag };
    }
}

pub fn genLogCall(self: *CodeGen, call_expr: Ast.CallExpr) CodeGen.GenError!CodeGen.ExprResult {
    if (call_expr.args.items.len != 1) {
        self.diagnostics.report(.@"error", 0, "log() takes exactly 1 argument", .{});
        return error.CodeGenError;
    }

    const arg = try genExpr(self, call_expr.args.items[0]);
    const printf_type = c.LLVMGlobalGetValueType(self.printf_fn);

    if (CodeGen.isIntegerType(arg.type_tag)) {
        // Pick the right format string and value based on type
        const fmt_and_val = switch (arg.type_tag) {
            .i8_type, .i16_type => .{
                self.fmt_int,
                c.LLVMBuildSExt(self.builder, arg.value, c.LLVMInt32TypeInContext(self.context), "sext_log"),
            },
            .u8_type, .u16_type => .{
                self.fmt_uint,
                c.LLVMBuildZExt(self.builder, arg.value, c.LLVMInt32TypeInContext(self.context), "zext_log"),
            },
            .i32_type => .{ self.fmt_int, arg.value },
            .u32_type => .{ self.fmt_uint, arg.value },
            .i64_type => .{ self.fmt_long, arg.value },
            .u64_type => .{ self.fmt_ulong, arg.value },
            .i128_type => .{
                self.fmt_long,
                c.LLVMBuildTrunc(self.builder, arg.value, c.LLVMInt64TypeInContext(self.context), "trunc_log"),
            },
            .u128_type => .{
                self.fmt_ulong,
                c.LLVMBuildTrunc(self.builder, arg.value, c.LLVMInt64TypeInContext(self.context), "trunc_log"),
            },
            else => unreachable,
        };
        const args_arr = [_]c.LLVMValueRef{ fmt_and_val[0], fmt_and_val[1] };
        _ = c.LLVMBuildCall2(self.builder, printf_type, self.printf_fn, @constCast(&args_arr), 2, "");
    } else if (CodeGen.isTypeTag(arg.type_tag, .char_type)) {
        // Print char using %c format - need to extend i8 to i32 for printf
        const ext = c.LLVMBuildZExt(self.builder, arg.value, c.LLVMInt32TypeInContext(self.context), "char_ext");
        const args_arr = [_]c.LLVMValueRef{ self.fmt_char, ext };
        _ = c.LLVMBuildCall2(self.builder, printf_type, self.printf_fn, @constCast(&args_arr), 2, "");
    } else if (CodeGen.isTypeTag(arg.type_tag, .f64_type)) {
        const args_arr = [_]c.LLVMValueRef{ self.fmt_float, arg.value };
        _ = c.LLVMBuildCall2(self.builder, printf_type, self.printf_fn, @constCast(&args_arr), 2, "");
    } else if (CodeGen.isTypeTag(arg.type_tag, .string_type)) {
        const args_arr = [_]c.LLVMValueRef{ self.fmt_str, arg.value };
        _ = c.LLVMBuildCall2(self.builder, printf_type, self.printf_fn, @constCast(&args_arr), 2, "");
    } else if (CodeGen.isTypeTag(arg.type_tag, .bool_type)) {
        const fmt = c.LLVMBuildSelect(self.builder, arg.value, self.fmt_bool_true, self.fmt_bool_false, "bool_fmt");
        const args_arr = [_]c.LLVMValueRef{fmt};
        _ = c.LLVMBuildCall2(self.builder, printf_type, self.printf_fn, @constCast(&args_arr), 1, "");
    } else if (CodeGen.isTypeTag(arg.type_tag, .void_type)) {
        self.diagnostics.report(.@"error", 0, "cannot log void value", .{});
        return error.CodeGenError;
    } else {
        self.diagnostics.report(.@"error", 0, "cannot log this type", .{});
        return error.CodeGenError;
    }

    return .{ .value = c.LLVMGetUndef(c.LLVMVoidTypeInContext(self.context)), .type_tag = .void_type };
}

pub fn genPanicCall(self: *CodeGen, call_expr: Ast.CallExpr) CodeGen.GenError!CodeGen.ExprResult {
    if (call_expr.args.items.len != 1) {
        self.diagnostics.report(.@"error", 0, "panic() takes exactly 1 argument", .{});
        return error.CodeGenError;
    }

    const arg = try genExpr(self, call_expr.args.items[0]);
    if (!CodeGen.isTypeTag(arg.type_tag, .string_type)) {
        self.diagnostics.report(.@"error", 0, "panic() requires a string argument", .{});
        return error.CodeGenError;
    }

    // Print "panic: <message>\n" via printf
    const printf_type = c.LLVMGlobalGetValueType(self.printf_fn);
    const args_arr = [_]c.LLVMValueRef{ self.fmt_panic, arg.value };
    _ = c.LLVMBuildCall2(self.builder, printf_type, self.printf_fn, @constCast(&args_arr), 2, "");

    // Call exit(1)
    const exit_type = c.LLVMGlobalGetValueType(self.exit_fn);
    const exit_args = [_]c.LLVMValueRef{c.LLVMConstInt(c.LLVMInt32TypeInContext(self.context), 1, 0)};
    _ = c.LLVMBuildCall2(self.builder, exit_type, self.exit_fn, @constCast(&exit_args), 1, "");
    _ = c.LLVMBuildUnreachable(self.builder);

    return .{ .value = c.LLVMGetUndef(c.LLVMVoidTypeInContext(self.context)), .type_tag = .void_type };
}

pub fn genExitCall(self: *CodeGen, call_expr: Ast.CallExpr) CodeGen.GenError!CodeGen.ExprResult {
    if (call_expr.args.items.len != 1) {
        self.diagnostics.report(.@"error", 0, "exit() takes exactly 1 argument", .{});
        return error.CodeGenError;
    }

    const arg = try genExpr(self, call_expr.args.items[0]);
    if (!CodeGen.isTypeTag(arg.type_tag, .i32_type)) {
        self.diagnostics.report(.@"error", 0, "exit() requires an i32 argument", .{});
        return error.CodeGenError;
    }

    const exit_type = c.LLVMGlobalGetValueType(self.exit_fn);
    const exit_args = [_]c.LLVMValueRef{arg.value};
    _ = c.LLVMBuildCall2(self.builder, exit_type, self.exit_fn, @constCast(&exit_args), 1, "");
    _ = c.LLVMBuildUnreachable(self.builder);

    return .{ .value = c.LLVMGetUndef(c.LLVMVoidTypeInContext(self.context)), .type_tag = .void_type };
}

pub fn genLenCall(self: *CodeGen, call_expr: Ast.CallExpr) CodeGen.GenError!CodeGen.ExprResult {
    if (call_expr.args.items.len != 1) {
        self.diagnostics.report(.@"error", 0, "len() takes exactly 1 argument", .{});
        return error.CodeGenError;
    }

    // We need to determine the type of the argument without generating a load
    const arg_expr = self.ast.getExpr(call_expr.args.items[0]);
    switch (arg_expr) {
        .identifier => |arg_name| {
            if (self.named_values.get(arg_name)) |nv| {
                switch (nv.type_tag) {
                    .array_type => |arr| {
                        return .{
                            .value = c.LLVMConstInt(c.LLVMInt32TypeInContext(self.context), arr.count, 0),
                            .type_tag = .i32_type,
                        };
                    },
                    .slice_type => {
                        // Load length field (index 1) from the slice struct
                        const slice_llvm = self.sliceLLVMType();
                        const len_ptr = c.LLVMBuildStructGEP2(self.builder, slice_llvm, nv.alloca, 1, "slice_len_ptr");
                        const len_val = c.LLVMBuildLoad2(self.builder, c.LLVMInt32TypeInContext(self.context), len_ptr, "slice_len");
                        return .{
                            .value = len_val,
                            .type_tag = .i32_type,
                        };
                    },
                    .string_type => {
                        // Load the string pointer, then call strlen
                        const str_val = c.LLVMBuildLoad2(self.builder, self.typeTagToLLVM(.string_type), nv.alloca, "str_val");
                        return genStrlen(self, str_val);
                    },
                    else => {},
                }
            }
        },
        else => {
            // For non-identifier expressions (e.g. function calls), generate the expression
            const arg = try genExpr(self, call_expr.args.items[0]);
            if (CodeGen.isTypeTag(arg.type_tag, .string_type)) {
                return genStrlen(self, arg.value);
            }
        },
    }

    self.diagnostics.report(.@"error", 0, "len() requires an array, slice, or string argument", .{});
    return error.CodeGenError;
}

pub fn genStrlen(self: *CodeGen, str_val: c.LLVMValueRef) CodeGen.GenError!CodeGen.ExprResult {
    const strlen_type = c.LLVMGlobalGetValueType(self.strlen_fn);
    const args_arr = [_]c.LLVMValueRef{str_val};
    const len_i64 = c.LLVMBuildCall2(self.builder, strlen_type, self.strlen_fn, @constCast(&args_arr), 1, "strlen");
    // Truncate i64 to i32 (strlen returns size_t which is i64)
    const len_i32 = c.LLVMBuildTrunc(self.builder, len_i64, c.LLVMInt32TypeInContext(self.context), "len_i32");
    return .{ .value = len_i32, .type_tag = .i32_type };
}

pub fn genStringConcat(self: *CodeGen, lhs_val: c.LLVMValueRef, rhs_val: c.LLVMValueRef) CodeGen.GenError!CodeGen.ExprResult {
    const strlen_type = c.LLVMGlobalGetValueType(self.strlen_fn);
    const malloc_type = c.LLVMGlobalGetValueType(self.malloc_fn);
    const sprintf_type = c.LLVMGlobalGetValueType(self.sprintf_fn);

    // Get lengths of both strings
    const lhs_args = [_]c.LLVMValueRef{lhs_val};
    const lhs_len = c.LLVMBuildCall2(self.builder, strlen_type, self.strlen_fn, @constCast(&lhs_args), 1, "lhs_len");
    const rhs_args = [_]c.LLVMValueRef{rhs_val};
    const rhs_len = c.LLVMBuildCall2(self.builder, strlen_type, self.strlen_fn, @constCast(&rhs_args), 1, "rhs_len");

    // total_len = lhs_len + rhs_len + 1
    const sum_len = c.LLVMBuildAdd(self.builder, lhs_len, rhs_len, "sum_len");
    const one = c.LLVMConstInt(c.LLVMInt64TypeInContext(self.context), 1, 0);
    const total_len = c.LLVMBuildAdd(self.builder, sum_len, one, "total_len");

    // Allocate buffer
    const malloc_args = [_]c.LLVMValueRef{total_len};
    const buf = c.LLVMBuildCall2(self.builder, malloc_type, self.malloc_fn, @constCast(&malloc_args), 1, "concat_buf");

    // sprintf(buf, "%s%s", lhs, rhs)
    const fmt_str = c.LLVMBuildGlobalStringPtr(self.builder, "%s%s", "concat_fmt");
    const sprintf_args = [_]c.LLVMValueRef{ buf, fmt_str, lhs_val, rhs_val };
    _ = c.LLVMBuildCall2(self.builder, sprintf_type, self.sprintf_fn, @constCast(&sprintf_args), 4, "");

    return .{ .value = buf, .type_tag = .string_type };
}

pub fn genSizeOf(self: *CodeGen, type_expr: Ast.TypeExpr) CodeGen.GenError!CodeGen.ExprResult {
    const type_tag = self.resolveTypeExpr(type_expr);
    const llvm_type = self.typeTagToLLVM(type_tag);

    // Use LLVMSizeOf which returns a constant i64, then truncate to i32
    const size_val = c.LLVMSizeOf(llvm_type);
    const truncated = c.LLVMConstTrunc(size_val, c.LLVMInt32TypeInContext(self.context));
    return .{ .value = truncated, .type_tag = .i32_type };
}

pub fn lookupFunctionReturnType(self: *CodeGen, name: []const u8) CodeGen.TypeTag {
    for (self.ast.program.decls.items) |decl| {
        switch (decl.decl) {
            .function => |fn_decl| {
                if (std.mem.eql(u8, fn_decl.name, name)) {
                    return self.resolveTypeExpr(fn_decl.return_type);
                }
            },
            .extern_decl => |ed| {
                if (std.mem.eql(u8, ed.name, name)) {
                    return self.resolveTypeExpr(ed.return_type);
                }
            },
            else => {},
        }
    }
    return .i32_type;
}
