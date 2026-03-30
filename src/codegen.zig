const std = @import("std");
const Ast = @import("ast.zig").Ast;
const Diagnostics = @import("diagnostics.zig").Diagnostics;
const codegen_types = @import("codegen_types.zig");
const codegen_stmts = @import("codegen_stmts.zig");
const codegen_exprs = @import("codegen_exprs.zig");

pub const c = @cImport({
    @cInclude("llvm-c/Core.h");
    @cInclude("llvm-c/Analysis.h");
    @cInclude("llvm-c/Target.h");
    @cInclude("llvm-c/TargetMachine.h");
    @cInclude("llvm-c/DebugInfo.h");
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
    type_tag_arena: std.heap.ArenaAllocator,
    ast: *const Ast,
    printf_fn: c.LLVMValueRef,
    fmt_int: c.LLVMValueRef,
    fmt_uint: c.LLVMValueRef,
    fmt_long: c.LLVMValueRef,
    fmt_ulong: c.LLVMValueRef,
    fmt_str: c.LLVMValueRef,
    fmt_float: c.LLVMValueRef,
    fmt_char: c.LLVMValueRef,
    fmt_bool_true: c.LLVMValueRef,
    fmt_bool_false: c.LLVMValueRef,
    loop_stack: std.ArrayList(LoopContext) = .{},
    struct_types: std.StringHashMap(StructTypeInfo),
    generic_struct_decls: std.StringHashMap(*const Ast.StructDecl),
    monomorphized_names: std.ArrayList([]const u8) = .{},
    monomorphized_origins: std.StringHashMap(MonomorphizedOrigin),
    generic_impl_decls: std.StringHashMap(*const Ast.ImplDecl),
    enum_types: std.StringHashMap(EnumTypeInfo),
    generic_enum_decls: std.StringHashMap(*const Ast.EnumDecl),
    strlen_fn: c.LLVMValueRef,
    malloc_fn: c.LLVMValueRef,
    sprintf_fn: c.LLVMValueRef,
    exit_fn: c.LLVMValueRef,
    fmt_panic: c.LLVMValueRef,
    imported_fn_return_types: std.StringHashMap(TypeTag),
    type_aliases: std.StringHashMap(Ast.TypeExpr),
    closure_counter: u32 = 0,
    // Generic function templates
    generic_fn_decls: std.StringHashMap(*const Ast.FnDecl),
    // Trait vtable infrastructure
    trait_decls: std.StringHashMap(*const Ast.TraitDecl),
    vtable_globals: std.StringHashMap(c.LLVMValueRef), // "TraitName_TypeName" -> global vtable
    // Global variables for argc/argv (set in main entry)
    argc_global: c.LLVMValueRef,
    argv_global: c.LLVMValueRef,
    // Debug info
    debug_enabled: bool = false,
    di_builder: ?c.LLVMDIBuilderRef = null,
    di_file: ?c.LLVMMetadataRef = null,
    di_compile_unit: ?c.LLVMMetadataRef = null,
    source: []const u8 = "",
    source_path: []const u8 = "",
    // Stdlib C function refs
    sqrt_fn: c.LLVMValueRef,
    pow_fn: c.LLVMValueRef,
    fabs_fn: c.LLVMValueRef,
    floor_fn: c.LLVMValueRef,
    ceil_fn: c.LLVMValueRef,
    fmin_fn: c.LLVMValueRef,
    fmax_fn: c.LLVMValueRef,
    sin_fn: c.LLVMValueRef,
    cos_fn: c.LLVMValueRef,
    tan_fn: c.LLVMValueRef,
    math_log_fn: c.LLVMValueRef,
    log10_fn: c.LLVMValueRef,
    exp_fn: c.LLVMValueRef,
    round_fn: c.LLVMValueRef,
    toupper_fn: c.LLVMValueRef,
    tolower_fn: c.LLVMValueRef,
    strstr_fn: c.LLVMValueRef,
    strncmp_fn: c.LLVMValueRef,
    memcpy_fn: c.LLVMValueRef,
    atoi_fn: c.LLVMValueRef,
    atof_fn: c.LLVMValueRef,
    puts_fn: c.LLVMValueRef,
    getenv_fn: c.LLVMValueRef,
    clock_fn: c.LLVMValueRef,
    isspace_fn: c.LLVMValueRef,
    // Stdlib format strings (no newline)
    fmt_int_raw: c.LLVMValueRef,
    fmt_uint_raw: c.LLVMValueRef,
    fmt_long_raw: c.LLVMValueRef,
    fmt_ulong_raw: c.LLVMValueRef,
    fmt_str_raw: c.LLVMValueRef,
    fmt_float_raw: c.LLVMValueRef,
    fmt_char_raw: c.LLVMValueRef,

    pub const StructTypeInfo = struct {
        llvm_type: c.LLVMTypeRef,
        field_names: []const []const u8,
        field_types: []const TypeTag,
    };

    pub const EnumTypeInfo = struct {
        llvm_type: c.LLVMTypeRef,
        variant_names: []const []const u8,
        variant_data_types: []const []const TypeTag,
    };

    const MonomorphizedOrigin = struct {
        generic_name: []const u8,
        type_args: []const Ast.TypeExpr,
    };

    const LoopContext = struct {
        continue_bb: c.LLVMBasicBlockRef,
        break_bb: c.LLVMBasicBlockRef,
    };

    pub const NamedValue = struct {
        alloca: c.LLVMValueRef,
        is_mutable: bool,
        type_tag: TypeTag,
    };

    pub const TypeTag = union(enum) {
        i8_type,
        i16_type,
        i32_type,
        i64_type,
        i128_type,
        u8_type,
        u16_type,
        u32_type,
        u64_type,
        u128_type,
        f64_type,
        bool_type,
        char_type,
        void_type,
        string_type,
        struct_type: []const u8,
        enum_type: []const u8,
        array_type: ArrayTypeTag,
        slice_type: SliceTypeTag,
        tuple_type: TupleTypeTag,
        ptr_type: PtrTypeTag,
        closure_type: ClosureTypeTag,
        fn_type: FnTypeTag,
        trait_type: []const u8,
    };

    pub const FnTypeTag = struct {
        param_types: []const TypeTag,
        return_type: *const TypeTag,
    };

    pub const ClosureTypeTag = struct {
        fn_name: []const u8,
        env_alloca: c.LLVMValueRef,
        return_type: *const TypeTag,
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

    pub const ExprResult = struct {
        value: c.LLVMValueRef,
        type_tag: TypeTag,
    };

    pub const RangeInfo = struct {
        start: c.LLVMValueRef,
        end: c.LLVMValueRef,
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

        // Declare exit: void exit(i32) — noreturn
        const exit_param_types = [_]c.LLVMTypeRef{c.LLVMInt32TypeInContext(context)};
        const exit_type = c.LLVMFunctionType(c.LLVMVoidTypeInContext(context), @constCast(&exit_param_types), 1, 0);
        const exit_fn = c.LLVMAddFunction(module, "exit", exit_type);

        // Declare stdlib C functions
        const double_type = c.LLVMDoubleTypeInContext(context);
        const i32_type_llvm = c.LLVMInt32TypeInContext(context);
        const i64_type_llvm = c.LLVMInt64TypeInContext(context);
        const i8ptr_type = c.LLVMPointerType(c.LLVMInt8TypeInContext(context), 0);

        // Math functions: f64 -> f64
        const math_f64_f64_params = [_]c.LLVMTypeRef{double_type};
        const math_f64_f64_type = c.LLVMFunctionType(double_type, @constCast(&math_f64_f64_params), 1, 0);
        const sqrt_fn = c.LLVMAddFunction(module, "sqrt", math_f64_f64_type);
        const fabs_fn = c.LLVMAddFunction(module, "fabs", math_f64_f64_type);
        const floor_fn = c.LLVMAddFunction(module, "floor", math_f64_f64_type);
        const ceil_fn = c.LLVMAddFunction(module, "ceil", math_f64_f64_type);
        const sin_fn = c.LLVMAddFunction(module, "sin", math_f64_f64_type);
        const cos_fn = c.LLVMAddFunction(module, "cos", math_f64_f64_type);
        const tan_fn = c.LLVMAddFunction(module, "tan", math_f64_f64_type);
        const math_log_fn = c.LLVMAddFunction(module, "log", math_f64_f64_type);
        const log10_fn = c.LLVMAddFunction(module, "log10", math_f64_f64_type);
        const exp_fn = c.LLVMAddFunction(module, "exp", math_f64_f64_type);
        const round_fn = c.LLVMAddFunction(module, "round", math_f64_f64_type);

        // Math functions: (f64, f64) -> f64
        const math_f64_f64_f64_params = [_]c.LLVMTypeRef{ double_type, double_type };
        const math_f64_f64_f64_type = c.LLVMFunctionType(double_type, @constCast(&math_f64_f64_f64_params), 2, 0);
        const pow_fn = c.LLVMAddFunction(module, "pow", math_f64_f64_f64_type);
        const fmin_fn = c.LLVMAddFunction(module, "fmin", math_f64_f64_f64_type);
        const fmax_fn = c.LLVMAddFunction(module, "fmax", math_f64_f64_f64_type);

        // String C functions: toupper/tolower: i32 -> i32
        const i32_i32_params = [_]c.LLVMTypeRef{i32_type_llvm};
        const i32_i32_type = c.LLVMFunctionType(i32_type_llvm, @constCast(&i32_i32_params), 1, 0);
        const toupper_fn = c.LLVMAddFunction(module, "toupper", i32_i32_type);
        const tolower_fn = c.LLVMAddFunction(module, "tolower", i32_i32_type);
        const isspace_fn = c.LLVMAddFunction(module, "isspace", i32_i32_type);

        // strstr: (i8*, i8*) -> i8*
        const two_str_params = [_]c.LLVMTypeRef{ i8ptr_type, i8ptr_type };
        const strstr_type = c.LLVMFunctionType(i8ptr_type, @constCast(&two_str_params), 2, 0);
        const strstr_fn = c.LLVMAddFunction(module, "strstr", strstr_type);

        // strncmp: (i8*, i8*, i64) -> i32
        const strncmp_params = [_]c.LLVMTypeRef{ i8ptr_type, i8ptr_type, i64_type_llvm };
        const strncmp_type = c.LLVMFunctionType(i32_type_llvm, @constCast(&strncmp_params), 3, 0);
        const strncmp_fn = c.LLVMAddFunction(module, "strncmp", strncmp_type);

        // memcpy: (i8*, i8*, i64) -> i8*
        const memcpy_params = [_]c.LLVMTypeRef{ i8ptr_type, i8ptr_type, i64_type_llvm };
        const memcpy_type = c.LLVMFunctionType(i8ptr_type, @constCast(&memcpy_params), 3, 0);
        const memcpy_fn = c.LLVMAddFunction(module, "memcpy", memcpy_type);

        // atoi: (i8*) -> i32
        const atoi_params = [_]c.LLVMTypeRef{i8ptr_type};
        const atoi_type = c.LLVMFunctionType(i32_type_llvm, @constCast(&atoi_params), 1, 0);
        const atoi_fn = c.LLVMAddFunction(module, "atoi", atoi_type);

        // atof: (i8*) -> f64
        const atof_type = c.LLVMFunctionType(double_type, @constCast(&atoi_params), 1, 0);
        const atof_fn = c.LLVMAddFunction(module, "atof", atof_type);

        // puts: (i8*) -> i32
        const puts_type = c.LLVMFunctionType(i32_type_llvm, @constCast(&atoi_params), 1, 0);
        const puts_fn = c.LLVMAddFunction(module, "puts", puts_type);

        // getenv: (i8*) -> i8*
        const getenv_type = c.LLVMFunctionType(i8ptr_type, @constCast(&atoi_params), 1, 0);
        const getenv_fn = c.LLVMAddFunction(module, "getenv", getenv_type);

        // clock: () -> i64
        const clock_type = c.LLVMFunctionType(i64_type_llvm, null, 0, 0);
        const clock_fn = c.LLVMAddFunction(module, "clock", clock_type);

        // Global variables for argc/argv
        const argc_global = c.LLVMAddGlobal(module, i32_type_llvm, "__asthra_argc");
        c.LLVMSetInitializer(argc_global, c.LLVMConstInt(i32_type_llvm, 0, 0));
        const i8ptr_ptr_type = c.LLVMPointerType(i8ptr_type, 0);
        const argv_global = c.LLVMAddGlobal(module, i8ptr_ptr_type, "__asthra_argv");
        c.LLVMSetInitializer(argv_global, c.LLVMConstNull(i8ptr_ptr_type));

        // Create format strings — need a temp function to position the builder
        const init_fn_type = c.LLVMFunctionType(c.LLVMVoidTypeInContext(context), null, 0, 0);
        const init_fn = c.LLVMAddFunction(module, "__asthra_init_strings", init_fn_type);
        const init_bb = c.LLVMAppendBasicBlockInContext(context, init_fn, "entry");
        c.LLVMPositionBuilderAtEnd(builder, init_bb);

        const fmt_int = c.LLVMBuildGlobalStringPtr(builder, "%d\n", "fmt_int");
        const fmt_uint = c.LLVMBuildGlobalStringPtr(builder, "%u\n", "fmt_uint");
        const fmt_long = c.LLVMBuildGlobalStringPtr(builder, "%ld\n", "fmt_long");
        const fmt_ulong = c.LLVMBuildGlobalStringPtr(builder, "%lu\n", "fmt_ulong");
        const fmt_str = c.LLVMBuildGlobalStringPtr(builder, "%s\n", "fmt_str");
        const fmt_float = c.LLVMBuildGlobalStringPtr(builder, "%f\n", "fmt_float");
        const fmt_char = c.LLVMBuildGlobalStringPtr(builder, "%c\n", "fmt_char");
        const fmt_bool_true = c.LLVMBuildGlobalStringPtr(builder, "true\n", "fmt_bool_true");
        const fmt_bool_false = c.LLVMBuildGlobalStringPtr(builder, "false\n", "fmt_bool_false");
        const fmt_panic = c.LLVMBuildGlobalStringPtr(builder, "panic: %s\n", "fmt_panic");
        const fmt_int_raw = c.LLVMBuildGlobalStringPtr(builder, "%d", "fmt_int_raw");
        const fmt_uint_raw = c.LLVMBuildGlobalStringPtr(builder, "%u", "fmt_uint_raw");
        const fmt_long_raw = c.LLVMBuildGlobalStringPtr(builder, "%ld", "fmt_long_raw");
        const fmt_ulong_raw = c.LLVMBuildGlobalStringPtr(builder, "%lu", "fmt_ulong_raw");
        const fmt_str_raw = c.LLVMBuildGlobalStringPtr(builder, "%s", "fmt_str_raw");
        const fmt_float_raw = c.LLVMBuildGlobalStringPtr(builder, "%f", "fmt_float_raw");
        const fmt_char_raw = c.LLVMBuildGlobalStringPtr(builder, "%c", "fmt_char_raw");

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
            .monomorphized_origins = std.StringHashMap(MonomorphizedOrigin).init(allocator),
            .generic_impl_decls = std.StringHashMap(*const Ast.ImplDecl).init(allocator),
            .enum_types = std.StringHashMap(EnumTypeInfo).init(allocator),
            .generic_enum_decls = std.StringHashMap(*const Ast.EnumDecl).init(allocator),
            .diagnostics = diagnostics,
            .allocator = allocator,
            .type_tag_arena = std.heap.ArenaAllocator.init(allocator),
            .ast = ast,
            .printf_fn = printf_fn,
            .fmt_int = fmt_int,
            .fmt_uint = fmt_uint,
            .fmt_long = fmt_long,
            .fmt_ulong = fmt_ulong,
            .fmt_str = fmt_str,
            .fmt_float = fmt_float,
            .fmt_char = fmt_char,
            .fmt_bool_true = fmt_bool_true,
            .fmt_bool_false = fmt_bool_false,
            .strlen_fn = strlen_fn,
            .malloc_fn = malloc_fn,
            .sprintf_fn = sprintf_fn,
            .exit_fn = exit_fn,
            .fmt_panic = fmt_panic,
            .imported_fn_return_types = std.StringHashMap(TypeTag).init(allocator),
            .type_aliases = std.StringHashMap(Ast.TypeExpr).init(allocator),
            .sqrt_fn = sqrt_fn,
            .pow_fn = pow_fn,
            .fabs_fn = fabs_fn,
            .floor_fn = floor_fn,
            .ceil_fn = ceil_fn,
            .fmin_fn = fmin_fn,
            .fmax_fn = fmax_fn,
            .sin_fn = sin_fn,
            .cos_fn = cos_fn,
            .tan_fn = tan_fn,
            .math_log_fn = math_log_fn,
            .log10_fn = log10_fn,
            .exp_fn = exp_fn,
            .round_fn = round_fn,
            .toupper_fn = toupper_fn,
            .tolower_fn = tolower_fn,
            .strstr_fn = strstr_fn,
            .strncmp_fn = strncmp_fn,
            .memcpy_fn = memcpy_fn,
            .atoi_fn = atoi_fn,
            .atof_fn = atof_fn,
            .puts_fn = puts_fn,
            .getenv_fn = getenv_fn,
            .clock_fn = clock_fn,
            .isspace_fn = isspace_fn,
            .fmt_int_raw = fmt_int_raw,
            .fmt_uint_raw = fmt_uint_raw,
            .fmt_long_raw = fmt_long_raw,
            .fmt_ulong_raw = fmt_ulong_raw,
            .fmt_str_raw = fmt_str_raw,
            .fmt_float_raw = fmt_float_raw,
            .fmt_char_raw = fmt_char_raw,
            .argc_global = argc_global,
            .argv_global = argv_global,
            .generic_fn_decls = std.StringHashMap(*const Ast.FnDecl).init(allocator),
            .trait_decls = std.StringHashMap(*const Ast.TraitDecl).init(allocator),
            .vtable_globals = std.StringHashMap(c.LLVMValueRef).init(allocator),
        };
    }

    pub fn initDebug(self: *CodeGen, source: []const u8, source_path: []const u8) void {
        self.debug_enabled = true;
        self.source = source;
        self.source_path = source_path;

        self.di_builder = c.LLVMCreateDIBuilder(self.module);

        // Extract filename and directory from source_path
        const filename = std.fs.path.basename(source_path);
        const directory = std.fs.path.dirname(source_path) orelse ".";

        self.di_file = c.LLVMDIBuilderCreateFile(
            self.di_builder.?,
            filename.ptr,
            filename.len,
            directory.ptr,
            directory.len,
        );

        self.di_compile_unit = c.LLVMDIBuilderCreateCompileUnit(
            self.di_builder.?,
            c.LLVMDWARFSourceLanguageC, // DW_LANG_C (closest to Asthra)
            self.di_file.?,
            "asthra",
            6, // producer name length
            0, // isOptimized
            "", // flags
            0, // flags length
            0, // runtime version
            null, // split name
            0, // split name length
            c.LLVMDWARFEmissionFull,
            0, // DWOId
            0, // splitDebugInlining
            0, // debugInfoForProfiling
            "", // sysroot
            0, // sysroot length
            "", // SDK
            0, // SDK length
        );

        // Add module flags for debug info
        c.LLVMAddModuleFlag(
            self.module,
            c.LLVMModuleFlagBehaviorWarning,
            "Debug Info Version",
            18, // length
            c.LLVMValueAsMetadata(c.LLVMConstInt(c.LLVMInt32TypeInContext(self.context), 3, 0)),
        );
        c.LLVMAddModuleFlag(
            self.module,
            c.LLVMModuleFlagBehaviorWarning,
            "Dwarf Version",
            13, // length
            c.LLVMValueAsMetadata(c.LLVMConstInt(c.LLVMInt32TypeInContext(self.context), 4, 0)),
        );
    }

    pub fn getLineFromOffset(self: *const CodeGen, byte_offset: u32) u32 {
        var line: u32 = 1;
        const end = @min(byte_offset, @as(u32, @intCast(self.source.len)));
        for (self.source[0..end]) |ch| {
            if (ch == '\n') line += 1;
        }
        return line;
    }

    pub fn setDebugLocation(self: *CodeGen, line: u32) void {
        if (!self.debug_enabled) return;
        if (self.di_builder == null) return;
        // Get current function's DISubprogram as scope
        const current_fn = c.LLVMGetBasicBlockParent(c.LLVMGetInsertBlock(self.builder));
        const subprogram = c.LLVMGetSubprogram(current_fn);
        if (subprogram == null) return;
        const loc = c.LLVMDIBuilderCreateDebugLocation(
            self.context,
            line,
            0, // column
            subprogram,
            null, // inlined at
        );
        c.LLVMSetCurrentDebugLocation2(self.builder, loc);
    }

    pub fn isStdlibNamespace(name: []const u8) bool {
        return std.mem.eql(u8, name, "math") or
            std.mem.eql(u8, name, "str") or
            std.mem.eql(u8, name, "io") or
            std.mem.eql(u8, name, "os");
    }

    pub fn deinit(self: *CodeGen) void {
        self.type_tag_arena.deinit();
        c.LLVMDisposeBuilder(self.builder);
        c.LLVMDisposeModule(self.module);
        c.LLVMContextDispose(self.context);
        self.named_values.deinit();
        self.const_values.deinit();
        self.loop_stack.deinit(self.allocator);
        var it = self.struct_types.iterator();
        while (it.next()) |entry| {
            self.allocator.free(entry.value_ptr.field_names);
        }
        self.struct_types.deinit();
        self.generic_struct_decls.deinit();
        self.monomorphized_origins.deinit();
        self.generic_impl_decls.deinit();
        for (self.monomorphized_names.items) |name| {
            self.allocator.free(name);
        }
        self.monomorphized_names.deinit(self.allocator);
        var eit = self.enum_types.iterator();
        while (eit.next()) |entry| {
            self.allocator.free(entry.value_ptr.variant_names);
        }
        self.enum_types.deinit();
        self.generic_enum_decls.deinit();
        self.imported_fn_return_types.deinit();
        self.type_aliases.deinit();
        self.generic_fn_decls.deinit();
        self.trait_decls.deinit();
        self.vtable_globals.deinit();
    }

    pub fn generate(self: *CodeGen) GenError!void {
        // Pre-pass: register type aliases
        for (self.ast.program.decls.items) |decl| {
            switch (decl.decl) {
                .type_alias => |ta| {
                    self.type_aliases.put(ta.name, ta.target) catch {};
                },
                else => {},
            }
        }

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

        // Third pass: register traits and generate impl methods (before functions that use them)
        for (self.ast.program.decls.items) |decl| {
            switch (decl.decl) {
                .trait_decl => |td| {
                    self.trait_decls.put(td.name, &td) catch {};
                },
                .impl_decl => |impl_decl| try self.genImplDecl(&impl_decl),
                else => {},
            }
        }

        // Fourth pass: generate vtables (after methods exist, before functions that call them)
        try self.generateVtables();

        // Fifth pass: generate functions (skip generic ones — they're monomorphized on demand)
        for (self.ast.program.decls.items) |*decl| {
            switch (decl.decl) {
                .function => |*fn_decl| {
                    if (fn_decl.type_params.items.len > 0) {
                        self.generic_fn_decls.put(fn_decl.name, fn_decl) catch {};
                    } else {
                        try self.genFunction(fn_decl);
                    }
                },
                else => {},
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

    /// Generate vtables for all trait implementations.
    fn generateVtables(self: *CodeGen) GenError!void {
        const i8ptr_ty = c.LLVMPointerType(c.LLVMInt8TypeInContext(self.context), 0);

        for (self.ast.program.decls.items) |decl| {
            switch (decl.decl) {
                .impl_decl => |impl_decl| {
                    const trait_name = impl_decl.trait_name orelse continue;
                    const type_name = impl_decl.type_name;
                    const td = self.trait_decls.get(trait_name) orelse continue;

                    const struct_info = self.struct_types.get(type_name) orelse continue;
                    const method_count = td.methods.items.len;

                    // Build vtable struct type: { fn_ptr, fn_ptr, ... }
                    var vtable_field_types = std.ArrayList(c.LLVMTypeRef){};
                    defer vtable_field_types.deinit(self.allocator);

                    // Build vtable function type and wrapper for each trait method
                    var vtable_values = std.ArrayList(c.LLVMValueRef){};
                    defer vtable_values.deinit(self.allocator);

                    for (td.methods.items) |trait_method| {
                        // Wrapper function type: ret_type(i8*, param_types...)
                        const ret_tag = self.resolveTypeExpr(trait_method.return_type);
                        const ret_llvm = self.typeTagToLLVM(ret_tag);

                        var wrapper_param_types = std.ArrayList(c.LLVMTypeRef){};
                        defer wrapper_param_types.deinit(self.allocator);
                        wrapper_param_types.append(self.allocator, i8ptr_ty) catch {}; // self as i8*
                        for (trait_method.params.items) |param| {
                            const pt = self.resolveTypeExpr(param.type_expr);
                            wrapper_param_types.append(self.allocator, self.typeTagToLLVM(pt)) catch {};
                        }
                        const wrapper_fn_type = c.LLVMFunctionType(ret_llvm, wrapper_param_types.items.ptr, @intCast(wrapper_param_types.items.len), 0);
                        vtable_field_types.append(self.allocator, c.LLVMPointerType(wrapper_fn_type, 0)) catch {};

                        // Generate wrapper function
                        const wrapper_name = std.fmt.allocPrint(self.allocator, "__vtable_{s}_{s}_{s}", .{ trait_name, type_name, trait_method.name }) catch return error.CodeGenError;
                        self.monomorphized_names.append(self.allocator, wrapper_name) catch {};
                        const wrapper_name_z = self.allocator.dupeZ(u8, wrapper_name) catch return error.CodeGenError;
                        defer self.allocator.free(wrapper_name_z);

                        const wrapper_fn = c.LLVMAddFunction(self.module, wrapper_name_z.ptr, wrapper_fn_type);
                        const wrapper_entry = c.LLVMAppendBasicBlockInContext(self.context, wrapper_fn, "entry");

                        const saved_block = c.LLVMGetInsertBlock(self.builder);
                        c.LLVMPositionBuilderAtEnd(self.builder, wrapper_entry);

                        // Bitcast i8* self to struct pointer, load it
                        const self_param = c.LLVMGetParam(wrapper_fn, 0);
                        const typed_ptr = c.LLVMBuildBitCast(self.builder, self_param, c.LLVMPointerType(struct_info.llvm_type, 0), "typed_self");
                        const loaded_self = c.LLVMBuildLoad2(self.builder, struct_info.llvm_type, typed_ptr, "self_val");

                        // Look up the real method
                        const real_name = std.fmt.allocPrint(self.allocator, "{s}_{s}", .{ type_name, trait_method.name }) catch return error.CodeGenError;
                        defer self.allocator.free(real_name);
                        const real_name_z = self.allocator.dupeZ(u8, real_name) catch return error.CodeGenError;
                        defer self.allocator.free(real_name_z);
                        const real_fn = c.LLVMGetNamedFunction(self.module, real_name_z.ptr) orelse {
                            self.diagnostics.report(.@"error", 0, "method '{s}' not found for vtable", .{real_name});
                            return error.CodeGenError;
                        };

                        // Call real method with loaded self + forwarded params
                        var call_args = std.ArrayList(c.LLVMValueRef){};
                        defer call_args.deinit(self.allocator);
                        call_args.append(self.allocator, loaded_self) catch {};
                        var pi: u32 = 1;
                        while (pi < @as(u32, @intCast(wrapper_param_types.items.len))) : (pi += 1) {
                            call_args.append(self.allocator, c.LLVMGetParam(wrapper_fn, pi)) catch {};
                        }

                        const real_fn_type = c.LLVMGlobalGetValueType(real_fn);
                        const is_void = c.LLVMGetTypeKind(ret_llvm) == c.LLVMVoidTypeKind;
                        if (is_void) {
                            _ = c.LLVMBuildCall2(self.builder, real_fn_type, real_fn, call_args.items.ptr, @intCast(call_args.items.len), "");
                            _ = c.LLVMBuildRetVoid(self.builder);
                        } else {
                            const result = c.LLVMBuildCall2(self.builder, real_fn_type, real_fn, call_args.items.ptr, @intCast(call_args.items.len), "vtable_call");
                            _ = c.LLVMBuildRet(self.builder, result);
                        }

                        c.LLVMPositionBuilderAtEnd(self.builder, saved_block);
                        vtable_values.append(self.allocator, wrapper_fn) catch {};
                    }

                    // Create vtable struct type
                    const vtable_struct_type = c.LLVMStructTypeInContext(self.context, vtable_field_types.items.ptr, @intCast(method_count), 0);

                    // Create global vtable constant
                    const vtable_global_name = std.fmt.allocPrint(self.allocator, "__vtable_{s}_{s}", .{ trait_name, type_name }) catch return error.CodeGenError;
                    self.monomorphized_names.append(self.allocator, vtable_global_name) catch {};
                    const vtable_global_name_z = self.allocator.dupeZ(u8, vtable_global_name) catch return error.CodeGenError;
                    defer self.allocator.free(vtable_global_name_z);

                    const vtable_const = c.LLVMConstStructInContext(self.context, vtable_values.items.ptr, @intCast(method_count), 0);
                    const vtable_global = c.LLVMAddGlobal(self.module, vtable_struct_type, vtable_global_name_z.ptr);
                    c.LLVMSetInitializer(vtable_global, vtable_const);
                    c.LLVMSetGlobalConstant(vtable_global, 1);

                    const key = std.fmt.allocPrint(self.allocator, "{s}_{s}", .{ trait_name, type_name }) catch return error.CodeGenError;
                    self.monomorphized_names.append(self.allocator, key) catch {};
                    self.vtable_globals.put(key, vtable_global) catch {};
                },
                else => {},
            }
        }
    }

    /// Generate code for an imported module's public functions.
    /// Temporarily swaps the AST pointer so genFunction can resolve expressions.
    /// Functions are mangled as __pkg_{package_name}_{fn_name} for namespace isolation.
    pub fn generateImportedModule(self: *CodeGen, imported_ast: *const Ast, package_name: []const u8) GenError!void {
        const saved_ast = self.ast;
        self.ast = imported_ast;
        defer self.ast = saved_ast;

        // Pass 1: Register type aliases from imported module
        for (imported_ast.program.decls.items) |decl| {
            if (decl.visibility != .public) continue;
            switch (decl.decl) {
                .type_alias => |ta| { self.type_aliases.put(ta.name, ta.target) catch {}; },
                else => {},
            }
        }

        // Pass 2: Register struct and enum types
        for (imported_ast.program.decls.items) |decl| {
            if (decl.visibility != .public) continue;
            switch (decl.decl) {
                .struct_decl => |sd| try self.genStructType(&sd),
                .enum_decl => |ed| try self.genEnumType(&ed),
                else => {},
            }
        }

        // Pass 3: Generate constants
        for (imported_ast.program.decls.items) |decl| {
            if (decl.visibility != .public) continue;
            switch (decl.decl) {
                .const_decl => |cd| try self.genConstDecl(&cd),
                else => {},
            }
        }

        // Pass 4: Generate impl methods (for methods on imported structs)
        for (imported_ast.program.decls.items) |decl| {
            if (decl.visibility != .public) continue;
            switch (decl.decl) {
                .impl_decl => |impl_decl| {
                    try self.genImplDecl(&impl_decl);
                    // Register method return types so lookupMethodReturnType can find them
                    for (impl_decl.methods.items) |method| {
                        const mangled = std.fmt.allocPrint(self.allocator, "{s}_{s}", .{ impl_decl.type_name, method.name }) catch continue;
                        self.monomorphized_names.append(self.allocator, mangled) catch {};
                        const ret_tag = self.resolveTypeExpr(method.return_type);
                        self.imported_fn_return_types.put(mangled, ret_tag) catch {};
                    }
                },
                else => {},
            }
        }

        // Pass 5: Generate public functions (with name mangling)
        for (imported_ast.program.decls.items) |decl| {
            if (decl.visibility != .public) continue;
            switch (decl.decl) {
                .function => |fn_decl| {
                    if (std.mem.eql(u8, fn_decl.name, "main")) continue;
                    const mangled = std.fmt.allocPrint(self.allocator, "__pkg_{s}_{s}", .{ package_name, fn_decl.name }) catch return error.CodeGenError;
                    self.monomorphized_names.append(self.allocator, mangled) catch {};
                    const ret_tag = self.resolveTypeExpr(fn_decl.return_type);
                    self.imported_fn_return_types.put(mangled, ret_tag) catch {};
                    try self.genFunctionWithName(&fn_decl, mangled);
                },
                else => {},
            }
        }
    }

    /// Generate a function with an explicit LLVM name (for name mangling of imported functions).
    pub fn genFunctionWithName(self: *CodeGen, fn_decl: *const Ast.FnDecl, llvm_name: []const u8) GenError!void {
        return codegen_stmts.genFunctionWithName(self, fn_decl, llvm_name);
    }

    // -----------------------------------------------------------------------
    // Delegating methods — type generation (codegen_types.zig)
    // -----------------------------------------------------------------------

    fn genStructType(self: *CodeGen, sd: *const Ast.StructDecl) GenError!void {
        return codegen_types.genStructType(self, sd);
    }

    pub fn monomorphizeStruct(self: *CodeGen, generic_name: []const u8, type_args: []const Ast.TypeExpr) GenError![]const u8 {
        return codegen_types.monomorphizeStruct(self, generic_name, type_args);
    }

    fn monomorphizeEnum(self: *CodeGen, generic_name: []const u8, type_args: []const Ast.TypeExpr) GenError![]const u8 {
        return codegen_types.monomorphizeEnum(self, generic_name, type_args);
    }

    pub fn resolveTypeExprWithSubst(self: *CodeGen, type_expr: Ast.TypeExpr, type_map: *const std.StringHashMap(TypeTag)) TypeTag {
        return codegen_types.resolveTypeExprWithSubst(self, type_expr, type_map);
    }

    fn genEnumType(self: *CodeGen, ed: *const Ast.EnumDecl) GenError!void {
        return codegen_types.genEnumType(self, ed);
    }

    fn ensureOptionType(self: *CodeGen, inner_tag: TypeTag) void {
        return codegen_types.ensureOptionType(self, inner_tag);
    }

    fn ensureResultType(self: *CodeGen, ok_tag: TypeTag, err_tag: TypeTag) void {
        return codegen_types.ensureResultType(self, ok_tag, err_tag);
    }

    pub fn genEnumConstructor(self: *CodeGen, enum_name: []const u8, variant_name: []const u8, args: *const std.ArrayList(Ast.ExprIndex)) GenError!ExprResult {
        return codegen_types.genEnumConstructor(self, enum_name, variant_name, args);
    }

    pub fn genGenericImplMethods(self: *CodeGen, mangled_name: []const u8, impl_decl: *const Ast.ImplDecl, sd: *const Ast.StructDecl, type_args: []const Ast.TypeExpr) GenError!void {
        return codegen_types.genGenericImplMethods(self, mangled_name, impl_decl, sd, type_args);
    }

    pub fn findFieldIndex(self: *const CodeGen, info: StructTypeInfo, field_name: []const u8) ?usize {
        return codegen_types.findFieldIndex(self, info, field_name);
    }

    // -----------------------------------------------------------------------
    // Delegating methods — statement + function codegen (codegen_stmts.zig)
    // -----------------------------------------------------------------------

    fn genFunction(self: *CodeGen, fn_decl: *const Ast.FnDecl) GenError!void {
        return codegen_stmts.genFunction(self, fn_decl);
    }

    pub fn genBlock(self: *CodeGen, block: *const Ast.Block, is_main: bool) GenError!void {
        return codegen_stmts.genBlock(self, block, is_main);
    }

    fn genImplDecl(self: *CodeGen, impl_decl: *const Ast.ImplDecl) GenError!void {
        return codegen_stmts.genImplDecl(self, impl_decl);
    }

    fn genExternDecl(self: *CodeGen, ed: *const Ast.ExternDecl) GenError!void {
        return codegen_stmts.genExternDecl(self, ed);
    }

    fn genConstDecl(self: *CodeGen, cd: *const Ast.ConstDecl) GenError!void {
        return codegen_stmts.genConstDecl(self, cd);
    }

    // -----------------------------------------------------------------------
    // Delegating methods — expression codegen (codegen_exprs.zig)
    // -----------------------------------------------------------------------

    pub fn genExpr(self: *CodeGen, expr_idx: Ast.ExprIndex) GenError!ExprResult {
        return codegen_exprs.genExpr(self, expr_idx);
    }

    // -----------------------------------------------------------------------
    // Core type resolution and conversion — kept in codegen.zig
    // -----------------------------------------------------------------------

    pub fn typeExprName(_: *CodeGen, type_expr: Ast.TypeExpr) []const u8 {
        return switch (type_expr) {
            .builtin => |b| switch (b) {
                .i32_type, .int_type => "i32",
                .i64_type => "i64",
                .f64_type, .float_type => "f64",
                .f32_type => "f32",
                .bool_type => "bool",
                .string_type => "string",
                .char_type => "char",
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

    pub fn resolveTypeExpr(self: *CodeGen, type_expr: Ast.TypeExpr) TypeTag {
        return switch (type_expr) {
            .builtin => |b| switch (b) {
                .void => .void_type,
                .bool_type => .bool_type,
                .i8_type => .i8_type,
                .i16_type => .i16_type,
                .i32_type, .int_type => .i32_type,
                .i64_type => .i64_type,
                .i128_type => .i128_type,
                .u8_type => .u8_type,
                .u16_type => .u16_type,
                .u32_type => .u32_type,
                .u64_type => .u64_type,
                .u128_type => .u128_type,
                .f64_type, .float_type => .f64_type,
                .string_type => .string_type,
                .char_type => .char_type,
                else => .i32_type,
            },
            .named => |name| {
                // Check type aliases first
                if (self.type_aliases.get(name)) |alias_target| {
                    return self.resolveTypeExpr(alias_target);
                }
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
                const elem_ptr = self.type_tag_arena.allocator().create(TypeTag) catch return .i32_type;
                elem_ptr.* = elem_tag;
                return .{ .array_type = .{ .element_type = elem_ptr, .count = arr.size } };
            },
            .slice_type => |sl| {
                const elem_tag = self.resolveTypeExpr(sl.element_type.*);
                const elem_ptr = self.type_tag_arena.allocator().create(TypeTag) catch return .i32_type;
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
                var elem_tags = self.type_tag_arena.allocator().alloc(TypeTag, count) catch return .i32_type;
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
                const pointee_ptr = self.type_tag_arena.allocator().create(TypeTag) catch return .i32_type;
                pointee_ptr.* = pointee_tag;
                return .{ .ptr_type = .{ .pointee = pointee_ptr, .is_mutable = pt.is_mutable } };
            },
            .dyn_type => |trait_name| {
                return .{ .trait_type = trait_name };
            },
            .fn_type => |ft| {
                const count = ft.param_types.items.len;
                const param_tags = self.type_tag_arena.allocator().alloc(TypeTag, count) catch return .i32_type;
                for (ft.param_types.items, 0..) |pt, i| {
                    param_tags[i] = self.resolveTypeExpr(pt);
                }
                const ret_tag = self.resolveTypeExpr(ft.return_type.*);
                const ret_ptr = self.type_tag_arena.allocator().create(TypeTag) catch return .i32_type;
                ret_ptr.* = ret_tag;
                return .{ .fn_type = .{ .param_types = param_tags, .return_type = ret_ptr } };
            },
            .inferred => {
                // Inferred types should be resolved at the call site, not here
                return .i32_type;
            },
        };
    }

    pub fn isTypeTag(tag: TypeTag, comptime expected: @typeInfo(TypeTag).@"union".tag_type.?) bool {
        return @as(@typeInfo(TypeTag).@"union".tag_type.?, tag) == expected;
    }

    pub fn isUnsignedInt(tag: TypeTag) bool {
        return switch (tag) {
            .u8_type, .u16_type, .u32_type, .u64_type, .u128_type => true,
            else => false,
        };
    }

    pub fn isIntegerType(tag: TypeTag) bool {
        return switch (tag) {
            .i8_type, .i16_type, .i32_type, .i64_type, .i128_type,
            .u8_type, .u16_type, .u32_type, .u64_type, .u128_type,
            => true,
            else => false,
        };
    }

    pub fn getTypeConversion(_: *const CodeGen, name: []const u8) ?TypeTag {
        if (std.mem.eql(u8, name, "i8")) return .i8_type;
        if (std.mem.eql(u8, name, "i16")) return .i16_type;
        if (std.mem.eql(u8, name, "i32")) return .i32_type;
        if (std.mem.eql(u8, name, "i64")) return .i64_type;
        if (std.mem.eql(u8, name, "i128")) return .i128_type;
        if (std.mem.eql(u8, name, "u8")) return .u8_type;
        if (std.mem.eql(u8, name, "u16")) return .u16_type;
        if (std.mem.eql(u8, name, "u32")) return .u32_type;
        if (std.mem.eql(u8, name, "u64")) return .u64_type;
        if (std.mem.eql(u8, name, "u128")) return .u128_type;
        if (std.mem.eql(u8, name, "f64")) return .f64_type;
        if (std.mem.eql(u8, name, "bool")) return .bool_type;
        if (std.mem.eql(u8, name, "char")) return .char_type;
        return null;
    }

    fn intBitWidth(tag: TypeTag) ?u32 {
        return switch (tag) {
            .i8_type, .u8_type, .char_type => 8,
            .i16_type, .u16_type => 16,
            .i32_type, .u32_type => 32,
            .i64_type, .u64_type => 64,
            .i128_type, .u128_type => 128,
            else => null,
        };
    }

    pub fn genTypeConversion(self: *CodeGen, src: ExprResult, target: TypeTag) GenError!ExprResult {
        const src_tag: @typeInfo(TypeTag).@"union".tag_type.? = src.type_tag;
        const tgt_tag: @typeInfo(TypeTag).@"union".tag_type.? = target;
        if (src_tag == tgt_tag) return src;

        const target_llvm = self.typeTagToLLVM(target);
        const src_is_unsigned = isUnsignedInt(src.type_tag);
        const tgt_is_unsigned = isUnsignedInt(target);

        const value: c.LLVMValueRef = if (isTypeTag(target, .f64_type)) blk: {
            // Any integer/char -> f64
            if (isIntegerType(src.type_tag) or isTypeTag(src.type_tag, .char_type)) {
                if (src_is_unsigned) {
                    break :blk c.LLVMBuildUIToFP(self.builder, src.value, target_llvm, "uitofp");
                } else {
                    break :blk c.LLVMBuildSIToFP(self.builder, src.value, target_llvm, "sitofp");
                }
            }
            self.diagnostics.report(.@"error", 0, "cannot convert to f64", .{});
            return error.CodeGenError;
        } else if (isIntegerType(target) or isTypeTag(target, .char_type)) blk: {
            // f64 -> integer
            if (isTypeTag(src.type_tag, .f64_type)) {
                if (tgt_is_unsigned) {
                    break :blk c.LLVMBuildFPToUI(self.builder, src.value, target_llvm, "fptoui");
                } else {
                    break :blk c.LLVMBuildFPToSI(self.builder, src.value, target_llvm, "fptosi");
                }
            }
            // integer/char -> integer/char: compare bit widths
            const src_width = intBitWidth(src.type_tag) orelse {
                self.diagnostics.report(.@"error", 0, "cannot convert to integer", .{});
                return error.CodeGenError;
            };
            const tgt_width = intBitWidth(target) orelse {
                self.diagnostics.report(.@"error", 0, "cannot convert to integer", .{});
                return error.CodeGenError;
            };
            if (src_width == tgt_width) {
                // Same width (e.g., i32 -> u32): no LLVM instruction needed
                break :blk src.value;
            } else if (tgt_width > src_width) {
                // Widening
                if (src_is_unsigned or isTypeTag(src.type_tag, .char_type)) {
                    break :blk c.LLVMBuildZExt(self.builder, src.value, target_llvm, "zext");
                } else {
                    break :blk c.LLVMBuildSExt(self.builder, src.value, target_llvm, "sext");
                }
            } else {
                // Narrowing
                break :blk c.LLVMBuildTrunc(self.builder, src.value, target_llvm, "trunc");
            }
        } else if (isTypeTag(target, .bool_type)) blk: {
            if (isIntegerType(src.type_tag)) {
                break :blk c.LLVMBuildICmp(self.builder, c.LLVMIntNE, src.value, c.LLVMConstInt(self.typeTagToLLVM(src.type_tag), 0, 0), "to_bool");
            }
            self.diagnostics.report(.@"error", 0, "unsupported type conversion", .{});
            return error.CodeGenError;
        } else {
            self.diagnostics.report(.@"error", 0, "unsupported type conversion", .{});
            return error.CodeGenError;
        };

        return .{ .value = value, .type_tag = target };
    }

    pub fn typeTagToLLVM(self: *const CodeGen, tag: TypeTag) c.LLVMTypeRef {
        return switch (tag) {
            .i8_type, .u8_type => c.LLVMInt8TypeInContext(self.context),
            .i16_type, .u16_type => c.LLVMInt16TypeInContext(self.context),
            .i32_type, .u32_type => c.LLVMInt32TypeInContext(self.context),
            .i64_type, .u64_type => c.LLVMInt64TypeInContext(self.context),
            .i128_type, .u128_type => c.LLVMInt128TypeInContext(self.context),
            .f64_type => c.LLVMDoubleTypeInContext(self.context),
            .bool_type => c.LLVMInt1TypeInContext(self.context),
            .char_type => c.LLVMInt8TypeInContext(self.context),
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
            .closure_type => {
                // Closure variable stores pointer to env struct
                return c.LLVMPointerType(c.LLVMInt8TypeInContext(self.context), 0);
            },
            .fn_type => {
                // Closure descriptor: { fn_ptr: ptr, env_ptr: ptr }
                const ptr_ty = c.LLVMPointerType(c.LLVMInt8TypeInContext(self.context), 0);
                var field_types_arr = [_]c.LLVMTypeRef{ ptr_ty, ptr_ty };
                return c.LLVMStructTypeInContext(self.context, &field_types_arr, 2, 0);
            },
            .trait_type => {
                // Trait object fat pointer: { data_ptr: ptr, vtable_ptr: ptr }
                const ptr_ty = c.LLVMPointerType(c.LLVMInt8TypeInContext(self.context), 0);
                var field_types_arr = [_]c.LLVMTypeRef{ ptr_ty, ptr_ty };
                return c.LLVMStructTypeInContext(self.context, &field_types_arr, 2, 0);
            },
        };
    }

    pub fn sliceLLVMType(self: *const CodeGen) c.LLVMTypeRef {
        var field_types_arr = [_]c.LLVMTypeRef{
            c.LLVMPointerType(c.LLVMInt8TypeInContext(self.context), 0),
            c.LLVMInt32TypeInContext(self.context),
        };
        return c.LLVMStructTypeInContext(self.context, &field_types_arr, 2, 0);
    }

    pub fn getImportPackageName(self: *const CodeGen, name: []const u8) ?[]const u8 {
        return self.ast.program.import_aliases.get(name);
    }

    pub fn emitObjectFile(self: *CodeGen, path: []const u8) GenError!void {
        // Finalize debug info before emission
        if (self.di_builder) |dib| {
            c.LLVMDIBuilderFinalize(dib);
        }

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
            .char_type => .char_type,
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
