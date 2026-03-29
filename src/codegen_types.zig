const std = @import("std");
const Ast = @import("ast.zig").Ast;
const codegen_mod = @import("codegen.zig");
const CodeGen = codegen_mod.CodeGen;
const c = codegen_mod.c;

pub fn genStructType(self: *CodeGen, sd: *const Ast.StructDecl) CodeGen.GenError!void {
    // If this struct has type parameters, it's a generic struct template.
    // Don't generate an LLVM type yet - it will be monomorphized on demand.
    if (sd.type_params.items.len > 0) {
        self.generic_struct_decls.put(sd.name, sd) catch {};
        return;
    }

    const name_z = self.allocator.dupeZ(u8, sd.name) catch return error.CodeGenError;
    defer self.allocator.free(name_z);

    const field_count: u32 = @intCast(sd.fields.items.len);
    var field_llvm_types = self.allocator.alloc(c.LLVMTypeRef, field_count) catch return error.CodeGenError;
    defer self.allocator.free(field_llvm_types);

    var field_names = self.allocator.alloc([]const u8, field_count) catch return error.CodeGenError;
    var field_type_tags = self.allocator.alloc(CodeGen.TypeTag, field_count) catch return error.CodeGenError;

    for (sd.fields.items, 0..) |field, i| {
        const ft = self.resolveTypeExpr(field.type_expr);
        field_llvm_types[i] = self.typeTagToLLVM(ft);
        field_names[i] = field.name;
        field_type_tags[i] = ft;
    }

    const struct_type = c.LLVMStructCreateNamed(self.context, name_z.ptr);
    c.LLVMStructSetBody(struct_type, field_llvm_types.ptr, field_count, 0);

    self.struct_types.put(sd.name, .{
        .llvm_type = struct_type,
        .field_names = field_names,
        .field_types = field_type_tags,
    }) catch {};
}

pub fn monomorphizeStruct(self: *CodeGen, generic_name: []const u8, type_args: []const Ast.TypeExpr) CodeGen.GenError![]const u8 {
    // Build the mangled name: e.g., "Pair_i32" or "Pair_i32_f64"
    var name_buf = std.ArrayList(u8){};
    defer name_buf.deinit(self.allocator);
    name_buf.appendSlice(self.allocator, generic_name) catch return error.CodeGenError;
    for (type_args) |ta| {
        name_buf.append(self.allocator, '_') catch return error.CodeGenError;
        const type_name = self.typeExprName(ta);
        name_buf.appendSlice(self.allocator, type_name) catch return error.CodeGenError;
    }
    // Check if already monomorphized - return the existing key to avoid duplicate allocation
    if (self.struct_types.getKey(name_buf.items)) |existing_key| {
        return existing_key;
    }

    const mangled_name = self.allocator.dupe(u8, name_buf.items) catch return error.CodeGenError;
    self.monomorphized_names.append(self.allocator, mangled_name) catch {};

    // Look up the generic struct declaration
    const sd = self.generic_struct_decls.get(generic_name) orelse {
        self.diagnostics.report(.@"error", 0, "undefined generic struct '{s}'", .{generic_name});
        return error.CodeGenError;
    };

    // Build type parameter substitution map
    var type_map = std.StringHashMap(CodeGen.TypeTag).init(self.allocator);
    defer type_map.deinit();
    for (sd.type_params.items, 0..) |tp, i| {
        if (i < type_args.len) {
            const resolved = self.resolveTypeExpr(type_args[i]);
            type_map.put(tp, resolved) catch return error.CodeGenError;
        }
    }

    // Generate the concrete struct type with substituted fields
    const field_count: u32 = @intCast(sd.fields.items.len);
    var field_llvm_types = self.allocator.alloc(c.LLVMTypeRef, field_count) catch return error.CodeGenError;
    defer self.allocator.free(field_llvm_types);
    var field_names = self.allocator.alloc([]const u8, field_count) catch return error.CodeGenError;
    var field_type_tags = self.allocator.alloc(CodeGen.TypeTag, field_count) catch return error.CodeGenError;

    for (sd.fields.items, 0..) |field, i| {
        const ft = resolveTypeExprWithSubst(self, field.type_expr, &type_map);
        field_llvm_types[i] = self.typeTagToLLVM(ft);
        field_names[i] = field.name;
        field_type_tags[i] = ft;
    }

    const name_z = self.allocator.dupeZ(u8, mangled_name) catch return error.CodeGenError;
    defer self.allocator.free(name_z);

    const struct_type = c.LLVMStructCreateNamed(self.context, name_z.ptr);
    c.LLVMStructSetBody(struct_type, field_llvm_types.ptr, field_count, 0);

    self.struct_types.put(mangled_name, .{
        .llvm_type = struct_type,
        .field_names = field_names,
        .field_types = field_type_tags,
    }) catch {};

    // Track origin so we can look up generic impls later
    self.monomorphized_origins.put(mangled_name, .{
        .generic_name = generic_name,
        .type_args = type_args,
    }) catch {};

    // Check if there's a generic impl for this struct and generate methods
    if (self.generic_impl_decls.get(generic_name)) |impl_decl| {
        genGenericImplMethods(self, mangled_name, impl_decl, sd, type_args) catch {};
    }

    return mangled_name;
}

pub fn monomorphizeEnum(self: *CodeGen, generic_name: []const u8, type_args: []const Ast.TypeExpr) CodeGen.GenError![]const u8 {
    // Build the mangled name: e.g., "Either_i32_string"
    var name_buf = std.ArrayList(u8){};
    defer name_buf.deinit(self.allocator);
    name_buf.appendSlice(self.allocator, generic_name) catch return error.CodeGenError;
    for (type_args) |ta| {
        name_buf.append(self.allocator, '_') catch return error.CodeGenError;
        const type_name = self.typeExprName(ta);
        name_buf.appendSlice(self.allocator, type_name) catch return error.CodeGenError;
    }
    // Check if already monomorphized
    if (self.enum_types.getKey(name_buf.items)) |existing_key| {
        return existing_key;
    }

    const mangled_name = self.allocator.dupe(u8, name_buf.items) catch return error.CodeGenError;
    self.monomorphized_names.append(self.allocator, mangled_name) catch {};

    // Look up the generic enum declaration
    const ed = self.generic_enum_decls.get(generic_name) orelse {
        self.diagnostics.report(.@"error", 0, "undefined generic enum '{s}'", .{generic_name});
        return error.CodeGenError;
    };

    // Build type parameter substitution map
    var type_map = std.StringHashMap(CodeGen.TypeTag).init(self.allocator);
    defer type_map.deinit();
    for (ed.type_params.items, 0..) |tp, i| {
        if (i < type_args.len) {
            const resolved = self.resolveTypeExpr(type_args[i]);
            type_map.put(tp, resolved) catch return error.CodeGenError;
        }
    }

    // Generate the concrete enum type with substituted variant data types
    const variant_count: usize = ed.variants.items.len;
    var variant_names = self.allocator.alloc([]const u8, variant_count) catch return error.CodeGenError;
    var variant_data_types = self.allocator.alloc([]const CodeGen.TypeTag, variant_count) catch return error.CodeGenError;

    var max_payload_size: usize = 0;
    for (ed.variants.items, 0..) |variant, i| {
        variant_names[i] = variant.name;
        const dt_count = variant.data_types.items.len;
        var dts = self.allocator.alloc(CodeGen.TypeTag, dt_count) catch return error.CodeGenError;
        var size: usize = 0;
        for (variant.data_types.items, 0..) |dt, j| {
            dts[j] = resolveTypeExprWithSubst(self, dt, &type_map);
            size += 8;
        }
        variant_data_types[i] = dts;
        if (size > max_payload_size) max_payload_size = size;
    }

    // Enum LLVM type: { i32 tag, [max_payload_size x i8] data }
    var field_types: [2]c.LLVMTypeRef = undefined;
    field_types[0] = c.LLVMInt32TypeInContext(self.context);
    if (max_payload_size > 0) {
        field_types[1] = c.LLVMArrayType(c.LLVMInt8TypeInContext(self.context), @intCast(max_payload_size));
    } else {
        field_types[1] = c.LLVMArrayType(c.LLVMInt8TypeInContext(self.context), 0);
    }

    const name_z = self.allocator.dupeZ(u8, mangled_name) catch return error.CodeGenError;
    defer self.allocator.free(name_z);

    const enum_type = c.LLVMStructCreateNamed(self.context, name_z.ptr);
    c.LLVMStructSetBody(enum_type, &field_types, 2, 0);

    self.enum_types.put(mangled_name, .{
        .llvm_type = enum_type,
        .variant_names = variant_names,
        .variant_data_types = variant_data_types,
    }) catch {};

    return mangled_name;
}

pub fn resolveTypeExprWithSubst(self: *CodeGen, type_expr: Ast.TypeExpr, type_map: *const std.StringHashMap(CodeGen.TypeTag)) CodeGen.TypeTag {
    return switch (type_expr) {
        .named => |name| {
            // Check if it's a type parameter that should be substituted
            if (type_map.get(name)) |resolved| {
                return resolved;
            }
            return self.resolveTypeExpr(type_expr);
        },
        else => self.resolveTypeExpr(type_expr),
    };
}

pub fn genEnumType(self: *CodeGen, ed: *const Ast.EnumDecl) CodeGen.GenError!void {
    // If this enum has type parameters, it's a generic enum template.
    // Don't generate an LLVM type yet - it will be monomorphized on demand.
    if (ed.type_params.items.len > 0) {
        self.generic_enum_decls.put(ed.name, ed) catch {};
        return;
    }

    const name_z = self.allocator.dupeZ(u8, ed.name) catch return error.CodeGenError;
    defer self.allocator.free(name_z);

    const variant_count: usize = ed.variants.items.len;
    var variant_names = self.allocator.alloc([]const u8, variant_count) catch return error.CodeGenError;
    var variant_data_types = self.allocator.alloc([]const CodeGen.TypeTag, variant_count) catch return error.CodeGenError;

    // Calculate max payload size
    var max_payload_size: usize = 0;
    for (ed.variants.items, 0..) |variant, i| {
        variant_names[i] = variant.name;
        const dt_count = variant.data_types.items.len;
        var dts = self.allocator.alloc(CodeGen.TypeTag, dt_count) catch return error.CodeGenError;
        var size: usize = 0;
        for (variant.data_types.items, 0..) |dt, j| {
            dts[j] = self.resolveTypeExpr(dt);
            size += 8; // assume 8 bytes per field for simplicity
        }
        variant_data_types[i] = dts;
        if (size > max_payload_size) max_payload_size = size;
    }

    // Enum LLVM type: { i32 tag, [max_payload_size x i8] data }
    var field_types: [2]c.LLVMTypeRef = undefined;
    field_types[0] = c.LLVMInt32TypeInContext(self.context); // tag
    if (max_payload_size > 0) {
        field_types[1] = c.LLVMArrayType(c.LLVMInt8TypeInContext(self.context), @intCast(max_payload_size));
    } else {
        field_types[1] = c.LLVMArrayType(c.LLVMInt8TypeInContext(self.context), 0);
    }

    const enum_type = c.LLVMStructCreateNamed(self.context, name_z.ptr);
    c.LLVMStructSetBody(enum_type, &field_types, 2, 0);

    self.enum_types.put(ed.name, .{
        .llvm_type = enum_type,
        .variant_names = variant_names,
        .variant_data_types = variant_data_types,
    }) catch {};
}

pub fn ensureOptionType(self: *CodeGen, inner_tag: CodeGen.TypeTag) void {
    if (self.enum_types.contains("Option")) return;

    // Option has two variants: Some(T) and None
    var variant_names = self.allocator.alloc([]const u8, 2) catch return;
    variant_names[0] = "Some";
    variant_names[1] = "None";

    var variant_data_types = self.allocator.alloc([]const CodeGen.TypeTag, 2) catch return;

    // Some variant has one data field of the inner type
    var some_types = self.allocator.alloc(CodeGen.TypeTag, 1) catch return;
    some_types[0] = inner_tag;
    variant_data_types[0] = some_types;

    // None variant has no data
    var none_types = self.allocator.alloc(CodeGen.TypeTag, 0) catch return;
    variant_data_types[1] = none_types;
    _ = &none_types;

    // Calculate max payload size (8 bytes for one field)
    const max_payload_size: u32 = 8;

    var field_types: [2]c.LLVMTypeRef = undefined;
    field_types[0] = c.LLVMInt32TypeInContext(self.context); // tag
    field_types[1] = c.LLVMArrayType(c.LLVMInt8TypeInContext(self.context), max_payload_size);

    const enum_type = c.LLVMStructCreateNamed(self.context, "Option");
    c.LLVMStructSetBody(enum_type, &field_types, 2, 0);

    self.enum_types.put("Option", .{
        .llvm_type = enum_type,
        .variant_names = variant_names,
        .variant_data_types = variant_data_types,
    }) catch {};
}

pub fn ensureResultType(self: *CodeGen, ok_tag: CodeGen.TypeTag, err_tag: CodeGen.TypeTag) void {
    if (self.enum_types.contains("Result")) return;

    // Result has two variants: Ok(T) and Err(E)
    var variant_names = self.allocator.alloc([]const u8, 2) catch return;
    variant_names[0] = "Ok";
    variant_names[1] = "Err";

    var variant_data_types = self.allocator.alloc([]const CodeGen.TypeTag, 2) catch return;

    // Ok variant has one data field of the ok type
    var ok_types = self.allocator.alloc(CodeGen.TypeTag, 1) catch return;
    ok_types[0] = ok_tag;
    variant_data_types[0] = ok_types;

    // Err variant has one data field of the err type
    var err_types = self.allocator.alloc(CodeGen.TypeTag, 1) catch return;
    err_types[0] = err_tag;
    variant_data_types[1] = err_types;

    // Max payload: max(8, 8) = 8 bytes (each field takes 8 bytes)
    const max_payload_size: u32 = 8;

    var field_types: [2]c.LLVMTypeRef = undefined;
    field_types[0] = c.LLVMInt32TypeInContext(self.context); // tag
    field_types[1] = c.LLVMArrayType(c.LLVMInt8TypeInContext(self.context), max_payload_size);

    const enum_type = c.LLVMStructCreateNamed(self.context, "Result");
    c.LLVMStructSetBody(enum_type, &field_types, 2, 0);

    self.enum_types.put("Result", .{
        .llvm_type = enum_type,
        .variant_names = variant_names,
        .variant_data_types = variant_data_types,
    }) catch {};
}

pub fn genGenericImplMethods(
    self: *CodeGen,
    mangled_name: []const u8,
    impl_decl: *const Ast.ImplDecl,
    sd: *const Ast.StructDecl,
    type_args: []const Ast.TypeExpr,
) CodeGen.GenError!void {
    // Build type parameter substitution map
    var type_map = std.StringHashMap(CodeGen.TypeTag).init(self.allocator);
    defer type_map.deinit();
    for (sd.type_params.items, 0..) |tp, i| {
        if (i < type_args.len) {
            const resolved = self.resolveTypeExpr(type_args[i]);
            type_map.put(tp, resolved) catch return error.CodeGenError;
        }
    }

    // Save builder position and named_values since we may be in the middle of generating another function
    const saved_bb = c.LLVMGetInsertBlock(self.builder);
    var saved_values = std.StringHashMap(CodeGen.NamedValue).init(self.allocator);
    defer saved_values.deinit();
    var it = self.named_values.iterator();
    while (it.next()) |entry| {
        saved_values.put(entry.key_ptr.*, entry.value_ptr.*) catch {};
    }

    for (impl_decl.methods.items) |method| {
        try genMethodWithSubst(self, mangled_name, &method, &type_map);
    }

    // Restore builder position and named_values
    if (saved_bb != null) {
        c.LLVMPositionBuilderAtEnd(self.builder, saved_bb);
    }
    self.named_values.clearRetainingCapacity();
    var it2 = saved_values.iterator();
    while (it2.next()) |entry| {
        self.named_values.put(entry.key_ptr.*, entry.value_ptr.*) catch {};
    }
}

pub fn genMethodWithSubst(self: *CodeGen, type_name: []const u8, method: *const Ast.MethodDecl, type_map: *const std.StringHashMap(CodeGen.TypeTag)) CodeGen.GenError!void {
    const return_type_tag = resolveTypeExprWithSubst(self, method.return_type, type_map);
    const llvm_return_type = self.typeTagToLLVM(return_type_tag);

    var param_types = std.ArrayList(c.LLVMTypeRef){};
    defer param_types.deinit(self.allocator);

    // If method has self, first param is the struct type
    if (method.has_self) {
        const self_type = CodeGen.TypeTag{ .struct_type = type_name };
        try param_types.append(self.allocator, self.typeTagToLLVM(self_type));
    }

    for (method.params.items) |param| {
        const pt = resolveTypeExprWithSubst(self, param.type_expr, type_map);
        try param_types.append(self.allocator, self.typeTagToLLVM(pt));
    }

    const fn_type = c.LLVMFunctionType(llvm_return_type, param_types.items.ptr, @intCast(param_types.items.len), 0);

    // Mangled name: TypeName_methodName
    const mangled_slice = std.fmt.allocPrint(self.allocator, "{s}_{s}", .{ type_name, method.name }) catch return error.CodeGenError;
    defer self.allocator.free(mangled_slice);
    const mangled = self.allocator.dupeZ(u8, mangled_slice) catch return error.CodeGenError;
    defer self.allocator.free(mangled);

    // Skip if already generated
    if (c.LLVMGetNamedFunction(self.module, mangled.ptr) != null) return;

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
        const pt = resolveTypeExprWithSubst(self, param.type_expr, type_map);

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

    try self.genBlock(&method.body, false);

    const current_bb = c.LLVMGetInsertBlock(self.builder);
    if (c.LLVMGetBasicBlockTerminator(current_bb) == null) {
        if (CodeGen.isTypeTag(return_type_tag, .void_type)) {
            _ = c.LLVMBuildRetVoid(self.builder);
        }
    }
}

pub fn genEnumConstructor(self: *CodeGen, enum_name: []const u8, variant_name: []const u8, args: *const std.ArrayList(Ast.ExprIndex)) CodeGen.GenError!CodeGen.ExprResult {
    // Generate all arg values upfront (needed for type inference with generic enums)
    var arg_vals = std.ArrayList(CodeGen.ExprResult){};
    defer arg_vals.deinit(self.allocator);
    for (args.items) |arg_idx| {
        const val = try self.genExpr(arg_idx);
        try arg_vals.append(self.allocator, val);
    }

    // For generic enums, resolve to the correct monomorphized version
    const resolved_name = if (self.enum_types.contains(enum_name))
        enum_name
    else if (self.generic_enum_decls.contains(enum_name)) blk: {
        // Search through monomorphized enum types to find a match
        var it = self.enum_types.iterator();
        while (it.next()) |entry| {
            const key = entry.key_ptr.*;
            if (std.mem.startsWith(u8, key, enum_name) and key.len > enum_name.len and key[enum_name.len] == '_') {
                const einfo = entry.value_ptr.*;
                // Find the variant in this instantiation
                for (einfo.variant_names, 0..) |vn, vi| {
                    if (std.mem.eql(u8, vn, variant_name)) {
                        const data_types = einfo.variant_data_types[vi];
                        if (data_types.len == arg_vals.items.len) {
                            var match = true;
                            for (data_types, 0..) |dt, di| {
                                const dt_tag: @typeInfo(CodeGen.TypeTag).@"union".tag_type.? = dt;
                                const arg_tag: @typeInfo(CodeGen.TypeTag).@"union".tag_type.? = arg_vals.items[di].type_tag;
                                if (dt_tag != arg_tag) {
                                    match = false;
                                    break;
                                }
                            }
                            if (match) break :blk key;
                        }
                        break;
                    }
                }
            }
        }
        self.diagnostics.report(.@"error", 0, "generic enum '{s}' has not been instantiated with matching type arguments", .{enum_name});
        return error.CodeGenError;
    } else {
        self.diagnostics.report(.@"error", 0, "undefined enum '{s}'", .{enum_name});
        return error.CodeGenError;
    };

    const info = self.enum_types.get(resolved_name) orelse {
        self.diagnostics.report(.@"error", 0, "undefined enum '{s}'", .{enum_name});
        return error.CodeGenError;
    };

    // Find variant index
    var variant_idx: ?usize = null;
    for (info.variant_names, 0..) |vn, i| {
        if (std.mem.eql(u8, vn, variant_name)) {
            variant_idx = i;
            break;
        }
    }
    const idx = variant_idx orelse {
        self.diagnostics.report(.@"error", 0, "undefined variant '{s}' on enum '{s}'", .{ variant_name, enum_name });
        return error.CodeGenError;
    };

    // Allocate enum on stack
    const alloca = c.LLVMBuildAlloca(self.builder, info.llvm_type, "enum_lit");

    // Set tag
    const tag_ptr = c.LLVMBuildStructGEP2(self.builder, info.llvm_type, alloca, 0, "tag_ptr");
    _ = c.LLVMBuildStore(self.builder, c.LLVMConstInt(c.LLVMInt32TypeInContext(self.context), @intCast(idx), 0), tag_ptr);

    // Store data fields if any
    const data_types = info.variant_data_types[idx];
    if (data_types.len > 0 and arg_vals.items.len > 0) {
        const data_ptr = c.LLVMBuildStructGEP2(self.builder, info.llvm_type, alloca, 1, "data_ptr");
        // Cast data array to pointer
        const data_byte_ptr = c.LLVMBuildBitCast(self.builder, data_ptr, c.LLVMPointerType(c.LLVMInt8TypeInContext(self.context), 0), "data_bytes");

        var offset: u64 = 0;
        for (arg_vals.items, 0..) |val, i| {
            if (i >= data_types.len) break;
            const field_type = self.typeTagToLLVM(data_types[i]);
            var gep_indices = [_]c.LLVMValueRef{c.LLVMConstInt(c.LLVMInt64TypeInContext(self.context), offset, 0)};
            const field_ptr = c.LLVMBuildGEP2(
                self.builder,
                c.LLVMInt8TypeInContext(self.context),
                data_byte_ptr,
                &gep_indices,
                1,
                "field_byte_ptr",
            );
            const typed_ptr = c.LLVMBuildBitCast(self.builder, field_ptr, c.LLVMPointerType(field_type, 0), "typed_ptr");
            _ = c.LLVMBuildStore(self.builder, val.value, typed_ptr);
            offset += 8; // each field takes 8 bytes
        }
    }

    const loaded = c.LLVMBuildLoad2(self.builder, info.llvm_type, alloca, "enum_val");
    return .{ .value = loaded, .type_tag = .{ .enum_type = resolved_name } };
}

pub fn findFieldIndex(_: *const CodeGen, info: CodeGen.StructTypeInfo, field_name: []const u8) ?usize {
    for (info.field_names, 0..) |name, i| {
        if (std.mem.eql(u8, name, field_name)) return i;
    }
    return null;
}
