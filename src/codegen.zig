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
    const_values: std.StringHashMap(NamedValue),
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
    struct_types: std.StringHashMap(StructTypeInfo),
    generic_struct_decls: std.StringHashMap(*const Ast.StructDecl),
    monomorphized_names: std.ArrayList([]const u8) = .{},
    enum_types: std.StringHashMap(EnumTypeInfo),
    generic_enum_decls: std.StringHashMap(*const Ast.EnumDecl),
    strlen_fn: c.LLVMValueRef,
    malloc_fn: c.LLVMValueRef,
    sprintf_fn: c.LLVMValueRef,

    const StructTypeInfo = struct {
        llvm_type: c.LLVMTypeRef,
        field_names: []const []const u8,
        field_types: []const TypeTag,
    };

    const EnumTypeInfo = struct {
        llvm_type: c.LLVMTypeRef,
        variant_names: []const []const u8,
        variant_data_types: []const []const TypeTag,
    };

    const LoopContext = struct {
        continue_bb: c.LLVMBasicBlockRef,
        break_bb: c.LLVMBasicBlockRef,
    };

    const NamedValue = struct {
        alloca: c.LLVMValueRef,
        is_mutable: bool,
        type_tag: TypeTag,
    };

    pub const TypeTag = union(enum) {
        i32_type,
        i64_type,
        f64_type,
        bool_type,
        void_type,
        string_type,
        struct_type: []const u8,
        enum_type: []const u8,
        array_type: ArrayTypeTag,
        slice_type: SliceTypeTag,
        tuple_type: TupleTypeTag,
        ptr_type: PtrTypeTag,
    };

    pub const PtrTypeTag = struct {
        pointee: *const TypeTag,
        is_mutable: bool,
    };

    pub const TupleTypeTag = struct {
        element_types: []const TypeTag,
        llvm_type: c.LLVMTypeRef,
    };

    pub const ArrayTypeTag = struct {
        element_type: *const TypeTag,
        count: u32,
    };

    pub const SliceTypeTag = struct {
        element_type: *const TypeTag,
    };

    pub fn init(allocator: std.mem.Allocator, module_name: [*:0]const u8, diagnostics: *Diagnostics, ast: *const Ast) CodeGen {
        const context = c.LLVMContextCreate();
        const module = c.LLVMModuleCreateWithNameInContext(module_name, context);
        const builder = c.LLVMCreateBuilderInContext(context);

        // Declare printf
        const printf_param_types = [_]c.LLVMTypeRef{c.LLVMPointerType(c.LLVMInt8TypeInContext(context), 0)};
        const printf_type = c.LLVMFunctionType(c.LLVMInt32TypeInContext(context), @constCast(&printf_param_types), 1, 1);
        const printf_fn = c.LLVMAddFunction(module, "printf", printf_type);

        // Declare strlen: i64 strlen(i8*)
        const strlen_param_types = [_]c.LLVMTypeRef{c.LLVMPointerType(c.LLVMInt8TypeInContext(context), 0)};
        const strlen_type = c.LLVMFunctionType(c.LLVMInt64TypeInContext(context), @constCast(&strlen_param_types), 1, 0);
        const strlen_fn = c.LLVMAddFunction(module, "strlen", strlen_type);

        // Declare malloc: i8* malloc(i64)
        const malloc_param_types = [_]c.LLVMTypeRef{c.LLVMInt64TypeInContext(context)};
        const malloc_type = c.LLVMFunctionType(c.LLVMPointerType(c.LLVMInt8TypeInContext(context), 0), @constCast(&malloc_param_types), 1, 0);
        const malloc_fn = c.LLVMAddFunction(module, "malloc", malloc_type);

        // Declare sprintf: i32 sprintf(i8*, i8*, ...) — variadic
        const sprintf_param_types = [_]c.LLVMTypeRef{ c.LLVMPointerType(c.LLVMInt8TypeInContext(context), 0), c.LLVMPointerType(c.LLVMInt8TypeInContext(context), 0) };
        const sprintf_type = c.LLVMFunctionType(c.LLVMInt32TypeInContext(context), @constCast(&sprintf_param_types), 2, 1);
        const sprintf_fn = c.LLVMAddFunction(module, "sprintf", sprintf_type);

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
            .const_values = std.StringHashMap(NamedValue).init(allocator),
            .struct_types = std.StringHashMap(StructTypeInfo).init(allocator),
            .generic_struct_decls = std.StringHashMap(*const Ast.StructDecl).init(allocator),
            .enum_types = std.StringHashMap(EnumTypeInfo).init(allocator),
            .generic_enum_decls = std.StringHashMap(*const Ast.EnumDecl).init(allocator),
            .diagnostics = diagnostics,
            .allocator = allocator,
            .ast = ast,
            .printf_fn = printf_fn,
            .fmt_int = fmt_int,
            .fmt_str = fmt_str,
            .fmt_float = fmt_float,
            .fmt_bool_true = fmt_bool_true,
            .fmt_bool_false = fmt_bool_false,
            .strlen_fn = strlen_fn,
            .malloc_fn = malloc_fn,
            .sprintf_fn = sprintf_fn,
        };
    }

    pub fn deinit(self: *CodeGen) void {
        c.LLVMDisposeBuilder(self.builder);
        c.LLVMDisposeModule(self.module);
        c.LLVMContextDispose(self.context);
        self.named_values.deinit();
        self.const_values.deinit();
        self.loop_stack.deinit(self.allocator);
        var it = self.struct_types.iterator();
        while (it.next()) |entry| {
            self.allocator.free(entry.value_ptr.field_names);
            self.allocator.free(entry.value_ptr.field_types);
        }
        self.struct_types.deinit();
        self.generic_struct_decls.deinit();
        for (self.monomorphized_names.items) |name| {
            self.allocator.free(name);
        }
        self.monomorphized_names.deinit(self.allocator);
        var eit = self.enum_types.iterator();
        while (eit.next()) |entry| {
            for (entry.value_ptr.variant_data_types) |dt| {
                self.allocator.free(dt);
            }
            self.allocator.free(entry.value_ptr.variant_names);
            self.allocator.free(entry.value_ptr.variant_data_types);
        }
        self.enum_types.deinit();
        self.generic_enum_decls.deinit();
    }

    pub fn generate(self: *CodeGen) GenError!void {
        // First pass: register all struct and enum types
        for (self.ast.program.decls.items) |decl| {
            switch (decl.decl) {
                .struct_decl => |sd| try self.genStructType(&sd),
                .enum_decl => |ed| try self.genEnumType(&ed),
                else => {},
            }
        }

        // Second pass: declare extern functions and generate constants
        for (self.ast.program.decls.items) |decl| {
            switch (decl.decl) {
                .extern_decl => |ed| try self.genExternDecl(&ed),
                .const_decl => |cd| try self.genConstDecl(&cd),
                else => {},
            }
        }

        // Third pass: generate functions and methods
        for (self.ast.program.decls.items) |decl| {
            switch (decl.decl) {
                .function => |fn_decl| try self.genFunction(&fn_decl),
                .struct_decl => {},
                .impl_decl => |impl_decl| try self.genImplDecl(&impl_decl),
                .enum_decl => {},
                .extern_decl => {},
                .const_decl => {},
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

    fn genStructType(self: *CodeGen, sd: *const Ast.StructDecl) GenError!void {
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
        var field_type_tags = self.allocator.alloc(TypeTag, field_count) catch return error.CodeGenError;

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

    fn monomorphizeStruct(self: *CodeGen, generic_name: []const u8, type_args: []const Ast.TypeExpr) GenError![]const u8 {
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
        var type_map = std.StringHashMap(TypeTag).init(self.allocator);
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
        var field_type_tags = self.allocator.alloc(TypeTag, field_count) catch return error.CodeGenError;

        for (sd.fields.items, 0..) |field, i| {
            const ft = self.resolveTypeExprWithSubst(field.type_expr, &type_map);
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

        return mangled_name;
    }

    fn monomorphizeEnum(self: *CodeGen, generic_name: []const u8, type_args: []const Ast.TypeExpr) GenError![]const u8 {
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
        var type_map = std.StringHashMap(TypeTag).init(self.allocator);
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
        var variant_data_types = self.allocator.alloc([]const TypeTag, variant_count) catch return error.CodeGenError;

        var max_payload_size: usize = 0;
        for (ed.variants.items, 0..) |variant, i| {
            variant_names[i] = variant.name;
            const dt_count = variant.data_types.items.len;
            var dts = self.allocator.alloc(TypeTag, dt_count) catch return error.CodeGenError;
            var size: usize = 0;
            for (variant.data_types.items, 0..) |dt, j| {
                dts[j] = self.resolveTypeExprWithSubst(dt, &type_map);
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

    fn typeExprName(_: *CodeGen, type_expr: Ast.TypeExpr) []const u8 {
        return switch (type_expr) {
            .builtin => |b| switch (b) {
                .i32_type, .int_type => "i32",
                .i64_type => "i64",
                .f64_type, .float_type => "f64",
                .f32_type => "f32",
                .bool_type => "bool",
                .string_type => "string",
                .void => "void",
                .u8_type => "u8",
                .u16_type => "u16",
                .u32_type => "u32",
                .u64_type => "u64",
                .u128_type => "u128",
                .i8_type => "i8",
                .i16_type => "i16",
                .i128_type => "i128",
                .usize_type => "usize",
                .isize_type => "isize",
                .never_type => "Never",
            },
            .named => |name| name,
            .generic_type => |gt| gt.name,
            .ptr_type => "ptr",
            else => "unknown",
        };
    }

    fn resolveTypeExprWithSubst(self: *CodeGen, type_expr: Ast.TypeExpr, type_map: *const std.StringHashMap(TypeTag)) TypeTag {
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

    fn genImplDecl(self: *CodeGen, impl_decl: *const Ast.ImplDecl) GenError!void {
        for (impl_decl.methods.items) |method| {
            try self.genMethod(impl_decl.type_name, &method);
        }
    }

    fn genMethod(self: *CodeGen, type_name: []const u8, method: *const Ast.MethodDecl) GenError!void {
        const return_type_tag = self.resolveTypeExpr(method.return_type);
        const llvm_return_type = self.typeTagToLLVM(return_type_tag);

        var param_types = std.ArrayList(c.LLVMTypeRef){};
        defer param_types.deinit(self.allocator);

        // If method has self, first param is the struct type
        if (method.has_self) {
            const self_type = TypeTag{ .struct_type = type_name };
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
            const self_type = TypeTag{ .struct_type = type_name };
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

        try self.genBlock(&method.body, false);

        const current_bb = c.LLVMGetInsertBlock(self.builder);
        if (c.LLVMGetBasicBlockTerminator(current_bb) == null) {
            if (isTypeTag(return_type_tag, .void_type)) {
                _ = c.LLVMBuildRetVoid(self.builder);
            }
        }
    }

    fn genEnumType(self: *CodeGen, ed: *const Ast.EnumDecl) GenError!void {
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
        var variant_data_types = self.allocator.alloc([]const TypeTag, variant_count) catch return error.CodeGenError;

        // Calculate max payload size
        var max_payload_size: usize = 0;
        for (ed.variants.items, 0..) |variant, i| {
            variant_names[i] = variant.name;
            const dt_count = variant.data_types.items.len;
            var dts = self.allocator.alloc(TypeTag, dt_count) catch return error.CodeGenError;
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

    fn ensureOptionType(self: *CodeGen, inner_tag: TypeTag) void {
        if (self.enum_types.contains("Option")) return;

        // Option has two variants: Some(T) and None
        var variant_names = self.allocator.alloc([]const u8, 2) catch return;
        variant_names[0] = "Some";
        variant_names[1] = "None";

        var variant_data_types = self.allocator.alloc([]const TypeTag, 2) catch return;

        // Some variant has one data field of the inner type
        var some_types = self.allocator.alloc(TypeTag, 1) catch return;
        some_types[0] = inner_tag;
        variant_data_types[0] = some_types;

        // None variant has no data
        var none_types = self.allocator.alloc(TypeTag, 0) catch return;
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

    fn ensureResultType(self: *CodeGen, ok_tag: TypeTag, err_tag: TypeTag) void {
        if (self.enum_types.contains("Result")) return;

        // Result has two variants: Ok(T) and Err(E)
        var variant_names = self.allocator.alloc([]const u8, 2) catch return;
        variant_names[0] = "Ok";
        variant_names[1] = "Err";

        var variant_data_types = self.allocator.alloc([]const TypeTag, 2) catch return;

        // Ok variant has one data field of the ok type
        var ok_types = self.allocator.alloc(TypeTag, 1) catch return;
        ok_types[0] = ok_tag;
        variant_data_types[0] = ok_types;

        // Err variant has one data field of the err type
        var err_types = self.allocator.alloc(TypeTag, 1) catch return;
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

    fn genEnumConstructor(self: *CodeGen, enum_name: []const u8, variant_name: []const u8, args: *const std.ArrayList(Ast.ExprIndex)) GenError!ExprResult {
        // Generate all arg values upfront (needed for type inference with generic enums)
        var arg_vals = std.ArrayList(ExprResult){};
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
                                    const dt_tag: @typeInfo(TypeTag).@"union".tag_type.? = dt;
                                    const arg_tag: @typeInfo(TypeTag).@"union".tag_type.? = arg_vals.items[di].type_tag;
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

    fn resolveTypeExpr(self: *CodeGen, type_expr: Ast.TypeExpr) TypeTag {
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
            .named => |name| {
                if (self.struct_types.contains(name)) {
                    return .{ .struct_type = name };
                }
                if (self.enum_types.contains(name)) {
                    return .{ .enum_type = name };
                }
                return .i32_type;
            },
            .array_type => |arr| {
                const elem_tag = self.resolveTypeExpr(arr.element_type.*);
                const elem_ptr = self.allocator.create(TypeTag) catch return .i32_type;
                elem_ptr.* = elem_tag;
                return .{ .array_type = .{ .element_type = elem_ptr, .count = arr.size } };
            },
            .slice_type => |sl| {
                const elem_tag = self.resolveTypeExpr(sl.element_type.*);
                const elem_ptr = self.allocator.create(TypeTag) catch return .i32_type;
                elem_ptr.* = elem_tag;
                return .{ .slice_type = .{ .element_type = elem_ptr } };
            },
            .option_type => |opt| {
                // Auto-register Option as an enum type for this inner type
                const inner_tag = self.resolveTypeExpr(opt.inner_type.*);
                self.ensureOptionType(inner_tag);
                return .{ .enum_type = "Option" };
            },
            .result_type => |res| {
                // Auto-register Result as an enum type for these inner types
                const ok_tag = self.resolveTypeExpr(res.ok_type.*);
                const err_tag = self.resolveTypeExpr(res.err_type.*);
                self.ensureResultType(ok_tag, err_tag);
                return .{ .enum_type = "Result" };
            },
            .tuple_type => |tt| {
                const count = tt.element_types.items.len;
                var elem_tags = self.allocator.alloc(TypeTag, count) catch return .i32_type;
                var elem_llvm_types = self.allocator.alloc(c.LLVMTypeRef, count) catch return .i32_type;
                defer self.allocator.free(elem_llvm_types);

                for (tt.element_types.items, 0..) |et, i| {
                    elem_tags[i] = self.resolveTypeExpr(et);
                    elem_llvm_types[i] = self.typeTagToLLVM(elem_tags[i]);
                }

                const llvm_type = c.LLVMStructTypeInContext(self.context, elem_llvm_types.ptr, @intCast(count), 0);
                return .{ .tuple_type = .{ .element_types = elem_tags, .llvm_type = llvm_type } };
            },
            .generic_type => |gt| {
                // Check if it's a generic enum first, then try generic struct
                if (self.generic_enum_decls.contains(gt.name)) {
                    const mangled_name = self.monomorphizeEnum(gt.name, gt.type_args.items) catch return .i32_type;
                    return .{ .enum_type = mangled_name };
                }
                // Monomorphize the generic struct with concrete type args
                const mangled_name = self.monomorphizeStruct(gt.name, gt.type_args.items) catch return .i32_type;
                return .{ .struct_type = mangled_name };
            },
            .ptr_type => |pt| {
                const pointee_tag = self.resolveTypeExpr(pt.pointee.*);
                const pointee_ptr = self.allocator.create(TypeTag) catch return .i32_type;
                pointee_ptr.* = pointee_tag;
                return .{ .ptr_type = .{ .pointee = pointee_ptr, .is_mutable = pt.is_mutable } };
            },
            .inferred => {
                // Inferred types should be resolved at the call site, not here
                return .i32_type;
            },
        };
    }

    fn genConstDecl(self: *CodeGen, cd: *const Ast.ConstDecl) GenError!void {
        const type_tag = self.resolveTypeExpr(cd.type_expr);
        const llvm_type = self.typeTagToLLVM(type_tag);

        const name_z = self.allocator.dupeZ(u8, cd.name) catch return error.CodeGenError;
        defer self.allocator.free(name_z);

        // Evaluate the constant initializer to an LLVM constant
        const const_val = self.evalConstExpr(cd.init_expr, type_tag) orelse {
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

    fn evalConstExpr(self: *CodeGen, expr_idx: Ast.ExprIndex, type_tag: TypeTag) ?c.LLVMValueRef {
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
            else => return null,
        }
    }

    fn genExternDecl(self: *CodeGen, ed: *const Ast.ExternDecl) GenError!void {
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

    fn genFunction(self: *CodeGen, fn_decl: *const Ast.FnDecl) GenError!void {
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

        try self.genBlock(&fn_decl.body, is_main);

        const current_bb = c.LLVMGetInsertBlock(self.builder);
        if (c.LLVMGetBasicBlockTerminator(current_bb) == null) {
            if (is_main) {
                _ = c.LLVMBuildRet(self.builder, c.LLVMConstInt(c.LLVMInt32TypeInContext(self.context), 0, 0));
            } else if (isTypeTag(return_type_tag, .void_type)) {
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
            .if_stmt => |if_s| try self.genIfStmt(if_s, is_main),
            .if_let_stmt => |il_s| try self.genIfLetStmt(il_s, is_main),
            .for_stmt => |for_s| try self.genForStmt(for_s, is_main),
            .expr_stmt => |expr_idx| {
                _ = try self.genExpr(expr_idx);
            },
            .match_stmt => |match_s| try self.genMatchStmt(match_s, is_main),
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
                try self.genBlock(block, is_main);
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

    fn genIfLetStmt(self: *CodeGen, il_stmt: Ast.IfLetStmt, is_main: bool) GenError!void {
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

                try self.genBlock(il_stmt.then_block, is_main);
                if (c.LLVMGetBasicBlockTerminator(c.LLVMGetInsertBlock(self.builder)) == null) {
                    _ = c.LLVMBuildBr(self.builder, merge_bb);
                }

                // Else block
                c.LLVMPositionBuilderAtEnd(self.builder, else_bb);
                if (il_stmt.else_block) |eb| {
                    try self.genBlock(eb, is_main);
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
            .identifier => {
                self.diagnostics.report(.@"error", 0, "if let requires enum pattern", .{});
                return error.CodeGenError;
            },
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

    fn genMatchStmt(self: *CodeGen, match_stmt: Ast.MatchStmt, is_main: bool) GenError!void {
        const match_val = try self.genExpr(match_stmt.expr);
        const function = c.LLVMGetBasicBlockParent(c.LLVMGetInsertBlock(self.builder));

        // Get enum info
        const enum_name = switch (match_val.type_tag) {
            .enum_type => |name| name,
            else => {
                self.diagnostics.report(.@"error", 0, "match requires enum type", .{});
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

        // Build switch
        const default_bb = c.LLVMAppendBasicBlockInContext(self.context, function, "match.default");
        const switch_inst = c.LLVMBuildSwitch(self.builder, tag_val, default_bb, @intCast(match_stmt.arms.items.len));

        // Default block just branches to merge
        c.LLVMPositionBuilderAtEnd(self.builder, default_bb);
        _ = c.LLVMBuildBr(self.builder, merge_bb);

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

                    try self.genBlock(arm.body, is_main);
                    if (c.LLVMGetBasicBlockTerminator(c.LLVMGetInsertBlock(self.builder)) == null) {
                        _ = c.LLVMBuildBr(self.builder, merge_bb);
                    }
                },
                .identifier => {
                    // Wildcard / catch-all pattern - not implemented yet
                    self.diagnostics.report(.@"error", 0, "identifier patterns in match not yet implemented", .{});
                    return error.CodeGenError;
                },
            }
        }

        c.LLVMPositionBuilderAtEnd(self.builder, merge_bb);
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
            .field_access => |fa| {
                return self.genFieldAccess(fa);
            },
            .struct_literal => |sl| {
                return self.genStructLiteral(sl);
            },
            .method_call => |mc| {
                return self.genMethodCall(mc);
            },
            .associated_call => |ac| {
                return self.genAssociatedCall(ac);
            },
            .enum_constructor => |ec| {
                return self.genEnumConstructor(ec.enum_name, ec.variant_name, &ec.args);
            },
            .array_literal => |al| {
                return self.genArrayLiteral(al);
            },
            .index_access => |ia| {
                return self.genIndexAccess(ia);
            },
            .slice_expr => |se| {
                return self.genSliceExpr(se);
            },
            .tuple_literal => |tl| {
                return self.genTupleLiteral(tl);
            },
            .sizeof_expr => |type_expr| {
                return self.genSizeOf(type_expr);
            },
            .await_expr => |inner_idx| {
                // Synchronous await: just load the handle variable (it already has the value)
                return self.genExpr(inner_idx);
            },
            .address_of => |operand_idx| {
                return self.genAddressOf(operand_idx);
            },
            .deref => |operand_idx| {
                return self.genDeref(operand_idx);
            },
        }
    }

    fn genAddressOf(self: *CodeGen, operand_idx: Ast.ExprIndex) GenError!ExprResult {
        // The operand should be an lvalue (a variable). Return its alloca pointer.
        const expr = self.ast.getExpr(operand_idx);
        switch (expr) {
            .identifier => |name| {
                const nv = self.named_values.get(name) orelse {
                    self.diagnostics.report(.@"error", 0, "undefined variable '{s}'", .{name});
                    return error.CodeGenError;
                };
                // Return the alloca as a pointer value
                const pointee_ptr = self.allocator.create(TypeTag) catch return error.CodeGenError;
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

    fn genDeref(self: *CodeGen, operand_idx: Ast.ExprIndex) GenError!ExprResult {
        // Evaluate the operand (should be a pointer)
        const ptr_val = try self.genExpr(operand_idx);
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

    fn genArrayLiteral(self: *CodeGen, al: Ast.ArrayLiteralExpr) GenError!ExprResult {
        if (al.elements.items.len == 0) {
            self.diagnostics.report(.@"error", 0, "empty array literal", .{});
            return error.CodeGenError;
        }

        // Generate first element to determine type
        const first = try self.genExpr(al.elements.items[0]);
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
            const elem_val = try self.genExpr(elem_idx);
            var gep_idx = [_]c.LLVMValueRef{
                c.LLVMConstInt(c.LLVMInt64TypeInContext(self.context), 0, 0),
                c.LLVMConstInt(c.LLVMInt64TypeInContext(self.context), @intCast(i), 0),
            };
            const elem_ptr = c.LLVMBuildGEP2(self.builder, array_llvm, alloca, &gep_idx, 2, "elem_ptr");
            _ = c.LLVMBuildStore(self.builder, elem_val.value, elem_ptr);
        }

        // Load the full array value
        const loaded = c.LLVMBuildLoad2(self.builder, array_llvm, alloca, "arr_val");

        const elem_ptr = self.allocator.create(TypeTag) catch return error.CodeGenError;
        elem_ptr.* = elem_type_tag;
        return .{ .value = loaded, .type_tag = .{ .array_type = .{ .element_type = elem_ptr, .count = count } } };
    }

    fn genTupleLiteral(self: *CodeGen, tl: Ast.TupleLiteralExpr) GenError!ExprResult {
        if (tl.elements.items.len < 2) {
            self.diagnostics.report(.@"error", 0, "tuple requires at least 2 elements", .{});
            return error.CodeGenError;
        }

        const count = tl.elements.items.len;

        // Generate all element values and collect types
        var elem_vals = self.allocator.alloc(ExprResult, count) catch return error.CodeGenError;
        defer self.allocator.free(elem_vals);
        var elem_tags = self.allocator.alloc(TypeTag, count) catch return error.CodeGenError;
        var elem_llvm_types = self.allocator.alloc(c.LLVMTypeRef, count) catch return error.CodeGenError;
        defer self.allocator.free(elem_llvm_types);

        for (tl.elements.items, 0..) |elem_idx, i| {
            elem_vals[i] = try self.genExpr(elem_idx);
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

    fn genIndexAccess(self: *CodeGen, ia: Ast.IndexAccessExpr) GenError!ExprResult {
        // We need the alloca (pointer) of the array or slice, not a loaded value
        const obj_expr = self.ast.getExpr(ia.object);
        switch (obj_expr) {
            .identifier => |name| {
                if (self.named_values.get(name)) |nv| {
                    switch (nv.type_tag) {
                        .array_type => |arr| {
                            const index_val = try self.genExpr(ia.index);
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
                            return self.genSliceIndexAccess(nv.alloca, sl, ia.index);
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

    fn genSliceIndexAccess(self: *CodeGen, slice_alloca: c.LLVMValueRef, sl: SliceTypeTag, index_expr_idx: Ast.ExprIndex) GenError!ExprResult {
        const slice_llvm = self.sliceLLVMType();
        // Extract pointer (field 0)
        const ptr_field = c.LLVMBuildStructGEP2(self.builder, slice_llvm, slice_alloca, 0, "slice_ptr_field");
        const data_ptr = c.LLVMBuildLoad2(self.builder, c.LLVMPointerType(c.LLVMInt8TypeInContext(self.context), 0), ptr_field, "slice_ptr");

        const index_val = try self.genExpr(index_expr_idx);
        const elem_llvm = self.typeTagToLLVM(sl.element_type.*);

        // GEP on the data pointer with element type
        var gep_idx = [_]c.LLVMValueRef{index_val.value};
        const elem_ptr = c.LLVMBuildGEP2(self.builder, elem_llvm, c.LLVMBuildBitCast(self.builder, data_ptr, c.LLVMPointerType(elem_llvm, 0), "typed_ptr"), &gep_idx, 1, "slice_elem_ptr");
        const loaded = c.LLVMBuildLoad2(self.builder, elem_llvm, elem_ptr, "slice_elem");
        return .{ .value = loaded, .type_tag = sl.element_type.* };
    }

    fn sliceLLVMType(self: *const CodeGen) c.LLVMTypeRef {
        var field_types_arr = [_]c.LLVMTypeRef{
            c.LLVMPointerType(c.LLVMInt8TypeInContext(self.context), 0),
            c.LLVMInt32TypeInContext(self.context),
        };
        return c.LLVMStructTypeInContext(self.context, &field_types_arr, 2, 0);
    }

    fn genSliceExpr(self: *CodeGen, se: Ast.SliceExprNode) GenError!ExprResult {
        const obj_expr = self.ast.getExpr(se.object);
        switch (obj_expr) {
            .identifier => |name| {
                if (self.named_values.get(name)) |nv| {
                    switch (nv.type_tag) {
                        .array_type => |arr| {
                            return self.genSliceFromArray(nv.alloca, arr, se);
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

    fn genSliceFromArray(self: *CodeGen, array_alloca: c.LLVMValueRef, arr: ArrayTypeTag, se: Ast.SliceExprNode) GenError!ExprResult {
        const elem_llvm = self.typeTagToLLVM(arr.element_type.*);
        const array_llvm = self.typeTagToLLVM(.{ .array_type = arr });

        // Compute start index
        const start_val = if (se.start) |start_idx|
            (try self.genExpr(start_idx)).value
        else
            c.LLVMConstInt(c.LLVMInt32TypeInContext(self.context), 0, 0);

        // Compute end index
        const end_val = if (se.end) |end_idx|
            (try self.genExpr(end_idx)).value
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
        const elem_ptr_tag = self.allocator.create(TypeTag) catch return error.CodeGenError;
        elem_ptr_tag.* = arr.element_type.*;
        _ = elem_llvm;

        return .{ .value = loaded, .type_tag = .{ .slice_type = .{ .element_type = elem_ptr_tag } } };
    }

    fn genFieldAccess(self: *CodeGen, fa: Ast.FieldAccessExpr) GenError!ExprResult {
        // Check if this is an enum constructor: EnumName.VariantName
        const obj_expr = self.ast.getExpr(fa.object);
        switch (obj_expr) {
            .identifier => |name| {
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
                const parent = try self.genFieldAccessPtr(fa.object);
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

    fn genFieldAccessPtr(self: *CodeGen, expr_idx: Ast.ExprIndex) GenError!ExprResult {
        const expr = self.ast.getExpr(expr_idx);
        switch (expr) {
            .identifier => |name| {
                if (self.named_values.get(name)) |nv| {
                    return .{ .value = nv.alloca, .type_tag = nv.type_tag };
                }
            },
            .field_access => |fa| {
                const parent = try self.genFieldAccessPtr(fa.object);
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

    fn genStructLiteral(self: *CodeGen, sl: Ast.StructLiteralExpr) GenError!ExprResult {
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
                const val = try self.genExpr(fi.value);
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

    fn genMethodCall(self: *CodeGen, mc: Ast.MethodCallExpr) GenError!ExprResult {
        // Check if this is an enum constructor: EnumName.VariantName(args)
        const mc_obj_expr = self.ast.getExpr(mc.object);
        switch (mc_obj_expr) {
            .identifier => |name| {
                if (self.enum_types.contains(name) or self.generic_enum_decls.contains(name)) {
                    return self.genEnumConstructor(name, mc.method, &mc.args);
                }
            },
            else => {},
        }

        // Get the object value
        const obj = try self.genExpr(mc.object);

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
            const result = c.LLVMBuildCall2(self.builder, fn_type, function, args.items.ptr, @intCast(args.items.len), "mcall");
            // Look up method return type from AST
            const ret_tag = self.lookupMethodReturnType(struct_name, mc.method);
            return .{ .value = result, .type_tag = ret_tag };
        }
    }

    fn genAssociatedCall(self: *CodeGen, ac: Ast.AssociatedCallExpr) GenError!ExprResult {
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
            const result = c.LLVMBuildCall2(self.builder, fn_type, function, args.items.ptr, @intCast(args.items.len), "acall");
            const ret_tag = self.lookupMethodReturnType(ac.type_name, ac.func_name);
            return .{ .value = result, .type_tag = ret_tag };
        }
    }

    fn lookupMethodReturnType(self: *CodeGen, type_name: []const u8, method_name: []const u8) TypeTag {
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
        return .i32_type;
    }

    fn genBinaryOp(self: *CodeGen, op: Ast.BinaryOp, lhs: ExprResult, rhs: ExprResult) GenError!ExprResult {
        // String concatenation: string + string
        if (op == .add and isTypeTag(lhs.type_tag, .string_type) and isTypeTag(rhs.type_tag, .string_type)) {
            return self.genStringConcat(lhs.value, rhs.value);
        }

        const is_float = isTypeTag(lhs.type_tag, .f64_type);
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
            .negate => if (isTypeTag(operand.type_tag, .f64_type))
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

        if (std.mem.eql(u8, name, "len")) {
            return self.genLenCall(call_expr);
        }

        if (std.mem.eql(u8, name, "range")) {
            self.diagnostics.report(.@"error", 0, "range() can only be used in for loops", .{});
            return error.CodeGenError;
        }

        // Type conversion calls: i32(expr), f64(expr), etc.
        if (self.getTypeConversion(name)) |target_type| {
            if (call_expr.args.items.len != 1) {
                self.diagnostics.report(.@"error", 0, "type conversion takes exactly 1 argument", .{});
                return error.CodeGenError;
            }
            const arg = try self.genExpr(call_expr.args.items[0]);
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
            // Look up return type from AST
            const ret_tag = self.lookupFunctionReturnType(name);
            return .{ .value = result, .type_tag = ret_tag };
        }
    }

    fn genLogCall(self: *CodeGen, call_expr: Ast.CallExpr) GenError!ExprResult {
        if (call_expr.args.items.len != 1) {
            self.diagnostics.report(.@"error", 0, "log() takes exactly 1 argument", .{});
            return error.CodeGenError;
        }

        const arg = try self.genExpr(call_expr.args.items[0]);
        const printf_type = c.LLVMGlobalGetValueType(self.printf_fn);

        if (isTypeTag(arg.type_tag, .i32_type) or isTypeTag(arg.type_tag, .i64_type)) {
            const args_arr = [_]c.LLVMValueRef{ self.fmt_int, arg.value };
            _ = c.LLVMBuildCall2(self.builder, printf_type, self.printf_fn, @constCast(&args_arr), 2, "");
        } else if (isTypeTag(arg.type_tag, .f64_type)) {
            const args_arr = [_]c.LLVMValueRef{ self.fmt_float, arg.value };
            _ = c.LLVMBuildCall2(self.builder, printf_type, self.printf_fn, @constCast(&args_arr), 2, "");
        } else if (isTypeTag(arg.type_tag, .string_type)) {
            const args_arr = [_]c.LLVMValueRef{ self.fmt_str, arg.value };
            _ = c.LLVMBuildCall2(self.builder, printf_type, self.printf_fn, @constCast(&args_arr), 2, "");
        } else if (isTypeTag(arg.type_tag, .bool_type)) {
            const fmt = c.LLVMBuildSelect(self.builder, arg.value, self.fmt_bool_true, self.fmt_bool_false, "bool_fmt");
            const args_arr = [_]c.LLVMValueRef{fmt};
            _ = c.LLVMBuildCall2(self.builder, printf_type, self.printf_fn, @constCast(&args_arr), 1, "");
        } else if (isTypeTag(arg.type_tag, .void_type)) {
            self.diagnostics.report(.@"error", 0, "cannot log void value", .{});
            return error.CodeGenError;
        } else {
            self.diagnostics.report(.@"error", 0, "cannot log this type", .{});
            return error.CodeGenError;
        }

        return .{ .value = c.LLVMGetUndef(c.LLVMVoidTypeInContext(self.context)), .type_tag = .void_type };
    }

    fn genLenCall(self: *CodeGen, call_expr: Ast.CallExpr) GenError!ExprResult {
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
                            return self.genStrlen(str_val);
                        },
                        else => {},
                    }
                }
            },
            else => {
                // For non-identifier expressions (e.g. function calls), generate the expression
                const arg = try self.genExpr(call_expr.args.items[0]);
                if (isTypeTag(arg.type_tag, .string_type)) {
                    return self.genStrlen(arg.value);
                }
            },
        }

        self.diagnostics.report(.@"error", 0, "len() requires an array, slice, or string argument", .{});
        return error.CodeGenError;
    }

    fn genStrlen(self: *CodeGen, str_val: c.LLVMValueRef) GenError!ExprResult {
        const strlen_type = c.LLVMGlobalGetValueType(self.strlen_fn);
        const args_arr = [_]c.LLVMValueRef{str_val};
        const len_i64 = c.LLVMBuildCall2(self.builder, strlen_type, self.strlen_fn, @constCast(&args_arr), 1, "strlen");
        // Truncate i64 to i32 (strlen returns size_t which is i64)
        const len_i32 = c.LLVMBuildTrunc(self.builder, len_i64, c.LLVMInt32TypeInContext(self.context), "len_i32");
        return .{ .value = len_i32, .type_tag = .i32_type };
    }

    fn genStringConcat(self: *CodeGen, lhs_val: c.LLVMValueRef, rhs_val: c.LLVMValueRef) GenError!ExprResult {
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

    fn genSizeOf(self: *CodeGen, type_expr: Ast.TypeExpr) GenError!ExprResult {
        const type_tag = self.resolveTypeExpr(type_expr);
        const llvm_type = self.typeTagToLLVM(type_tag);

        // Use LLVMSizeOf which returns a constant i64, then truncate to i32
        const size_val = c.LLVMSizeOf(llvm_type);
        const truncated = c.LLVMConstTrunc(size_val, c.LLVMInt32TypeInContext(self.context));
        return .{ .value = truncated, .type_tag = .i32_type };
    }

    fn lookupFunctionReturnType(self: *CodeGen, name: []const u8) TypeTag {
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

    fn isTypeTag(tag: TypeTag, comptime expected: @typeInfo(TypeTag).@"union".tag_type.?) bool {
        return @as(@typeInfo(TypeTag).@"union".tag_type.?, tag) == expected;
    }

    fn findFieldIndex(_: *const CodeGen, info: StructTypeInfo, field_name: []const u8) ?usize {
        for (info.field_names, 0..) |name, i| {
            if (std.mem.eql(u8, name, field_name)) return i;
        }
        return null;
    }

    fn getTypeConversion(_: *const CodeGen, name: []const u8) ?TypeTag {
        if (std.mem.eql(u8, name, "i32")) return .i32_type;
        if (std.mem.eql(u8, name, "i64")) return .i64_type;
        if (std.mem.eql(u8, name, "f64")) return .f64_type;
        if (std.mem.eql(u8, name, "bool")) return .bool_type;
        return null;
    }

    fn genTypeConversion(self: *CodeGen, src: ExprResult, target: TypeTag) GenError!ExprResult {
        const src_tag: @typeInfo(TypeTag).@"union".tag_type.? = src.type_tag;
        const tgt_tag: @typeInfo(TypeTag).@"union".tag_type.? = target;
        if (src_tag == tgt_tag) return src;

        const value: c.LLVMValueRef = if (isTypeTag(target, .f64_type)) blk: {
            if (isTypeTag(src.type_tag, .i32_type) or isTypeTag(src.type_tag, .i64_type)) {
                break :blk c.LLVMBuildSIToFP(self.builder, src.value, c.LLVMDoubleTypeInContext(self.context), "sitofp");
            }
            self.diagnostics.report(.@"error", 0, "cannot convert to f64", .{});
            return error.CodeGenError;
        } else if (isTypeTag(target, .i32_type)) blk: {
            if (isTypeTag(src.type_tag, .f64_type)) {
                break :blk c.LLVMBuildFPToSI(self.builder, src.value, c.LLVMInt32TypeInContext(self.context), "fptosi");
            }
            if (isTypeTag(src.type_tag, .i64_type)) {
                break :blk c.LLVMBuildTrunc(self.builder, src.value, c.LLVMInt32TypeInContext(self.context), "trunc");
            }
            self.diagnostics.report(.@"error", 0, "cannot convert to i32", .{});
            return error.CodeGenError;
        } else if (isTypeTag(target, .i64_type)) blk: {
            if (isTypeTag(src.type_tag, .f64_type)) {
                break :blk c.LLVMBuildFPToSI(self.builder, src.value, c.LLVMInt64TypeInContext(self.context), "fptosi");
            }
            if (isTypeTag(src.type_tag, .i32_type)) {
                break :blk c.LLVMBuildSExt(self.builder, src.value, c.LLVMInt64TypeInContext(self.context), "sext");
            }
            self.diagnostics.report(.@"error", 0, "cannot convert to i64", .{});
            return error.CodeGenError;
        } else {
            self.diagnostics.report(.@"error", 0, "unsupported type conversion", .{});
            return error.CodeGenError;
        };

        return .{ .value = value, .type_tag = target };
    }

    fn typeTagToLLVM(self: *const CodeGen, tag: TypeTag) c.LLVMTypeRef {
        return switch (tag) {
            .i32_type => c.LLVMInt32TypeInContext(self.context),
            .i64_type => c.LLVMInt64TypeInContext(self.context),
            .f64_type => c.LLVMDoubleTypeInContext(self.context),
            .bool_type => c.LLVMInt1TypeInContext(self.context),
            .void_type => c.LLVMVoidTypeInContext(self.context),
            .string_type => c.LLVMPointerType(c.LLVMInt8TypeInContext(self.context), 0),
            .struct_type => |name| {
                if (self.struct_types.get(name)) |info| {
                    return info.llvm_type;
                }
                return c.LLVMInt32TypeInContext(self.context);
            },
            .enum_type => |name| {
                if (self.enum_types.get(name)) |info| {
                    return info.llvm_type;
                }
                return c.LLVMInt32TypeInContext(self.context);
            },
            .array_type => |arr| {
                const elem_llvm = self.typeTagToLLVM(arr.element_type.*);
                return c.LLVMArrayType(elem_llvm, arr.count);
            },
            .slice_type => {
                // Slice is { i8*, i32 } — pointer + length
                var field_types_arr = [_]c.LLVMTypeRef{
                    c.LLVMPointerType(c.LLVMInt8TypeInContext(self.context), 0),
                    c.LLVMInt32TypeInContext(self.context),
                };
                return c.LLVMStructTypeInContext(self.context, &field_types_arr, 2, 0);
            },
            .tuple_type => |tt| {
                return tt.llvm_type;
            },
            .ptr_type => |pt| {
                const pointee_llvm = self.typeTagToLLVM(pt.pointee.*);
                return c.LLVMPointerType(pointee_llvm, 0);
            },
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
        .array_type => .i32_type, // fallback - real resolution happens in CodeGen
        .slice_type => .i32_type, // fallback - real resolution happens in CodeGen
        .option_type => .{ .enum_type = "Option" }, // resolved properly in CodeGen
        .result_type => .{ .enum_type = "Result" }, // resolved properly in CodeGen
        .tuple_type => .i32_type, // resolved properly in CodeGen
        .generic_type => .i32_type, // resolved properly in CodeGen via monomorphization
        .ptr_type => .i32_type, // resolved properly in CodeGen
        .inferred => .i32_type, // resolved at call site via expression type
    };
}
