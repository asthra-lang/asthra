const std = @import("std");
const Ast = @import("ast.zig").Ast;
const codegen_mod = @import("codegen.zig");
const CodeGen = codegen_mod.CodeGen;
const c = codegen_mod.c;

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
            return genClosureExpr(self, cl);
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
            const pointee_ptr = self.allocator.create(CodeGen.TypeTag) catch return error.CodeGenError;
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

    const elem_ptr = self.allocator.create(CodeGen.TypeTag) catch return error.CodeGenError;
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

    const elem_type_ptr = self.allocator.create(CodeGen.TypeTag) catch return error.CodeGenError;
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
    var elem_tags = self.allocator.alloc(CodeGen.TypeTag, count) catch return error.CodeGenError;
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
    const elem_ptr_tag = self.allocator.create(CodeGen.TypeTag) catch return error.CodeGenError;
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
                return genMathConstant(self, fa.field);
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
                return genStdlibCall(self, name, mc.method, &mc.args);
            }
        },
        else => {},
    }

    // Get the object value
    const obj = try genExpr(self, mc.object);

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
                        type_map.put(tp, resolved) catch {};
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
    return .i32_type;
}

pub fn genBinaryOp(self: *CodeGen, op: Ast.BinaryOp, lhs: CodeGen.ExprResult, rhs: CodeGen.ExprResult) CodeGen.GenError!CodeGen.ExprResult {
    // String concatenation: string + string
    if (op == .add and CodeGen.isTypeTag(lhs.type_tag, .string_type) and CodeGen.isTypeTag(rhs.type_tag, .string_type)) {
        return genStringConcat(self, lhs.value, rhs.value);
    }

    const is_float = CodeGen.isTypeTag(lhs.type_tag, .f64_type);
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
            .closure_type => |ct| return genClosureCall(self, ct, &call_expr.args),
            .fn_type => |ft| return genFnTypeCall(self, nv, ft, &call_expr.args),
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
    if (std.mem.eql(u8, name, "map")) return genMapCall(self, call_expr);
    if (std.mem.eql(u8, name, "filter")) return genFilterCall(self, call_expr);
    if (std.mem.eql(u8, name, "reduce")) return genReduceCall(self, call_expr);

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

    const function = c.LLVMGetNamedFunction(self.module, name_z.ptr);
    if (function == null) {
        self.diagnostics.report(.@"error", 0, "undefined function '{s}'", .{name});
        return error.CodeGenError;
    }

    var args = std.ArrayList(c.LLVMValueRef){};
    defer args.deinit(self.allocator);
    for (call_expr.args.items) |arg_idx| {
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
        const result = c.LLVMBuildCall2(self.builder, fn_type, function, args.items.ptr, @intCast(args.items.len), "call");
        // Look up return type from AST
        const ret_tag = lookupFunctionReturnType(self, name);
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

    if (CodeGen.isTypeTag(arg.type_tag, .i32_type) or CodeGen.isTypeTag(arg.type_tag, .i64_type)) {
        const args_arr = [_]c.LLVMValueRef{ self.fmt_int, arg.value };
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

// ── Standard Library ──────────────────────────────────────────────────────

fn genStdlibCall(self: *CodeGen, namespace: []const u8, func: []const u8, args: *const std.ArrayList(Ast.ExprIndex)) CodeGen.GenError!CodeGen.ExprResult {
    if (std.mem.eql(u8, namespace, "math")) {
        return genMathCall(self, func, args);
    } else if (std.mem.eql(u8, namespace, "str")) {
        return genStrCall(self, func, args);
    } else if (std.mem.eql(u8, namespace, "io")) {
        return genIoCall(self, func, args);
    } else if (std.mem.eql(u8, namespace, "os")) {
        return genOsCall(self, func, args);
    }
    self.diagnostics.report(.@"error", 0, "unknown stdlib namespace '{s}'", .{namespace});
    return error.CodeGenError;
}

fn genMathConstant(self: *CodeGen, field: []const u8) CodeGen.GenError!CodeGen.ExprResult {
    if (std.mem.eql(u8, field, "PI")) {
        return .{
            .value = c.LLVMConstReal(c.LLVMDoubleTypeInContext(self.context), 3.14159265358979323846),
            .type_tag = .f64_type,
        };
    } else if (std.mem.eql(u8, field, "E")) {
        return .{
            .value = c.LLVMConstReal(c.LLVMDoubleTypeInContext(self.context), 2.71828182845904523536),
            .type_tag = .f64_type,
        };
    }
    self.diagnostics.report(.@"error", 0, "unknown math constant '{s}'", .{field});
    return error.CodeGenError;
}

fn ensureF64(self: *CodeGen, result: CodeGen.ExprResult) c.LLVMValueRef {
    return switch (result.type_tag) {
        .i32_type => c.LLVMBuildSIToFP(self.builder, result.value, c.LLVMDoubleTypeInContext(self.context), "i2f"),
        .i64_type => c.LLVMBuildSIToFP(self.builder, result.value, c.LLVMDoubleTypeInContext(self.context), "i64_2f"),
        else => result.value,
    };
}

fn genMathCall(self: *CodeGen, func: []const u8, args: *const std.ArrayList(Ast.ExprIndex)) CodeGen.GenError!CodeGen.ExprResult {
    if (std.mem.eql(u8, func, "sqrt")) {
        if (args.items.len != 1) { self.diagnostics.report(.@"error", 0, "math.sqrt() expects 1 argument", .{}); return error.CodeGenError; }
        const arg = try genExpr(self, args.items[0]);
        const f64_val = ensureF64(self, arg);
        const fn_type = c.LLVMGlobalGetValueType(self.sqrt_fn);
        var call_args = [_]c.LLVMValueRef{f64_val};
        return .{ .value = c.LLVMBuildCall2(self.builder, fn_type, self.sqrt_fn, &call_args, 1, "sqrt_res"), .type_tag = .f64_type };
    } else if (std.mem.eql(u8, func, "pow")) {
        if (args.items.len != 2) { self.diagnostics.report(.@"error", 0, "math.pow() expects 2 arguments", .{}); return error.CodeGenError; }
        const a = try genExpr(self, args.items[0]);
        const b = try genExpr(self, args.items[1]);
        const fn_type = c.LLVMGlobalGetValueType(self.pow_fn);
        var call_args = [_]c.LLVMValueRef{ ensureF64(self, a), ensureF64(self, b) };
        return .{ .value = c.LLVMBuildCall2(self.builder, fn_type, self.pow_fn, &call_args, 2, "pow_res"), .type_tag = .f64_type };
    } else if (std.mem.eql(u8, func, "abs")) {
        if (args.items.len != 1) { self.diagnostics.report(.@"error", 0, "math.abs() expects 1 argument", .{}); return error.CodeGenError; }
        const arg = try genExpr(self, args.items[0]);
        if (arg.type_tag == .i32_type) {
            const zero = c.LLVMConstInt(c.LLVMInt32TypeInContext(self.context), 0, 0);
            const is_neg = c.LLVMBuildICmp(self.builder, c.LLVMIntSLT, arg.value, zero, "is_neg");
            const neg_val = c.LLVMBuildNSWSub(self.builder, zero, arg.value, "neg");
            return .{ .value = c.LLVMBuildSelect(self.builder, is_neg, neg_val, arg.value, "abs_res"), .type_tag = .i32_type };
        } else {
            const f64_val = ensureF64(self, arg);
            const fn_type = c.LLVMGlobalGetValueType(self.fabs_fn);
            var call_args = [_]c.LLVMValueRef{f64_val};
            return .{ .value = c.LLVMBuildCall2(self.builder, fn_type, self.fabs_fn, &call_args, 1, "fabs_res"), .type_tag = .f64_type };
        }
    } else if (std.mem.eql(u8, func, "min")) {
        if (args.items.len != 2) { self.diagnostics.report(.@"error", 0, "math.min() expects 2 arguments", .{}); return error.CodeGenError; }
        const a = try genExpr(self, args.items[0]);
        const b = try genExpr(self, args.items[1]);
        if (a.type_tag == .i32_type and b.type_tag == .i32_type) {
            const cmp = c.LLVMBuildICmp(self.builder, c.LLVMIntSLT, a.value, b.value, "min_cmp");
            return .{ .value = c.LLVMBuildSelect(self.builder, cmp, a.value, b.value, "min_res"), .type_tag = .i32_type };
        } else {
            const fn_type = c.LLVMGlobalGetValueType(self.fmin_fn);
            var call_args = [_]c.LLVMValueRef{ ensureF64(self, a), ensureF64(self, b) };
            return .{ .value = c.LLVMBuildCall2(self.builder, fn_type, self.fmin_fn, &call_args, 2, "fmin_res"), .type_tag = .f64_type };
        }
    } else if (std.mem.eql(u8, func, "max")) {
        if (args.items.len != 2) { self.diagnostics.report(.@"error", 0, "math.max() expects 2 arguments", .{}); return error.CodeGenError; }
        const a = try genExpr(self, args.items[0]);
        const b = try genExpr(self, args.items[1]);
        if (a.type_tag == .i32_type and b.type_tag == .i32_type) {
            const cmp = c.LLVMBuildICmp(self.builder, c.LLVMIntSGT, a.value, b.value, "max_cmp");
            return .{ .value = c.LLVMBuildSelect(self.builder, cmp, a.value, b.value, "max_res"), .type_tag = .i32_type };
        } else {
            const fn_type = c.LLVMGlobalGetValueType(self.fmax_fn);
            var call_args = [_]c.LLVMValueRef{ ensureF64(self, a), ensureF64(self, b) };
            return .{ .value = c.LLVMBuildCall2(self.builder, fn_type, self.fmax_fn, &call_args, 2, "fmax_res"), .type_tag = .f64_type };
        }
    } else if (std.mem.eql(u8, func, "floor")) {
        if (args.items.len != 1) { self.diagnostics.report(.@"error", 0, "math.floor() expects 1 argument", .{}); return error.CodeGenError; }
        const arg = try genExpr(self, args.items[0]);
        const fn_type = c.LLVMGlobalGetValueType(self.floor_fn);
        var call_args = [_]c.LLVMValueRef{ensureF64(self, arg)};
        return .{ .value = c.LLVMBuildCall2(self.builder, fn_type, self.floor_fn, &call_args, 1, "floor_res"), .type_tag = .f64_type };
    } else if (std.mem.eql(u8, func, "ceil")) {
        if (args.items.len != 1) { self.diagnostics.report(.@"error", 0, "math.ceil() expects 1 argument", .{}); return error.CodeGenError; }
        const arg = try genExpr(self, args.items[0]);
        const fn_type = c.LLVMGlobalGetValueType(self.ceil_fn);
        var call_args = [_]c.LLVMValueRef{ensureF64(self, arg)};
        return .{ .value = c.LLVMBuildCall2(self.builder, fn_type, self.ceil_fn, &call_args, 1, "ceil_res"), .type_tag = .f64_type };
    }
    self.diagnostics.report(.@"error", 0, "unknown math function '{s}'", .{func});
    return error.CodeGenError;
}

fn genStrCall(self: *CodeGen, func: []const u8, args: *const std.ArrayList(Ast.ExprIndex)) CodeGen.GenError!CodeGen.ExprResult {
    if (std.mem.eql(u8, func, "to_int")) {
        if (args.items.len != 1) { self.diagnostics.report(.@"error", 0, "str.to_int() expects 1 argument", .{}); return error.CodeGenError; }
        const arg = try genExpr(self, args.items[0]);
        const fn_type = c.LLVMGlobalGetValueType(self.atoi_fn);
        var call_args = [_]c.LLVMValueRef{arg.value};
        return .{ .value = c.LLVMBuildCall2(self.builder, fn_type, self.atoi_fn, &call_args, 1, "atoi_res"), .type_tag = .i32_type };
    } else if (std.mem.eql(u8, func, "to_float")) {
        if (args.items.len != 1) { self.diagnostics.report(.@"error", 0, "str.to_float() expects 1 argument", .{}); return error.CodeGenError; }
        const arg = try genExpr(self, args.items[0]);
        const fn_type = c.LLVMGlobalGetValueType(self.atof_fn);
        var call_args = [_]c.LLVMValueRef{arg.value};
        return .{ .value = c.LLVMBuildCall2(self.builder, fn_type, self.atof_fn, &call_args, 1, "atof_res"), .type_tag = .f64_type };
    } else if (std.mem.eql(u8, func, "from_int")) {
        if (args.items.len != 1) { self.diagnostics.report(.@"error", 0, "str.from_int() expects 1 argument", .{}); return error.CodeGenError; }
        const arg = try genExpr(self, args.items[0]);
        // malloc(32) + sprintf(buf, "%d", n)
        const buf_size = c.LLVMConstInt(c.LLVMInt64TypeInContext(self.context), 32, 0);
        const malloc_fn_type = c.LLVMGlobalGetValueType(self.malloc_fn);
        var malloc_args = [_]c.LLVMValueRef{buf_size};
        const buf = c.LLVMBuildCall2(self.builder, malloc_fn_type, self.malloc_fn, &malloc_args, 1, "buf");
        const sprintf_fn_type = c.LLVMGlobalGetValueType(self.sprintf_fn);
        var sprintf_args = [_]c.LLVMValueRef{ buf, self.fmt_int_raw, arg.value };
        _ = c.LLVMBuildCall2(self.builder, sprintf_fn_type, self.sprintf_fn, &sprintf_args, 3, "");
        return .{ .value = buf, .type_tag = .string_type };
    } else if (std.mem.eql(u8, func, "from_float")) {
        if (args.items.len != 1) { self.diagnostics.report(.@"error", 0, "str.from_float() expects 1 argument", .{}); return error.CodeGenError; }
        const arg = try genExpr(self, args.items[0]);
        const buf_size = c.LLVMConstInt(c.LLVMInt64TypeInContext(self.context), 64, 0);
        const malloc_fn_type = c.LLVMGlobalGetValueType(self.malloc_fn);
        var malloc_args = [_]c.LLVMValueRef{buf_size};
        const buf = c.LLVMBuildCall2(self.builder, malloc_fn_type, self.malloc_fn, &malloc_args, 1, "buf");
        const sprintf_fn_type = c.LLVMGlobalGetValueType(self.sprintf_fn);
        var sprintf_args = [_]c.LLVMValueRef{ buf, self.fmt_float_raw, ensureF64(self, arg) };
        _ = c.LLVMBuildCall2(self.builder, sprintf_fn_type, self.sprintf_fn, &sprintf_args, 3, "");
        return .{ .value = buf, .type_tag = .string_type };
    } else if (std.mem.eql(u8, func, "contains")) {
        if (args.items.len != 2) { self.diagnostics.report(.@"error", 0, "str.contains() expects 2 arguments", .{}); return error.CodeGenError; }
        const s = try genExpr(self, args.items[0]);
        const sub = try genExpr(self, args.items[1]);
        const fn_type = c.LLVMGlobalGetValueType(self.strstr_fn);
        var call_args = [_]c.LLVMValueRef{ s.value, sub.value };
        const result = c.LLVMBuildCall2(self.builder, fn_type, self.strstr_fn, &call_args, 2, "strstr_res");
        const null_ptr = c.LLVMConstNull(c.LLVMPointerType(c.LLVMInt8TypeInContext(self.context), 0));
        const is_not_null = c.LLVMBuildICmp(self.builder, c.LLVMIntNE, result, null_ptr, "contains_res");
        return .{ .value = is_not_null, .type_tag = .bool_type };
    } else if (std.mem.eql(u8, func, "starts_with")) {
        if (args.items.len != 2) { self.diagnostics.report(.@"error", 0, "str.starts_with() expects 2 arguments", .{}); return error.CodeGenError; }
        const s = try genExpr(self, args.items[0]);
        const prefix = try genExpr(self, args.items[1]);
        // strlen(prefix)
        const strlen_fn_type = c.LLVMGlobalGetValueType(self.strlen_fn);
        var strlen_args = [_]c.LLVMValueRef{prefix.value};
        const prefix_len = c.LLVMBuildCall2(self.builder, strlen_fn_type, self.strlen_fn, &strlen_args, 1, "prefix_len");
        // strncmp(s, prefix, prefix_len) == 0
        const strncmp_fn_type = c.LLVMGlobalGetValueType(self.strncmp_fn);
        var cmp_args = [_]c.LLVMValueRef{ s.value, prefix.value, prefix_len };
        const cmp_result = c.LLVMBuildCall2(self.builder, strncmp_fn_type, self.strncmp_fn, &cmp_args, 3, "cmp_res");
        const zero = c.LLVMConstInt(c.LLVMInt32TypeInContext(self.context), 0, 0);
        const is_eq = c.LLVMBuildICmp(self.builder, c.LLVMIntEQ, cmp_result, zero, "starts_with_res");
        return .{ .value = is_eq, .type_tag = .bool_type };
    } else if (std.mem.eql(u8, func, "ends_with")) {
        if (args.items.len != 2) { self.diagnostics.report(.@"error", 0, "str.ends_with() expects 2 arguments", .{}); return error.CodeGenError; }
        const s = try genExpr(self, args.items[0]);
        const suffix = try genExpr(self, args.items[1]);
        const strlen_fn_type = c.LLVMGlobalGetValueType(self.strlen_fn);
        // strlen(s) and strlen(suffix)
        var s_len_args = [_]c.LLVMValueRef{s.value};
        const s_len = c.LLVMBuildCall2(self.builder, strlen_fn_type, self.strlen_fn, &s_len_args, 1, "s_len");
        var suf_len_args = [_]c.LLVMValueRef{suffix.value};
        const suf_len = c.LLVMBuildCall2(self.builder, strlen_fn_type, self.strlen_fn, &suf_len_args, 1, "suf_len");
        // offset = s_len - suf_len
        const offset = c.LLVMBuildSub(self.builder, s_len, suf_len, "offset");
        // Check offset >= 0 (unsigned: suf_len <= s_len)
        const is_long_enough = c.LLVMBuildICmp(self.builder, c.LLVMIntUGE, s_len, suf_len, "long_enough");
        // s + offset
        var gep_offset = [_]c.LLVMValueRef{offset};
        const s_end = c.LLVMBuildGEP2(self.builder, c.LLVMInt8TypeInContext(self.context), s.value, &gep_offset, 1, "s_end");
        // strncmp(s + offset, suffix, suf_len) == 0
        const strncmp_fn_type = c.LLVMGlobalGetValueType(self.strncmp_fn);
        var cmp_args = [_]c.LLVMValueRef{ s_end, suffix.value, suf_len };
        const cmp_result = c.LLVMBuildCall2(self.builder, strncmp_fn_type, self.strncmp_fn, &cmp_args, 3, "cmp_res");
        const zero = c.LLVMConstInt(c.LLVMInt32TypeInContext(self.context), 0, 0);
        const is_match = c.LLVMBuildICmp(self.builder, c.LLVMIntEQ, cmp_result, zero, "match");
        // result = is_long_enough AND is_match
        const result = c.LLVMBuildAnd(self.builder, is_long_enough, is_match, "ends_with_res");
        return .{ .value = result, .type_tag = .bool_type };
    } else if (std.mem.eql(u8, func, "upper") or std.mem.eql(u8, func, "lower")) {
        if (args.items.len != 1) { self.diagnostics.report(.@"error", 0, "str.{s}() expects 1 argument", .{func}); return error.CodeGenError; }
        const arg = try genExpr(self, args.items[0]);
        const conv_fn = if (std.mem.eql(u8, func, "upper")) self.toupper_fn else self.tolower_fn;
        return genStrCaseConvert(self, arg.value, conv_fn);
    } else if (std.mem.eql(u8, func, "trim")) {
        if (args.items.len != 1) { self.diagnostics.report(.@"error", 0, "str.trim() expects 1 argument", .{}); return error.CodeGenError; }
        const arg = try genExpr(self, args.items[0]);
        return genStrTrim(self, arg.value);
    }
    self.diagnostics.report(.@"error", 0, "unknown str function '{s}'", .{func});
    return error.CodeGenError;
}

fn genStrCaseConvert(self: *CodeGen, str_val: c.LLVMValueRef, conv_fn: c.LLVMValueRef) CodeGen.GenError!CodeGen.ExprResult {
    const i8_type = c.LLVMInt8TypeInContext(self.context);
    const i32_llvm = c.LLVMInt32TypeInContext(self.context);
    const i64_llvm = c.LLVMInt64TypeInContext(self.context);

    // Get string length
    const strlen_fn_type = c.LLVMGlobalGetValueType(self.strlen_fn);
    var strlen_args = [_]c.LLVMValueRef{str_val};
    const len = c.LLVMBuildCall2(self.builder, strlen_fn_type, self.strlen_fn, &strlen_args, 1, "len");

    // malloc(len + 1)
    const one = c.LLVMConstInt(i64_llvm, 1, 0);
    const buf_size = c.LLVMBuildAdd(self.builder, len, one, "buf_size");
    const malloc_fn_type = c.LLVMGlobalGetValueType(self.malloc_fn);
    var malloc_args = [_]c.LLVMValueRef{buf_size};
    const buf = c.LLVMBuildCall2(self.builder, malloc_fn_type, self.malloc_fn, &malloc_args, 1, "buf");

    // Get current function for basic blocks
    const current_fn = c.LLVMGetBasicBlockParent(c.LLVMGetInsertBlock(self.builder));

    // Loop: for i in 0..len: buf[i] = conv(src[i])
    const loop_header = c.LLVMAppendBasicBlockInContext(self.context, current_fn, "case_loop");
    const loop_body = c.LLVMAppendBasicBlockInContext(self.context, current_fn, "case_body");
    const loop_end = c.LLVMAppendBasicBlockInContext(self.context, current_fn, "case_end");

    _ = c.LLVMBuildBr(self.builder, loop_header);

    // Loop header: i = phi(0, entry)(i_next, body); if i >= len, exit
    c.LLVMPositionBuilderAtEnd(self.builder, loop_header);
    const i_phi = c.LLVMBuildPhi(self.builder, i64_llvm, "i");
    const done = c.LLVMBuildICmp(self.builder, c.LLVMIntUGE, i_phi, len, "done");
    _ = c.LLVMBuildCondBr(self.builder, done, loop_end, loop_body);

    // Loop body: load char, convert, store
    c.LLVMPositionBuilderAtEnd(self.builder, loop_body);
    var gep_i = [_]c.LLVMValueRef{i_phi};
    const src_ptr = c.LLVMBuildGEP2(self.builder, i8_type, str_val, &gep_i, 1, "src_ptr");
    const ch = c.LLVMBuildLoad2(self.builder, i8_type, src_ptr, "ch");
    const ch_ext = c.LLVMBuildZExt(self.builder, ch, i32_llvm, "ch_ext");
    const conv_fn_type = c.LLVMGlobalGetValueType(conv_fn);
    var conv_args = [_]c.LLVMValueRef{ch_ext};
    const converted = c.LLVMBuildCall2(self.builder, conv_fn_type, conv_fn, &conv_args, 1, "converted");
    const trunc = c.LLVMBuildTrunc(self.builder, converted, i8_type, "trunc");
    var gep_i2 = [_]c.LLVMValueRef{i_phi};
    const dst_ptr = c.LLVMBuildGEP2(self.builder, i8_type, buf, &gep_i2, 1, "dst_ptr");
    _ = c.LLVMBuildStore(self.builder, trunc, dst_ptr);
    const i_next = c.LLVMBuildAdd(self.builder, i_phi, one, "i_next");
    _ = c.LLVMBuildBr(self.builder, loop_header);

    // Wire phi
    const zero_i64 = c.LLVMConstInt(i64_llvm, 0, 0);
    const entry_bb = c.LLVMGetPreviousBasicBlock(loop_header);
    c.LLVMAddIncoming(i_phi, @constCast(&[_]c.LLVMValueRef{zero_i64}), @constCast(&[_]c.LLVMBasicBlockRef{entry_bb}), 1);
    c.LLVMAddIncoming(i_phi, @constCast(&[_]c.LLVMValueRef{i_next}), @constCast(&[_]c.LLVMBasicBlockRef{loop_body}), 1);

    // Null-terminate
    c.LLVMPositionBuilderAtEnd(self.builder, loop_end);
    var gep_len = [_]c.LLVMValueRef{len};
    const null_ptr = c.LLVMBuildGEP2(self.builder, i8_type, buf, &gep_len, 1, "null_ptr");
    _ = c.LLVMBuildStore(self.builder, c.LLVMConstInt(i8_type, 0, 0), null_ptr);

    return .{ .value = buf, .type_tag = .string_type };
}

fn genStrTrim(self: *CodeGen, str_val: c.LLVMValueRef) CodeGen.GenError!CodeGen.ExprResult {
    const i8_type = c.LLVMInt8TypeInContext(self.context);
    const i32_llvm = c.LLVMInt32TypeInContext(self.context);
    const i64_llvm = c.LLVMInt64TypeInContext(self.context);
    const zero_i64 = c.LLVMConstInt(i64_llvm, 0, 0);
    const one_i64 = c.LLVMConstInt(i64_llvm, 1, 0);

    // Get string length
    const strlen_fn_type = c.LLVMGlobalGetValueType(self.strlen_fn);
    var strlen_args = [_]c.LLVMValueRef{str_val};
    const len = c.LLVMBuildCall2(self.builder, strlen_fn_type, self.strlen_fn, &strlen_args, 1, "len");

    const current_fn = c.LLVMGetBasicBlockParent(c.LLVMGetInsertBlock(self.builder));

    // Scan forward: find first non-space
    const fwd_header = c.LLVMAppendBasicBlockInContext(self.context, current_fn, "trim_fwd");
    const fwd_body = c.LLVMAppendBasicBlockInContext(self.context, current_fn, "trim_fwd_body");
    const fwd_done = c.LLVMAppendBasicBlockInContext(self.context, current_fn, "trim_fwd_done");

    _ = c.LLVMBuildBr(self.builder, fwd_header);
    c.LLVMPositionBuilderAtEnd(self.builder, fwd_header);
    const start_phi = c.LLVMBuildPhi(self.builder, i64_llvm, "start");
    const fwd_at_end = c.LLVMBuildICmp(self.builder, c.LLVMIntUGE, start_phi, len, "fwd_at_end");
    _ = c.LLVMBuildCondBr(self.builder, fwd_at_end, fwd_done, fwd_body);

    c.LLVMPositionBuilderAtEnd(self.builder, fwd_body);
    var gep_start = [_]c.LLVMValueRef{start_phi};
    const fwd_ptr = c.LLVMBuildGEP2(self.builder, i8_type, str_val, &gep_start, 1, "fwd_ptr");
    const fwd_ch = c.LLVMBuildLoad2(self.builder, i8_type, fwd_ptr, "fwd_ch");
    const fwd_ch_ext = c.LLVMBuildZExt(self.builder, fwd_ch, i32_llvm, "fwd_ch_ext");
    const isspace_fn_type = c.LLVMGlobalGetValueType(self.isspace_fn);
    var isspace_args = [_]c.LLVMValueRef{fwd_ch_ext};
    const is_space_fwd = c.LLVMBuildCall2(self.builder, isspace_fn_type, self.isspace_fn, &isspace_args, 1, "is_space");
    const is_space_bool = c.LLVMBuildICmp(self.builder, c.LLVMIntNE, is_space_fwd, c.LLVMConstInt(i32_llvm, 0, 0), "is_sp");
    const start_next = c.LLVMBuildAdd(self.builder, start_phi, one_i64, "start_next");
    // If space, continue; else, done
    _ = c.LLVMBuildCondBr(self.builder, is_space_bool, fwd_header, fwd_done);

    // Wire start_phi
    const entry_bb = c.LLVMGetPreviousBasicBlock(fwd_header);
    c.LLVMAddIncoming(start_phi, @constCast(&[_]c.LLVMValueRef{zero_i64}), @constCast(&[_]c.LLVMBasicBlockRef{entry_bb}), 1);
    c.LLVMAddIncoming(start_phi, @constCast(&[_]c.LLVMValueRef{start_next}), @constCast(&[_]c.LLVMBasicBlockRef{fwd_body}), 1);

    // fwd_done: start = phi(start_phi from header, start_phi from body)
    c.LLVMPositionBuilderAtEnd(self.builder, fwd_done);
    const final_start = c.LLVMBuildPhi(self.builder, i64_llvm, "final_start");
    c.LLVMAddIncoming(final_start, @constCast(&[_]c.LLVMValueRef{ start_phi, start_phi }), @constCast(&[_]c.LLVMBasicBlockRef{ fwd_header, fwd_body }), 2);

    // Scan backward: find last non-space (end index, exclusive)
    const bwd_header = c.LLVMAppendBasicBlockInContext(self.context, current_fn, "trim_bwd");
    const bwd_body = c.LLVMAppendBasicBlockInContext(self.context, current_fn, "trim_bwd_body");
    const bwd_done = c.LLVMAppendBasicBlockInContext(self.context, current_fn, "trim_bwd_done");

    _ = c.LLVMBuildBr(self.builder, bwd_header);
    c.LLVMPositionBuilderAtEnd(self.builder, bwd_header);
    const end_phi = c.LLVMBuildPhi(self.builder, i64_llvm, "end");
    const bwd_at_start = c.LLVMBuildICmp(self.builder, c.LLVMIntULE, end_phi, final_start, "bwd_at_start");
    _ = c.LLVMBuildCondBr(self.builder, bwd_at_start, bwd_done, bwd_body);

    c.LLVMPositionBuilderAtEnd(self.builder, bwd_body);
    const end_minus_1 = c.LLVMBuildSub(self.builder, end_phi, one_i64, "end_m1");
    var gep_end_m1 = [_]c.LLVMValueRef{end_minus_1};
    const bwd_ptr = c.LLVMBuildGEP2(self.builder, i8_type, str_val, &gep_end_m1, 1, "bwd_ptr");
    const bwd_ch = c.LLVMBuildLoad2(self.builder, i8_type, bwd_ptr, "bwd_ch");
    const bwd_ch_ext = c.LLVMBuildZExt(self.builder, bwd_ch, i32_llvm, "bwd_ch_ext");
    var bwd_isspace_args = [_]c.LLVMValueRef{bwd_ch_ext};
    const is_space_bwd = c.LLVMBuildCall2(self.builder, isspace_fn_type, self.isspace_fn, &bwd_isspace_args, 1, "is_space_bwd");
    const is_space_bwd_bool = c.LLVMBuildICmp(self.builder, c.LLVMIntNE, is_space_bwd, c.LLVMConstInt(i32_llvm, 0, 0), "is_sp_bwd");
    _ = c.LLVMBuildCondBr(self.builder, is_space_bwd_bool, bwd_header, bwd_done);

    // Wire end_phi
    c.LLVMAddIncoming(end_phi, @constCast(&[_]c.LLVMValueRef{len}), @constCast(&[_]c.LLVMBasicBlockRef{fwd_done}), 1);
    c.LLVMAddIncoming(end_phi, @constCast(&[_]c.LLVMValueRef{end_minus_1}), @constCast(&[_]c.LLVMBasicBlockRef{bwd_body}), 1);

    // bwd_done: end = phi(end_phi from header, end_phi from body)
    c.LLVMPositionBuilderAtEnd(self.builder, bwd_done);
    const final_end = c.LLVMBuildPhi(self.builder, i64_llvm, "final_end");
    c.LLVMAddIncoming(final_end, @constCast(&[_]c.LLVMValueRef{ end_phi, end_phi }), @constCast(&[_]c.LLVMBasicBlockRef{ bwd_header, bwd_body }), 2);

    // Copy trimmed substring: len = final_end - final_start
    const trimmed_len = c.LLVMBuildSub(self.builder, final_end, final_start, "trimmed_len");
    const buf_size = c.LLVMBuildAdd(self.builder, trimmed_len, one_i64, "buf_size");
    const malloc_fn_type = c.LLVMGlobalGetValueType(self.malloc_fn);
    var malloc_args = [_]c.LLVMValueRef{buf_size};
    const buf = c.LLVMBuildCall2(self.builder, malloc_fn_type, self.malloc_fn, &malloc_args, 1, "trim_buf");

    // src = str_val + final_start
    var gep_final_start = [_]c.LLVMValueRef{final_start};
    const src = c.LLVMBuildGEP2(self.builder, i8_type, str_val, &gep_final_start, 1, "trim_src");
    const memcpy_fn_type = c.LLVMGlobalGetValueType(self.memcpy_fn);
    var memcpy_args = [_]c.LLVMValueRef{ buf, src, trimmed_len };
    _ = c.LLVMBuildCall2(self.builder, memcpy_fn_type, self.memcpy_fn, &memcpy_args, 3, "");

    // Null-terminate
    var gep_trimmed = [_]c.LLVMValueRef{trimmed_len};
    const null_pos = c.LLVMBuildGEP2(self.builder, i8_type, buf, &gep_trimmed, 1, "null_pos");
    _ = c.LLVMBuildStore(self.builder, c.LLVMConstInt(i8_type, 0, 0), null_pos);

    return .{ .value = buf, .type_tag = .string_type };
}

fn genIoCall(self: *CodeGen, func: []const u8, args: *const std.ArrayList(Ast.ExprIndex)) CodeGen.GenError!CodeGen.ExprResult {
    const void_val = c.LLVMGetUndef(c.LLVMVoidTypeInContext(self.context));
    if (std.mem.eql(u8, func, "println")) {
        if (args.items.len != 1) { self.diagnostics.report(.@"error", 0, "io.println() expects 1 argument", .{}); return error.CodeGenError; }
        const arg = try genExpr(self, args.items[0]);
        // Use printf with newline format string (same as log)
        const printf_fn_type = c.LLVMGlobalGetValueType(self.printf_fn);
        const fmt = switch (arg.type_tag) {
            .i32_type => self.fmt_int,
            .i64_type => self.fmt_int,
            .f64_type => self.fmt_float,
            .string_type => self.fmt_str,
            .char_type => self.fmt_char,
            .bool_type => return blk: {
                // Same pattern as genLogCall for bools
                const true_str = self.fmt_bool_true;
                const false_str = self.fmt_bool_false;
                const fmt_sel = c.LLVMBuildSelect(self.builder, arg.value, true_str, false_str, "bool_fmt");
                var print_args = [_]c.LLVMValueRef{fmt_sel};
                _ = c.LLVMBuildCall2(self.builder, printf_fn_type, self.printf_fn, &print_args, 1, "");
                break :blk .{ .value = void_val, .type_tag = .void_type };
            },
            else => { self.diagnostics.report(.@"error", 0, "io.println: unsupported type", .{}); return error.CodeGenError; },
        };
        var print_args = [_]c.LLVMValueRef{ fmt, arg.value };
        _ = c.LLVMBuildCall2(self.builder, printf_fn_type, self.printf_fn, &print_args, 2, "");
        return .{ .value = void_val, .type_tag = .void_type };
    } else if (std.mem.eql(u8, func, "print")) {
        if (args.items.len != 1) { self.diagnostics.report(.@"error", 0, "io.print() expects 1 argument", .{}); return error.CodeGenError; }
        const arg = try genExpr(self, args.items[0]);
        // Use printf with no-newline format string
        const printf_fn_type = c.LLVMGlobalGetValueType(self.printf_fn);
        const fmt = switch (arg.type_tag) {
            .i32_type => self.fmt_int_raw,
            .i64_type => self.fmt_int_raw,
            .f64_type => self.fmt_float_raw,
            .string_type => self.fmt_str_raw,
            .char_type => self.fmt_char_raw,
            .bool_type => return blk: {
                const true_str = c.LLVMBuildGlobalStringPtr(self.builder, "true", "true_no_nl");
                const false_str = c.LLVMBuildGlobalStringPtr(self.builder, "false", "false_no_nl");
                const fmt_sel = c.LLVMBuildSelect(self.builder, arg.value, true_str, false_str, "bool_fmt");
                var print_args = [_]c.LLVMValueRef{fmt_sel};
                _ = c.LLVMBuildCall2(self.builder, printf_fn_type, self.printf_fn, &print_args, 1, "");
                break :blk .{ .value = void_val, .type_tag = .void_type };
            },
            else => { self.diagnostics.report(.@"error", 0, "io.print: unsupported type", .{}); return error.CodeGenError; },
        };
        var print_args = [_]c.LLVMValueRef{ fmt, arg.value };
        _ = c.LLVMBuildCall2(self.builder, printf_fn_type, self.printf_fn, &print_args, 2, "");
        return .{ .value = void_val, .type_tag = .void_type };
    } else if (std.mem.eql(u8, func, "read_line")) {
        if (args.items.len != 0) { self.diagnostics.report(.@"error", 0, "io.read_line() expects 0 arguments", .{}); return error.CodeGenError; }
        // malloc(4096)
        const buf_size = c.LLVMConstInt(c.LLVMInt64TypeInContext(self.context), 4096, 0);
        const malloc_fn_type = c.LLVMGlobalGetValueType(self.malloc_fn);
        var malloc_args = [_]c.LLVMValueRef{buf_size};
        const buf = c.LLVMBuildCall2(self.builder, malloc_fn_type, self.malloc_fn, &malloc_args, 1, "read_buf");
        // Store empty string first (in case fgets fails)
        _ = c.LLVMBuildStore(self.builder, c.LLVMConstInt(c.LLVMInt8TypeInContext(self.context), 0, 0), buf);
        // Declare fgets: i8* fgets(i8*, i32, i8*)
        const i8ptr_type = c.LLVMPointerType(c.LLVMInt8TypeInContext(self.context), 0);
        const fgets_params = [_]c.LLVMTypeRef{ i8ptr_type, c.LLVMInt32TypeInContext(self.context), i8ptr_type };
        const fgets_type = c.LLVMFunctionType(i8ptr_type, @constCast(&fgets_params), 3, 0);
        var fgets_fn = c.LLVMGetNamedFunction(self.module, "fgets");
        if (fgets_fn == null) {
            fgets_fn = c.LLVMAddFunction(self.module, "fgets", fgets_type);
        }
        // Get stdin - platform-specific (macOS: __stdinp)
        var stdin_global = c.LLVMGetNamedGlobal(self.module, "__stdinp");
        if (stdin_global == null) {
            stdin_global = c.LLVMAddGlobal(self.module, i8ptr_type, "__stdinp");
            c.LLVMSetExternallyInitialized(stdin_global, 1);
        }
        const stdin_val = c.LLVMBuildLoad2(self.builder, i8ptr_type, stdin_global, "stdin");
        var fgets_args = [_]c.LLVMValueRef{ buf, c.LLVMConstInt(c.LLVMInt32TypeInContext(self.context), 4096, 0), stdin_val };
        _ = c.LLVMBuildCall2(self.builder, fgets_type, fgets_fn, &fgets_args, 3, "");
        // Strip trailing newline
        const strlen_fn_type = c.LLVMGlobalGetValueType(self.strlen_fn);
        var strlen_args = [_]c.LLVMValueRef{buf};
        const read_len = c.LLVMBuildCall2(self.builder, strlen_fn_type, self.strlen_fn, &strlen_args, 1, "read_len");
        const has_content = c.LLVMBuildICmp(self.builder, c.LLVMIntUGT, read_len, c.LLVMConstInt(c.LLVMInt64TypeInContext(self.context), 0, 0), "has_content");
        const current_fn = c.LLVMGetBasicBlockParent(c.LLVMGetInsertBlock(self.builder));
        const strip_bb = c.LLVMAppendBasicBlockInContext(self.context, current_fn, "strip_nl");
        const done_bb = c.LLVMAppendBasicBlockInContext(self.context, current_fn, "read_done");
        _ = c.LLVMBuildCondBr(self.builder, has_content, strip_bb, done_bb);
        c.LLVMPositionBuilderAtEnd(self.builder, strip_bb);
        const last_idx = c.LLVMBuildSub(self.builder, read_len, c.LLVMConstInt(c.LLVMInt64TypeInContext(self.context), 1, 0), "last_idx");
        var gep_last = [_]c.LLVMValueRef{last_idx};
        const last_ptr = c.LLVMBuildGEP2(self.builder, c.LLVMInt8TypeInContext(self.context), buf, &gep_last, 1, "last_ptr");
        const last_ch = c.LLVMBuildLoad2(self.builder, c.LLVMInt8TypeInContext(self.context), last_ptr, "last_ch");
        const is_nl = c.LLVMBuildICmp(self.builder, c.LLVMIntEQ, last_ch, c.LLVMConstInt(c.LLVMInt8TypeInContext(self.context), 10, 0), "is_nl");
        const strip_bb2 = c.LLVMAppendBasicBlockInContext(self.context, current_fn, "do_strip");
        _ = c.LLVMBuildCondBr(self.builder, is_nl, strip_bb2, done_bb);
        c.LLVMPositionBuilderAtEnd(self.builder, strip_bb2);
        _ = c.LLVMBuildStore(self.builder, c.LLVMConstInt(c.LLVMInt8TypeInContext(self.context), 0, 0), last_ptr);
        _ = c.LLVMBuildBr(self.builder, done_bb);
        c.LLVMPositionBuilderAtEnd(self.builder, done_bb);
        return .{ .value = buf, .type_tag = .string_type };
    }
    self.diagnostics.report(.@"error", 0, "unknown io function '{s}'", .{func});
    return error.CodeGenError;
}

fn genOsCall(self: *CodeGen, func: []const u8, args: *const std.ArrayList(Ast.ExprIndex)) CodeGen.GenError!CodeGen.ExprResult {
    if (std.mem.eql(u8, func, "getenv")) {
        if (args.items.len != 1) { self.diagnostics.report(.@"error", 0, "os.getenv() expects 1 argument", .{}); return error.CodeGenError; }
        const arg = try genExpr(self, args.items[0]);
        const fn_type = c.LLVMGlobalGetValueType(self.getenv_fn);
        var call_args = [_]c.LLVMValueRef{arg.value};
        const result = c.LLVMBuildCall2(self.builder, fn_type, self.getenv_fn, &call_args, 1, "getenv_res");
        // If null, return empty string
        const null_ptr = c.LLVMConstNull(c.LLVMPointerType(c.LLVMInt8TypeInContext(self.context), 0));
        const is_null = c.LLVMBuildICmp(self.builder, c.LLVMIntEQ, result, null_ptr, "is_null");
        const empty_str = c.LLVMBuildGlobalStringPtr(self.builder, "", "empty_str");
        const final_val = c.LLVMBuildSelect(self.builder, is_null, empty_str, result, "env_val");
        return .{ .value = final_val, .type_tag = .string_type };
    } else if (std.mem.eql(u8, func, "clock")) {
        if (args.items.len != 0) { self.diagnostics.report(.@"error", 0, "os.clock() expects 0 arguments", .{}); return error.CodeGenError; }
        const fn_type = c.LLVMGlobalGetValueType(self.clock_fn);
        const clock_val = c.LLVMBuildCall2(self.builder, fn_type, self.clock_fn, null, 0, "clock_res");
        // Convert to f64 and divide by CLOCKS_PER_SEC (1000000 on POSIX)
        const f64_val = c.LLVMBuildSIToFP(self.builder, clock_val, c.LLVMDoubleTypeInContext(self.context), "clock_f64");
        const divisor = c.LLVMConstReal(c.LLVMDoubleTypeInContext(self.context), 1000000.0);
        const seconds = c.LLVMBuildFDiv(self.builder, f64_val, divisor, "seconds");
        return .{ .value = seconds, .type_tag = .f64_type };
    } else if (std.mem.eql(u8, func, "args")) {
        if (args.items.len != 0) { self.diagnostics.report(.@"error", 0, "os.args() expects 0 arguments", .{}); return error.CodeGenError; }
        const i8ptr_ty = c.LLVMPointerType(c.LLVMInt8TypeInContext(self.context), 0);
        const i8ptr_ptr_ty = c.LLVMPointerType(i8ptr_ty, 0);
        // Load argc and argv from globals
        const argc = c.LLVMBuildLoad2(self.builder, c.LLVMInt32TypeInContext(self.context), self.argc_global, "argc");
        const argv = c.LLVMBuildLoad2(self.builder, i8ptr_ptr_ty, self.argv_global, "argv");
        // Build slice struct { ptr, len }
        const slice_llvm = self.sliceLLVMType();
        const slice_alloca = c.LLVMBuildAlloca(self.builder, slice_llvm, "args_slice");
        const ptr_field = c.LLVMBuildStructGEP2(self.builder, slice_llvm, slice_alloca, 0, "args_ptr_f");
        _ = c.LLVMBuildStore(self.builder, c.LLVMBuildBitCast(self.builder, argv, i8ptr_ty, "argv_cast"), ptr_field);
        const len_field = c.LLVMBuildStructGEP2(self.builder, slice_llvm, slice_alloca, 1, "args_len_f");
        _ = c.LLVMBuildStore(self.builder, argc, len_field);
        const slice_val = c.LLVMBuildLoad2(self.builder, slice_llvm, slice_alloca, "args_val");
        const elem_ptr = self.allocator.create(CodeGen.TypeTag) catch return error.CodeGenError;
        elem_ptr.* = .string_type;
        return .{ .value = slice_val, .type_tag = .{ .slice_type = .{ .element_type = elem_ptr } } };
    }
    self.diagnostics.report(.@"error", 0, "unknown os function '{s}'", .{func});
    return error.CodeGenError;
}

// ── Closures ──────────────────────────────────────────────────────────────

fn genClosureExpr(self: *CodeGen, cl: Ast.ClosureExpr) CodeGen.GenError!CodeGen.ExprResult {
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

fn collectFreeVars(self: *CodeGen, block: *const Ast.Block, param_names: *const std.StringHashMap(void), captured: *std.StringHashMap(CodeGen.NamedValue)) void {
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

fn collectFreeVarsExpr(self: *CodeGen, expr_idx: Ast.ExprIndex, param_names: *const std.StringHashMap(void), captured: *std.StringHashMap(CodeGen.NamedValue)) void {
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

fn genFnTypeCall(self: *CodeGen, nv: CodeGen.NamedValue, ft: CodeGen.FnTypeTag, args: *const std.ArrayList(Ast.ExprIndex)) CodeGen.GenError!CodeGen.ExprResult {
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
        const arg_val = try genExpr(self, arg_idx);
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

fn genClosureCall(self: *CodeGen, ct: CodeGen.ClosureTypeTag, args: *const std.ArrayList(Ast.ExprIndex)) CodeGen.GenError!CodeGen.ExprResult {
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
        const arg_val = try genExpr(self, arg_idx);
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

// ── Higher-Order Array Functions ──────────────────────────────────────────

/// Helper: call a closure descriptor with LLVM values (not AST ExprIndex)
fn callClosureWithValues(self: *CodeGen, desc_alloca: c.LLVMValueRef, ft: CodeGen.FnTypeTag, values: []const c.LLVMValueRef) CodeGen.GenError!c.LLVMValueRef {
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

    return c.LLVMBuildCall2(self.builder, callee_fn_type, fn_ptr_typed, call_args.items.ptr, @intCast(call_args.items.len), "hof_call");
}

fn getArrayAllocaFromArg(self: *CodeGen, arg_idx: Ast.ExprIndex) ?struct { alloca: c.LLVMValueRef, arr: CodeGen.ArrayTypeTag } {
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

fn genMapCall(self: *CodeGen, call_expr: Ast.CallExpr) CodeGen.GenError!CodeGen.ExprResult {
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
    const fn_result = try genExpr(self, call_expr.args.items[1]);
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

    const mapped_val = try callClosureWithValues(self, fn_alloca, ft, &[_]c.LLVMValueRef{in_val});

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

    const out_elem_ptr = self.allocator.create(CodeGen.TypeTag) catch return error.CodeGenError;
    out_elem_ptr.* = out_elem_type;
    return .{ .value = result, .type_tag = .{ .array_type = .{ .element_type = out_elem_ptr, .count = count } } };
}

fn genReduceCall(self: *CodeGen, call_expr: Ast.CallExpr) CodeGen.GenError!CodeGen.ExprResult {
    if (call_expr.args.items.len != 3) { self.diagnostics.report(.@"error", 0, "reduce() expects 3 arguments (array, init, function)", .{}); return error.CodeGenError; }

    const arr_info = getArrayAllocaFromArg(self, call_expr.args.items[0]) orelse {
        self.diagnostics.report(.@"error", 0, "reduce() first argument must be an array variable", .{});
        return error.CodeGenError;
    };
    const count = arr_info.arr.count;
    const elem_type = arr_info.arr.element_type.*;
    const elem_llvm = self.typeTagToLLVM(elem_type);
    const input_array_llvm = self.typeTagToLLVM(.{ .array_type = arr_info.arr });

    const init_val = try genExpr(self, call_expr.args.items[1]);

    const fn_result = try genExpr(self, call_expr.args.items[2]);
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

    const new_acc = try callClosureWithValues(self, fn_alloca, ft, &[_]c.LLVMValueRef{ acc_phi, in_val });

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

fn genFilterCall(self: *CodeGen, call_expr: Ast.CallExpr) CodeGen.GenError!CodeGen.ExprResult {
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

    const fn_result = try genExpr(self, call_expr.args.items[1]);
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

    const pred_result = try callClosureWithValues(self, fn_alloca, ft, &[_]c.LLVMValueRef{in_val});
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

    const elem_ptr = self.allocator.create(CodeGen.TypeTag) catch return error.CodeGenError;
    elem_ptr.* = elem_type;
    return .{ .value = slice_val, .type_tag = .{ .slice_type = .{ .element_type = elem_ptr } } };
}
