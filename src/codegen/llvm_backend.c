/**
 * Asthra Programming Language LLVM Backend
 * Generates LLVM IR from Asthra AST
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "backend_interface.h"
#include "../parser/ast_types.h"
#include "../analysis/semantic_symbols.h"
#include "../analysis/type_info.h"
#include "../compiler.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>

#ifdef ASTHRA_ENABLE_LLVM_BACKEND
#include <llvm-c/Core.h>
#include <llvm-c/Analysis.h>
#include <llvm-c/BitWriter.h>
#include <llvm-c/Target.h>
#include <llvm-c/TargetMachine.h>
#include <llvm-c/DebugInfo.h>
#endif

// Private data for LLVM backend
typedef struct LLVMBackendData {
#ifdef ASTHRA_ENABLE_LLVM_BACKEND
    LLVMContextRef context;
    LLVMModuleRef module;
    LLVMBuilderRef builder;
    LLVMValueRef current_function;
    AsthraCompilerContext *compiler_ctx;
    
    // Type cache for commonly used types
    LLVMTypeRef i32_type;
    LLVMTypeRef i64_type;
    LLVMTypeRef f32_type;
    LLVMTypeRef f64_type;
    LLVMTypeRef bool_type;
    LLVMTypeRef void_type;
    LLVMTypeRef unit_type;
    LLVMTypeRef ptr_type;
    
    // Runtime function declarations
    LLVMValueRef runtime_malloc_fn;
    LLVMValueRef runtime_free_fn;
    LLVMValueRef runtime_panic_fn;
    LLVMValueRef runtime_log_fn;
    
    // Debug info support
    LLVMDIBuilderRef di_builder;
    LLVMMetadataRef di_compile_unit;
    LLVMMetadataRef di_file;
    LLVMMetadataRef current_debug_scope;
    
    // Debug type cache
    LLVMMetadataRef di_i32_type;
    LLVMMetadataRef di_i64_type;
    LLVMMetadataRef di_f32_type;
    LLVMMetadataRef di_f64_type;
    LLVMMetadataRef di_bool_type;
    LLVMMetadataRef di_void_type;
    LLVMMetadataRef di_ptr_type;
#else
    void *dummy; // Keep struct non-empty
#endif
    char *output_filename;
} LLVMBackendData;

// Forward declarations
static void declare_runtime_functions(LLVMBackendData *data);
static void initialize_debug_info(LLVMBackendData *data, const char *filename, bool is_optimized);
static void cache_basic_debug_types(LLVMBackendData *data);
static LLVMMetadataRef asthra_type_to_debug_type(LLVMBackendData *data, const TypeInfo *type);
static void set_debug_location(LLVMBackendData *data, const ASTNode *node);
static void finalize_debug_info(LLVMBackendData *data);
static LLVMValueRef generate_expression(LLVMBackendData *data, const ASTNode *node);
static void generate_statement(LLVMBackendData *data, const ASTNode *node);
static LLVMValueRef generate_identifier(LLVMBackendData *data, const ASTNode *node);
static LLVMValueRef generate_unary_op(LLVMBackendData *data, const ASTNode *node);

// Initialize the LLVM backend
static int llvm_backend_initialize(AsthraBackend *backend, const AsthraCompilerOptions *options) {
    if (!backend || !options) return -1;
    
#ifndef ASTHRA_ENABLE_LLVM_BACKEND
    backend->last_error = "LLVM backend support not compiled in. Please rebuild with -DENABLE_LLVM_BACKEND=ON";
    return -1;
#else
    // Allocate private data
    LLVMBackendData *data = calloc(1, sizeof(LLVMBackendData));
    if (!data) {
        backend->last_error = "Failed to allocate LLVM backend data";
        return -1;
    }
    
    backend->private_data = data;
    
    // Initialize LLVM
    LLVMInitializeNativeTarget();
    LLVMInitializeNativeAsmPrinter();
    LLVMInitializeNativeAsmParser();
    
    // Create context and module
    data->context = LLVMContextCreate();
    if (!data->context) {
        backend->last_error = "Failed to create LLVM context";
        free(data);
        return -1;
    }
    
    const char *module_name = options->input_file ? options->input_file : "asthra_module";
    data->module = LLVMModuleCreateWithNameInContext(module_name, data->context);
    if (!data->module) {
        backend->last_error = "Failed to create LLVM module";
        LLVMContextDispose(data->context);
        free(data);
        return -1;
    }
    
    // Create IR builder
    data->builder = LLVMCreateBuilderInContext(data->context);
    if (!data->builder) {
        backend->last_error = "Failed to create LLVM builder";
        LLVMDisposeModule(data->module);
        LLVMContextDispose(data->context);
        free(data);
        return -1;
    }
    
    // Set target triple
    char *triple = LLVMGetDefaultTargetTriple();
    LLVMSetTarget(data->module, triple);
    LLVMDisposeMessage(triple);
    
    // Cache common types
    data->i32_type = LLVMInt32TypeInContext(data->context);
    data->i64_type = LLVMInt64TypeInContext(data->context);
    data->f32_type = LLVMFloatTypeInContext(data->context);
    data->f64_type = LLVMDoubleTypeInContext(data->context);
    data->bool_type = LLVMInt1TypeInContext(data->context);
    data->void_type = LLVMVoidTypeInContext(data->context);
    data->ptr_type = LLVMPointerType(LLVMInt8TypeInContext(data->context), 0);
    
    // Unit type is represented as an empty struct
    data->unit_type = LLVMStructTypeInContext(data->context, NULL, 0, 0);
    
    // Declare runtime functions
    declare_runtime_functions(data);
    
    // Store options
    backend->options.optimization_level = options->opt_level;
    backend->options.debug_info = options->debug_info;
    backend->options.verbose = options->verbose;
    backend->options.target_arch = options->target_arch;
    backend->options.asm_syntax = options->asm_syntax;
    
    // Initialize debug info if requested
    if (options->debug_info) {
        const char *filename = options->input_file ? options->input_file : "asthra_module";
        bool is_optimized = options->opt_level > ASTHRA_OPT_NONE;
        initialize_debug_info(data, filename, is_optimized);
    }
    
    // Success
    return 0;
#endif
}

#ifdef ASTHRA_ENABLE_LLVM_BACKEND

// Additional forward declarations  
static void generate_function(LLVMBackendData *data, const ASTNode *node);
static LLVMTypeRef asthra_type_to_llvm(LLVMBackendData *data, const TypeInfo *type);

// Declare Asthra runtime functions in LLVM module
static void declare_runtime_functions(LLVMBackendData *data) {
    // Memory allocation functions
    {
        // void* asthra_alloc(size_t size, int zone)
        LLVMTypeRef param_types[] = {data->i64_type, data->i32_type};
        LLVMTypeRef fn_type = LLVMFunctionType(data->ptr_type, param_types, 2, false);
        data->runtime_malloc_fn = LLVMAddFunction(data->module, "asthra_alloc", fn_type);
        LLVMSetLinkage(data->runtime_malloc_fn, LLVMExternalLinkage);
    }
    
    {
        // void asthra_free(void* ptr, int zone)
        LLVMTypeRef param_types[] = {data->ptr_type, data->i32_type};
        LLVMTypeRef fn_type = LLVMFunctionType(data->void_type, param_types, 2, false);
        data->runtime_free_fn = LLVMAddFunction(data->module, "asthra_free", fn_type);
        LLVMSetLinkage(data->runtime_free_fn, LLVMExternalLinkage);
    }
    
    // GC functions
    {
        // void asthra_gc_register_root(void* ptr)
        LLVMTypeRef param_types[] = {data->ptr_type};
        LLVMTypeRef fn_type = LLVMFunctionType(data->void_type, param_types, 1, false);
        LLVMValueRef fn = LLVMAddFunction(data->module, "asthra_gc_register_root", fn_type);
        LLVMSetLinkage(fn, LLVMExternalLinkage);
    }
    
    {
        // void asthra_gc_unregister_root(void* ptr)
        LLVMTypeRef param_types[] = {data->ptr_type};
        LLVMTypeRef fn_type = LLVMFunctionType(data->void_type, param_types, 1, false);
        LLVMValueRef fn = LLVMAddFunction(data->module, "asthra_gc_unregister_root", fn_type);
        LLVMSetLinkage(fn, LLVMExternalLinkage);
    }
    
    // Error handling
    {
        // void asthra_panic(const char* message)
        LLVMTypeRef param_types[] = {data->ptr_type};
        LLVMTypeRef fn_type = LLVMFunctionType(data->void_type, param_types, 1, false);
        data->runtime_panic_fn = LLVMAddFunction(data->module, "asthra_panic", fn_type);
        LLVMSetLinkage(data->runtime_panic_fn, LLVMExternalLinkage);
    }
    
    // Logging
    {
        // void asthra_log(int level, const char* format, ...)
        LLVMTypeRef param_types[] = {data->i32_type, data->ptr_type};
        LLVMTypeRef fn_type = LLVMFunctionType(data->void_type, param_types, 2, true); // variadic
        data->runtime_log_fn = LLVMAddFunction(data->module, "asthra_log", fn_type);
        LLVMSetLinkage(data->runtime_log_fn, LLVMExternalLinkage);
    }
    
    // String operations
    {
        // AsthraString* asthra_string_create(const char* data, size_t length)
        LLVMTypeRef param_types[] = {data->ptr_type, data->i64_type};
        LLVMTypeRef fn_type = LLVMFunctionType(data->ptr_type, param_types, 2, false);
        LLVMValueRef fn = LLVMAddFunction(data->module, "asthra_string_create", fn_type);
        LLVMSetLinkage(fn, LLVMExternalLinkage);
    }
    
    {
        // void asthra_string_free(AsthraString* str)
        LLVMTypeRef param_types[] = {data->ptr_type};
        LLVMTypeRef fn_type = LLVMFunctionType(data->void_type, param_types, 1, false);
        LLVMValueRef fn = LLVMAddFunction(data->module, "asthra_string_free", fn_type);
        LLVMSetLinkage(fn, LLVMExternalLinkage);
    }
    
    // Print function for basic output (temporary until proper I/O)
    {
        // int printf(const char* format, ...)
        LLVMTypeRef param_types[] = {data->ptr_type};
        LLVMTypeRef fn_type = LLVMFunctionType(data->i32_type, param_types, 1, true); // variadic
        LLVMValueRef fn = LLVMAddFunction(data->module, "printf", fn_type);
        LLVMSetLinkage(fn, LLVMExternalLinkage);
    }
}

// Initialize debug info support
static void initialize_debug_info(LLVMBackendData *data, const char *filename, bool is_optimized) {
    // Create DIBuilder
    data->di_builder = LLVMCreateDIBuilder(data->module);
    
    // Extract directory and filename
    const char *last_slash = strrchr(filename, '/');
    char *directory = ".";
    const char *file_only = filename;
    
    if (last_slash) {
        size_t dir_len = last_slash - filename;
        directory = malloc(dir_len + 1);
        strncpy(directory, filename, dir_len);
        directory[dir_len] = '\0';
        file_only = last_slash + 1;
    }
    
    // Create file descriptor
    data->di_file = LLVMDIBuilderCreateFile(data->di_builder, 
        file_only, strlen(file_only),
        directory, strlen(directory));
    
    // Create compile unit
    data->di_compile_unit = LLVMDIBuilderCreateCompileUnit(
        data->di_builder,
        LLVMDWARFSourceLanguageC,      // Using C as closest language
        data->di_file,
        "Asthra Compiler", 15,         // Producer
        is_optimized,
        "", 0,                         // Flags
        0,                             // Runtime version
        "", 0,                         // Split name
        LLVMDWARFEmissionFull,
        0,                             // DWOId
        true,                          // Split debug inlining
        false,                         // Debug info for profiling
        "", 0,                         // SysRoot
        "", 0                          // SDK
    );
    
    // Set as current debug scope
    data->current_debug_scope = data->di_compile_unit;
    
    // Cache basic debug types
    cache_basic_debug_types(data);
    
    // Free allocated directory if needed
    if (last_slash && strcmp(directory, ".") != 0) {
        free(directory);
    }
}

// DWARF type encoding constants from DWARF standard
#define DW_ATE_signed 0x05
#define DW_ATE_unsigned 0x07
#define DW_ATE_float 0x04
#define DW_ATE_boolean 0x02
#define DW_ATE_signed_char 0x06
#define DW_ATE_unsigned_char 0x08

// Cache commonly used debug types
static void cache_basic_debug_types(LLVMBackendData *data) {
    // Integer types
    data->di_i32_type = LLVMDIBuilderCreateBasicType(
        data->di_builder, "i32", 3, 32, DW_ATE_signed, LLVMDIFlagZero);
    data->di_i64_type = LLVMDIBuilderCreateBasicType(
        data->di_builder, "i64", 3, 64, DW_ATE_signed, LLVMDIFlagZero);
    
    // Floating point types
    data->di_f32_type = LLVMDIBuilderCreateBasicType(
        data->di_builder, "f32", 3, 32, DW_ATE_float, LLVMDIFlagZero);
    data->di_f64_type = LLVMDIBuilderCreateBasicType(
        data->di_builder, "f64", 3, 64, DW_ATE_float, LLVMDIFlagZero);
    
    // Boolean type
    data->di_bool_type = LLVMDIBuilderCreateBasicType(
        data->di_builder, "bool", 4, 1, DW_ATE_boolean, LLVMDIFlagZero);
    
    // Void type (as unspecified type)
    data->di_void_type = LLVMDIBuilderCreateUnspecifiedType(
        data->di_builder, "void", 4);
    
    // Pointer type (void*)
    data->di_ptr_type = LLVMDIBuilderCreatePointerType(
        data->di_builder, data->di_void_type, 64, 0, 0, "", 0);
}

// Convert Asthra type to debug type
static LLVMMetadataRef asthra_type_to_debug_type(LLVMBackendData *data, const TypeInfo *type) {
    if (!type || !data->di_builder) return data->di_void_type;
    
    switch (type->category) {
        case TYPE_INFO_PRIMITIVE:
            switch (type->data.primitive.kind) {
                case PRIMITIVE_INFO_I32: return data->di_i32_type;
                case PRIMITIVE_INFO_I64: return data->di_i64_type;
                case PRIMITIVE_INFO_U32: 
                    return LLVMDIBuilderCreateBasicType(
                        data->di_builder, "u32", 3, 32, DW_ATE_unsigned, LLVMDIFlagZero);
                case PRIMITIVE_INFO_U64:
                    return LLVMDIBuilderCreateBasicType(
                        data->di_builder, "u64", 3, 64, DW_ATE_unsigned, LLVMDIFlagZero);
                case PRIMITIVE_INFO_F32: return data->di_f32_type;
                case PRIMITIVE_INFO_F64: return data->di_f64_type;
                case PRIMITIVE_INFO_BOOL: return data->di_bool_type;
                case PRIMITIVE_INFO_VOID: return data->di_void_type;
                case PRIMITIVE_INFO_STRING:
                    // String as char* for now
                    return LLVMDIBuilderCreatePointerType(
                        data->di_builder,
                        LLVMDIBuilderCreateBasicType(
                            data->di_builder, "char", 4, 8, DW_ATE_signed_char, LLVMDIFlagZero),
                        64, 0, 0, "", 0);
                default:
                    return data->di_void_type;
            }
            
        case TYPE_INFO_POINTER:
            {
                LLVMMetadataRef pointee_type = asthra_type_to_debug_type(
                    data, type->data.pointer.pointee_type);
                return LLVMDIBuilderCreatePointerType(
                    data->di_builder, pointee_type, 64, 0, 0, "", 0);
            }
            
        case TYPE_INFO_FUNCTION:
            {
                // Create function debug type
                size_t param_count = type->data.function.param_count;
                LLVMMetadataRef *param_types = NULL;
                
                // First element is return type
                size_t type_count = param_count + 1;
                param_types = malloc(type_count * sizeof(LLVMMetadataRef));
                
                // Return type first
                param_types[0] = type->data.function.return_type ? 
                    asthra_type_to_debug_type(data, type->data.function.return_type) :
                    data->di_void_type;
                
                // Then parameter types
                for (size_t i = 0; i < param_count; i++) {
                    param_types[i + 1] = asthra_type_to_debug_type(data, type->data.function.param_types[i]);
                }
                
                LLVMMetadataRef fn_type = LLVMDIBuilderCreateSubroutineType(
                    data->di_builder,
                    data->di_file,
                    param_types,
                    (unsigned)type_count,
                    LLVMDIFlagZero
                );
                
                free(param_types);
                return fn_type;
            }
            
        case TYPE_INFO_STRUCT:
            // TODO: Implement struct type debug info
            return data->di_ptr_type;
            
        default:
            return data->di_void_type;
    }
}

// Set debug location for current instruction
static void set_debug_location(LLVMBackendData *data, const ASTNode *node) {
    if (!data->di_builder || !node) return;
    
    // Only set location if we have valid line info
    if (node->location.line > 0) {
        LLVMMetadataRef loc = LLVMDIBuilderCreateDebugLocation(
            data->context,
            node->location.line,
            node->location.column,
            data->current_debug_scope,
            NULL  // No inlined location
        );
        LLVMSetCurrentDebugLocation(data->builder, loc);
    }
}

// Finalize debug info
static void finalize_debug_info(LLVMBackendData *data) {
    if (data->di_builder) {
        LLVMDIBuilderFinalize(data->di_builder);
        
        // Add module flags for debug info
        LLVMAddModuleFlag(data->module, LLVMModuleFlagBehaviorWarning,
                          "Debug Info Version", 18,
                          LLVMValueAsMetadata(LLVMConstInt(data->i32_type, 3, false)));
        
        LLVMAddModuleFlag(data->module, LLVMModuleFlagBehaviorWarning,
                          "Dwarf Version", 13,
                          LLVMValueAsMetadata(LLVMConstInt(data->i32_type, 4, false)));
    }
}

// Convert Asthra type to LLVM type
static LLVMTypeRef asthra_type_to_llvm(LLVMBackendData *data, const TypeInfo *type) {
    if (!type) return data->void_type;
    
    switch (type->category) {
        case TYPE_INFO_PRIMITIVE:
            switch (type->data.primitive.kind) {
                case PRIMITIVE_INFO_I32: return data->i32_type;
                case PRIMITIVE_INFO_I64: return data->i64_type;
                case PRIMITIVE_INFO_U32: return LLVMInt32TypeInContext(data->context);
                case PRIMITIVE_INFO_U64: return LLVMInt64TypeInContext(data->context);
                case PRIMITIVE_INFO_I8:  return LLVMInt8TypeInContext(data->context);
                case PRIMITIVE_INFO_U8:  return LLVMInt8TypeInContext(data->context);
                case PRIMITIVE_INFO_I16: return LLVMInt16TypeInContext(data->context);
                case PRIMITIVE_INFO_U16: return LLVMInt16TypeInContext(data->context);
                case PRIMITIVE_INFO_I128: return LLVMInt128TypeInContext(data->context);
                case PRIMITIVE_INFO_U128: return LLVMInt128TypeInContext(data->context);
                case PRIMITIVE_INFO_F32: return data->f32_type;
                case PRIMITIVE_INFO_F64: return data->f64_type;
                case PRIMITIVE_INFO_BOOL: return data->bool_type;
                case PRIMITIVE_INFO_STRING: return data->ptr_type; // Strings are char*
                case PRIMITIVE_INFO_VOID: return data->void_type;
                default: return data->void_type;
            }
            
        case TYPE_INFO_SLICE:
            // Slices are represented as structs with ptr + length
            {
                LLVMTypeRef elem_type = asthra_type_to_llvm(data, type->data.slice.element_type);
                LLVMTypeRef fields[2] = {
                    LLVMPointerType(elem_type, 0),  // data pointer
                    data->i64_type                   // length
                };
                return LLVMStructTypeInContext(data->context, fields, 2, 0);
            }
            
        case TYPE_INFO_POINTER:
            {
                LLVMTypeRef pointee = asthra_type_to_llvm(data, type->data.pointer.pointee_type);
                return LLVMPointerType(pointee, 0);
            }
            
        case TYPE_INFO_FUNCTION:
            {
                // Convert parameter types
                size_t param_count = type->data.function.param_count;
                LLVMTypeRef *param_types = NULL;
                if (param_count > 0) {
                    param_types = malloc(param_count * sizeof(LLVMTypeRef));
                    for (size_t i = 0; i < param_count; i++) {
                        param_types[i] = asthra_type_to_llvm(data, type->data.function.param_types[i]);
                    }
                }
                
                LLVMTypeRef ret_type = asthra_type_to_llvm(data, type->data.function.return_type);
                LLVMTypeRef fn_type = LLVMFunctionType(ret_type, param_types, (unsigned)param_count, false);
                
                if (param_types) free(param_types);
                return LLVMPointerType(fn_type, 0);
            }
            
        case TYPE_INFO_STRUCT:
            // TODO: Implement struct type conversion
            return data->ptr_type;
            
        case TYPE_INFO_ENUM:
            // Enums are typically represented as integers
            return data->i32_type;
            
        case TYPE_INFO_TUPLE:
            // TODO: Implement tuple type conversion
            return data->ptr_type;
            
        default:
            return data->void_type;
    }
}

// Generate code for binary operations
static LLVMValueRef generate_binary_op(LLVMBackendData *data, const ASTNode *node) {
    LLVMValueRef left = generate_expression(data, node->data.binary_expr.left);
    LLVMValueRef right = generate_expression(data, node->data.binary_expr.right);
    
    if (!left || !right) return NULL;
    
    switch (node->data.binary_expr.operator) {
        case BINOP_ADD:
            if (LLVMGetTypeKind(LLVMTypeOf(left)) == LLVMFloatTypeKind ||
                LLVMGetTypeKind(LLVMTypeOf(left)) == LLVMDoubleTypeKind) {
                return LLVMBuildFAdd(data->builder, left, right, "add");
            } else {
                return LLVMBuildAdd(data->builder, left, right, "add");
            }
            
        case BINOP_SUB:
            if (LLVMGetTypeKind(LLVMTypeOf(left)) == LLVMFloatTypeKind ||
                LLVMGetTypeKind(LLVMTypeOf(left)) == LLVMDoubleTypeKind) {
                return LLVMBuildFSub(data->builder, left, right, "sub");
            } else {
                return LLVMBuildSub(data->builder, left, right, "sub");
            }
            
        case BINOP_MUL:
            if (LLVMGetTypeKind(LLVMTypeOf(left)) == LLVMFloatTypeKind ||
                LLVMGetTypeKind(LLVMTypeOf(left)) == LLVMDoubleTypeKind) {
                return LLVMBuildFMul(data->builder, left, right, "mul");
            } else {
                return LLVMBuildMul(data->builder, left, right, "mul");
            }
            
        case BINOP_DIV:
            if (LLVMGetTypeKind(LLVMTypeOf(left)) == LLVMFloatTypeKind ||
                LLVMGetTypeKind(LLVMTypeOf(left)) == LLVMDoubleTypeKind) {
                return LLVMBuildFDiv(data->builder, left, right, "div");
            } else {
                // TODO: Handle signed vs unsigned division
                return LLVMBuildSDiv(data->builder, left, right, "div");
            }
            
        case BINOP_MOD:
            // TODO: Handle signed vs unsigned modulo
            return LLVMBuildSRem(data->builder, left, right, "mod");
            
        case BINOP_EQ:
            if (LLVMGetTypeKind(LLVMTypeOf(left)) == LLVMFloatTypeKind ||
                LLVMGetTypeKind(LLVMTypeOf(left)) == LLVMDoubleTypeKind) {
                return LLVMBuildFCmp(data->builder, LLVMRealOEQ, left, right, "eq");
            } else {
                return LLVMBuildICmp(data->builder, LLVMIntEQ, left, right, "eq");
            }
            
        case BINOP_NE:
            if (LLVMGetTypeKind(LLVMTypeOf(left)) == LLVMFloatTypeKind ||
                LLVMGetTypeKind(LLVMTypeOf(left)) == LLVMDoubleTypeKind) {
                return LLVMBuildFCmp(data->builder, LLVMRealONE, left, right, "ne");
            } else {
                return LLVMBuildICmp(data->builder, LLVMIntNE, left, right, "ne");
            }
            
        case BINOP_LT:
            if (LLVMGetTypeKind(LLVMTypeOf(left)) == LLVMFloatTypeKind ||
                LLVMGetTypeKind(LLVMTypeOf(left)) == LLVMDoubleTypeKind) {
                return LLVMBuildFCmp(data->builder, LLVMRealOLT, left, right, "lt");
            } else {
                // TODO: Handle signed vs unsigned comparison
                return LLVMBuildICmp(data->builder, LLVMIntSLT, left, right, "lt");
            }
            
        case BINOP_GT:
            if (LLVMGetTypeKind(LLVMTypeOf(left)) == LLVMFloatTypeKind ||
                LLVMGetTypeKind(LLVMTypeOf(left)) == LLVMDoubleTypeKind) {
                return LLVMBuildFCmp(data->builder, LLVMRealOGT, left, right, "gt");
            } else {
                // TODO: Handle signed vs unsigned comparison
                return LLVMBuildICmp(data->builder, LLVMIntSGT, left, right, "gt");
            }
            
        case BINOP_LE:
            if (LLVMGetTypeKind(LLVMTypeOf(left)) == LLVMFloatTypeKind ||
                LLVMGetTypeKind(LLVMTypeOf(left)) == LLVMDoubleTypeKind) {
                return LLVMBuildFCmp(data->builder, LLVMRealOLE, left, right, "le");
            } else {
                // TODO: Handle signed vs unsigned comparison
                return LLVMBuildICmp(data->builder, LLVMIntSLE, left, right, "le");
            }
            
        case BINOP_GE:
            if (LLVMGetTypeKind(LLVMTypeOf(left)) == LLVMFloatTypeKind ||
                LLVMGetTypeKind(LLVMTypeOf(left)) == LLVMDoubleTypeKind) {
                return LLVMBuildFCmp(data->builder, LLVMRealOGE, left, right, "ge");
            } else {
                // TODO: Handle signed vs unsigned comparison
                return LLVMBuildICmp(data->builder, LLVMIntSGE, left, right, "ge");
            }
            
        case BINOP_AND:
            return LLVMBuildAnd(data->builder, left, right, "and");
            
        case BINOP_OR:
            return LLVMBuildOr(data->builder, left, right, "or");
            
        default:
            return NULL;
    }
}

// Generate code for identifiers (variable/function references)
static LLVMValueRef generate_identifier(LLVMBackendData *data, const ASTNode *node) {
    const char *name = node->data.identifier.name;
    
    // First check local variables in current function
    if (data->current_function) {
        // Search through function parameters
        LLVMTypeRef fn_type = LLVMGetElementType(LLVMTypeOf(data->current_function));
        unsigned param_count = LLVMCountParamTypes(fn_type);
        
        for (unsigned i = 0; i < param_count; i++) {
            LLVMValueRef param = LLVMGetParam(data->current_function, i);
            const char *param_name = LLVMGetValueName(param);
            if (param_name && strcmp(param_name, name) == 0) {
                return param;
            }
        }
        
        // TODO: Search through local variables once we implement variable declarations
    }
    
    // Check for builtin functions and map to runtime functions
    if (strcmp(name, "log") == 0) {
        // Get asthra_simple_log function
        LLVMValueRef log_fn = LLVMGetNamedFunction(data->module, "asthra_simple_log");
        if (!log_fn) {
            // Declare asthra_simple_log function: void asthra_simple_log(const char* message)
            LLVMTypeRef param_types[] = {data->ptr_type};
            LLVMTypeRef fn_type = LLVMFunctionType(data->void_type, param_types, 1, false);
            log_fn = LLVMAddFunction(data->module, "asthra_simple_log", fn_type);
            LLVMSetLinkage(log_fn, LLVMExternalLinkage);
        }
        return log_fn;
    } else if (strcmp(name, "panic") == 0) {
        return data->runtime_panic_fn;
    } else if (strcmp(name, "args") == 0) {
        // args() function should be declared as asthra_runtime_get_args
        LLVMValueRef args_fn = LLVMGetNamedFunction(data->module, "asthra_runtime_get_args");
        if (!args_fn) {
            // Declare asthra_runtime_get_args function: []string asthra_runtime_get_args()
            LLVMTypeRef slice_fields[2] = {data->ptr_type, data->i64_type};
            LLVMTypeRef slice_type = LLVMStructTypeInContext(data->context, slice_fields, 2, 0);
            LLVMTypeRef fn_type = LLVMFunctionType(slice_type, NULL, 0, false);
            args_fn = LLVMAddFunction(data->module, "asthra_runtime_get_args", fn_type);
            LLVMSetLinkage(args_fn, LLVMExternalLinkage);
        }
        return args_fn;
    }
    
    // Check global functions
    LLVMValueRef global_fn = LLVMGetNamedFunction(data->module, name);
    if (global_fn) {
        return global_fn;
    }
    
    // Check global variables
    LLVMValueRef global_var = LLVMGetNamedGlobal(data->module, name);
    if (global_var) {
        // Load the value from global variable
        return LLVMBuildLoad2(data->builder, LLVMGlobalGetValueType(global_var), global_var, name);
    }
    
    // Not found
    return NULL;
}

// Generate code for unary operations
static LLVMValueRef generate_unary_op(LLVMBackendData *data, const ASTNode *node) {
    LLVMValueRef operand = generate_expression(data, node->data.unary_expr.operand);
    if (!operand) return NULL;
    
    switch (node->data.unary_expr.operator) {
        case UNOP_MINUS:
            if (LLVMGetTypeKind(LLVMTypeOf(operand)) == LLVMFloatTypeKind ||
                LLVMGetTypeKind(LLVMTypeOf(operand)) == LLVMDoubleTypeKind) {
                return LLVMBuildFNeg(data->builder, operand, "neg");
            } else {
                return LLVMBuildNeg(data->builder, operand, "neg");
            }
            
        case UNOP_NOT:
            // Logical NOT - convert to boolean first if needed
            if (LLVMGetTypeKind(LLVMTypeOf(operand)) != LLVMIntegerTypeKind ||
                LLVMGetIntTypeWidth(LLVMTypeOf(operand)) != 1) {
                // Convert to bool by comparing with zero
                operand = LLVMBuildICmp(data->builder, LLVMIntNE, operand,
                                        LLVMConstNull(LLVMTypeOf(operand)), "tobool");
            }
            return LLVMBuildNot(data->builder, operand, "not");
            
        case UNOP_BITWISE_NOT:
            return LLVMBuildNot(data->builder, operand, "bitnot");
            
        case UNOP_DEREF:
            {
                // Dereference pointer
                // Get the pointee type from the pointer
                LLVMTypeRef ptr_type = LLVMTypeOf(operand);
                LLVMTypeRef elem_type = LLVMGetElementType(ptr_type);
                return LLVMBuildLoad2(data->builder, elem_type, operand, "deref");
            }
            
        case UNOP_ADDRESS_OF:
            // Take address - operand must be an lvalue
            // TODO: Properly handle lvalue checking
            return operand; // For now, assume operand is already an address
            
        case UNOP_SIZEOF:
            // sizeof operator - return size of type
            {
                LLVMTypeRef type = LLVMTypeOf(operand);
                LLVMValueRef size = LLVMSizeOf(type);
                // Convert to i64 for consistency
                return LLVMBuildZExt(data->builder, size, data->i64_type, "sizeof");
            }
            
        default:
            return NULL;
    }
}

// Generate code for function calls
static LLVMValueRef generate_call_expr(LLVMBackendData *data, const ASTNode *node) {
    // Get the function to call
    LLVMValueRef function = generate_expression(data, node->data.call_expr.function);
    if (!function) return NULL;
    
    // Check if this is a special builtin function call that needs parameter transformation
    const char *function_name = NULL;
    if (node->data.call_expr.function->type == AST_IDENTIFIER) {
        function_name = node->data.call_expr.function->data.identifier.name;
    }
    
    // Generate arguments
    size_t arg_count = node->data.call_expr.args ? node->data.call_expr.args->count : 0;
    LLVMValueRef *args = NULL;
    size_t actual_arg_count = arg_count;
    
    // Regular function call - no special handling needed
    if (arg_count > 0) {
        args = malloc(arg_count * sizeof(LLVMValueRef));
        for (size_t i = 0; i < arg_count; i++) {
            args[i] = generate_expression(data, node->data.call_expr.args->nodes[i]);
            if (!args[i]) {
                free(args);
                return NULL;
            }
        }
    }
    
    // Build the call
    // Get function type from the global function
    LLVMTypeRef fn_type = LLVMGlobalGetValueType(function);
    if (!fn_type) {
        // Fallback: try to get it from the function pointer type
        LLVMTypeRef fn_ptr_type = LLVMTypeOf(function);
        if (LLVMGetTypeKind(fn_ptr_type) == LLVMPointerTypeKind) {
            fn_type = LLVMGetElementType(fn_ptr_type);
        }
    }
    
    if (!fn_type) {
        if (args) free(args);
        return NULL;
    }
    
    LLVMValueRef result = LLVMBuildCall2(data->builder, fn_type, function, args, (unsigned)actual_arg_count, "call");
    
    if (args) free(args);
    return result;
}

// Generate code for array/slice indexing
static LLVMValueRef generate_index_expr(LLVMBackendData *data, const ASTNode *node) {
    LLVMValueRef array = generate_expression(data, node->data.index_access.array);
    LLVMValueRef index = generate_expression(data, node->data.index_access.index);
    
    if (!array || !index) return NULL;
    
    // Get element pointer
    LLVMValueRef indices[2] = {
        LLVMConstInt(data->i64_type, 0, false), // First index for array decay
        index                                     // Actual index
    };
    
    // Get array type for GEP
    LLVMTypeRef array_type = LLVMTypeOf(array);
    LLVMValueRef element_ptr = LLVMBuildGEP2(data->builder, array_type, array, indices, 2, "elemptr");
    
    // Load the value - need to get element type from array
    LLVMTypeRef elem_type = LLVMGetElementType(LLVMGetElementType(array_type));
    return LLVMBuildLoad2(data->builder, elem_type, element_ptr, "elem");
}

// Generate code for field access
static LLVMValueRef generate_field_access(LLVMBackendData *data, const ASTNode *node) {
    LLVMValueRef object = generate_expression(data, node->data.field_access.object);
    if (!object) return NULL;
    
    const char *field_name = node->data.field_access.field_name;
    
    // TODO: Look up field index from struct type info
    // For now, we'll need to integrate with the type system
    
    // Placeholder: assume field index 0
    LLVMValueRef indices[2] = {
        LLVMConstInt(data->i32_type, 0, false),
        LLVMConstInt(data->i32_type, 0, false) // TODO: Get actual field index
    };
    
    LLVMTypeRef object_type = LLVMTypeOf(object);
    LLVMValueRef field_ptr = LLVMBuildGEP2(data->builder, object_type, object, indices, 2, field_name);
    // TODO: Get actual field type from struct info
    LLVMTypeRef field_type = data->i32_type; // Placeholder
    return LLVMBuildLoad2(data->builder, field_type, field_ptr, field_name);
}

// Generate code for array literals
static LLVMValueRef generate_array_literal(LLVMBackendData *data, const ASTNode *node) {
    size_t elem_count = node->data.array_literal.elements ? 
                        node->data.array_literal.elements->count : 0;
    
    if (elem_count == 0) {
        // Empty array
        return LLVMConstArray(data->i32_type, NULL, 0);
    }
    
    // Generate element values
    LLVMValueRef *elements = malloc(elem_count * sizeof(LLVMValueRef));
    LLVMTypeRef elem_type = NULL;
    
    for (size_t i = 0; i < elem_count; i++) {
        elements[i] = generate_expression(data, node->data.array_literal.elements->nodes[i]);
        if (!elements[i]) {
            free(elements);
            return NULL;
        }
        
        // Get element type from first element
        if (i == 0) {
            elem_type = LLVMTypeOf(elements[i]);
        }
    }
    
    // Create array constant
    LLVMValueRef array = LLVMConstArray(elem_type, elements, (unsigned)elem_count);
    free(elements);
    
    return array;
}

// Generate code for expressions
static LLVMValueRef generate_expression(LLVMBackendData *data, const ASTNode *node) {
    if (!node) return NULL;
    
    // Set debug location for this expression
    set_debug_location(data, node);
    
    switch (node->type) {
        // Literals
        case AST_INTEGER_LITERAL:
            {
                // Use type information from semantic analysis if available
                LLVMTypeRef int_type = data->i32_type; // default
                if (node->type_info && node->type_info->category == TYPE_INFO_PRIMITIVE) {
                    switch (node->type_info->data.primitive.kind) {
                        case PRIMITIVE_INFO_I8:
                            int_type = LLVMInt8Type();
                            break;
                        case PRIMITIVE_INFO_I16:
                            int_type = LLVMInt16Type();
                            break;
                        case PRIMITIVE_INFO_I32:
                            int_type = data->i32_type;
                            break;
                        case PRIMITIVE_INFO_I64:
                            int_type = data->i64_type;
                            break;
                        default:
                            int_type = data->i32_type;
                            break;
                    }
                }
                return LLVMConstInt(int_type, node->data.integer_literal.value, false);
            }
            
        case AST_FLOAT_LITERAL:
            return LLVMConstReal(data->f64_type, node->data.float_literal.value);
            
        case AST_STRING_LITERAL:
            {
                // Create global string constant
                LLVMValueRef str = LLVMBuildGlobalStringPtr(data->builder, 
                    node->data.string_literal.value, ".str");
                return str;
            }
            
        case AST_BOOL_LITERAL:
        case AST_BOOLEAN_LITERAL:
            return LLVMConstInt(data->bool_type, node->data.bool_literal.value ? 1 : 0, false);
            
        case AST_CHAR_LITERAL:
            return LLVMConstInt(LLVMInt32TypeInContext(data->context), 
                                node->data.char_literal.value, false);
            
        case AST_UNIT_LITERAL:
            // Unit value is an empty struct
            return LLVMConstNamedStruct(data->unit_type, NULL, 0);
            
        // Binary operations
        case AST_BINARY_EXPR:
            return generate_binary_op(data, node);
            
        // Other expressions
        case AST_IDENTIFIER:
            return generate_identifier(data, node);
            
        case AST_UNARY_EXPR:
            return generate_unary_op(data, node);
            
        case AST_CALL_EXPR:
            return generate_call_expr(data, node);
            
        case AST_INDEX_ACCESS:
            return generate_index_expr(data, node);
            
        case AST_FIELD_ACCESS:
            return generate_field_access(data, node);
            
        case AST_ARRAY_LITERAL:
            return generate_array_literal(data, node);
            
        // TODO: Implement remaining expression types
        // case AST_CAST_EXPR:
        case AST_STRUCT_LITERAL:
        case AST_TUPLE_LITERAL:
        case AST_SLICE_EXPR:
        case AST_ASSOCIATED_FUNC_CALL:
            // Not yet implemented
            return NULL;
            
        default:
            return NULL;
    }
}

// Generate code for statements
static void generate_statement(LLVMBackendData *data, const ASTNode *node) {
    if (!node) return;
    
    // Set debug location for this statement
    set_debug_location(data, node);
    
    switch (node->type) {
        case AST_RETURN_STMT:
            {
                LLVMValueRef ret_val = NULL;
                if (node->data.return_stmt.expression) {
                    ret_val = generate_expression(data, node->data.return_stmt.expression);
                }
                
                if (ret_val) {
                    LLVMBuildRet(data->builder, ret_val);
                } else {
                    LLVMBuildRetVoid(data->builder);
                }
            }
            break;
            
        case AST_LET_STMT:  // Variable declaration
            {
                const char *var_name = node->data.let_stmt.name;
                LLVMTypeRef var_type = data->i32_type; // Default type
                
                // Get type from type_info if available
                if (node->type_info) {
                    var_type = asthra_type_to_llvm(data, node->type_info);
                } else if (node->data.let_stmt.type && node->data.let_stmt.type->type_info) {
                    var_type = asthra_type_to_llvm(data, node->data.let_stmt.type->type_info);
                }
                
                // Allocate space on stack
                LLVMValueRef alloca = LLVMBuildAlloca(data->builder, var_type, var_name);
                
                // Initialize if there's an initializer
                if (node->data.let_stmt.initializer) {
                    LLVMValueRef init_val = generate_expression(data, node->data.let_stmt.initializer);
                    if (init_val) {
                        LLVMBuildStore(data->builder, init_val, alloca);
                    }
                }
                
                // Generate debug info for local variable
                if (data->di_builder && data->current_debug_scope && node->location.line > 0) {
                    LLVMMetadataRef di_var_type = asthra_type_to_debug_type(data, 
                        node->type_info ? node->type_info : 
                        (node->data.let_stmt.type ? node->data.let_stmt.type->type_info : NULL));
                    
                    if (di_var_type) {
                        LLVMMetadataRef di_var = LLVMDIBuilderCreateAutoVariable(
                            data->di_builder,
                            data->current_debug_scope,
                            var_name,
                            strlen(var_name),
                            data->di_file,
                            node->location.line,
                            di_var_type,
                            true,  // preserve through opts
                            LLVMDIFlagZero,
                            0  // align
                        );
                        
                        // Create debug declare
                        LLVMMetadataRef expr = LLVMDIBuilderCreateExpression(data->di_builder, NULL, 0);
                        LLVMMetadataRef loc = LLVMDIBuilderCreateDebugLocation(
                            data->context,
                            node->location.line,
                            node->location.column,
                            data->current_debug_scope,
                            NULL
                        );
                        
                        // Insert declare instruction for debug info
                        // TODO: Update to use LLVMDIBuilderInsertDeclareRecordAtEnd for LLVM 20+
                        // LLVMValueRef dbg_declare = LLVMDIBuilderInsertDeclareAtEnd(
                        //     data->di_builder,
                        //     alloca,
                        //     di_var,
                        //     expr,
                        //     loc,
                        //     LLVMGetInsertBlock(data->builder)
                        // );
                    }
                }
                
                // TODO: Store variable in symbol table for later lookup
                // For now, we'll need to enhance identifier lookup to find local variables
            }
            break;
            
        case AST_ASSIGNMENT:
            {
                // Generate lvalue address
                LLVMValueRef target = NULL;
                
                if (node->data.assignment.target->type == AST_IDENTIFIER) {
                    // Simple variable assignment
                    const char *var_name = node->data.assignment.target->data.identifier.name;
                    // TODO: Look up variable address from symbol table
                    target = LLVMGetNamedGlobal(data->module, var_name);
                } else if (node->data.assignment.target->type == AST_FIELD_ACCESS) {
                    // Field assignment
                    target = generate_expression(data, node->data.assignment.target);
                } else if (node->data.assignment.target->type == AST_INDEX_ACCESS) {
                    // Array element assignment
                    LLVMValueRef array = generate_expression(data, 
                        node->data.assignment.target->data.index_access.array);
                    LLVMValueRef index = generate_expression(data, 
                        node->data.assignment.target->data.index_access.index);
                    
                    if (array && index) {
                        LLVMValueRef indices[2] = {
                            LLVMConstInt(data->i64_type, 0, false),
                            index
                        };
                        LLVMTypeRef array_type = LLVMTypeOf(array);
                        target = LLVMBuildGEP2(data->builder, array_type, array, indices, 2, "elemptr");
                    }
                }
                
                // Generate rvalue
                if (target) {
                    LLVMValueRef value = generate_expression(data, node->data.assignment.value);
                    if (value) {
                        LLVMBuildStore(data->builder, value, target);
                    }
                }
            }
            break;
            
        case AST_IF_STMT:
            {
                // Generate condition
                LLVMValueRef cond = generate_expression(data, node->data.if_stmt.condition);
                if (!cond) break;
                
                // Convert to boolean if needed
                if (LLVMGetTypeKind(LLVMTypeOf(cond)) != LLVMIntegerTypeKind ||
                    LLVMGetIntTypeWidth(LLVMTypeOf(cond)) != 1) {
                    cond = LLVMBuildICmp(data->builder, LLVMIntNE, cond,
                                         LLVMConstNull(LLVMTypeOf(cond)), "ifcond");
                }
                
                // Create basic blocks
                LLVMValueRef function = data->current_function;
                LLVMBasicBlockRef then_bb = LLVMAppendBasicBlockInContext(
                    data->context, function, "then");
                LLVMBasicBlockRef else_bb = NULL;
                LLVMBasicBlockRef merge_bb = LLVMAppendBasicBlockInContext(
                    data->context, function, "ifcont");
                
                if (node->data.if_stmt.else_block) {
                    else_bb = LLVMAppendBasicBlockInContext(
                        data->context, function, "else");
                    LLVMBuildCondBr(data->builder, cond, then_bb, else_bb);
                } else {
                    LLVMBuildCondBr(data->builder, cond, then_bb, merge_bb);
                }
                
                // Generate then block
                LLVMPositionBuilderAtEnd(data->builder, then_bb);
                generate_statement(data, node->data.if_stmt.then_block);
                if (!LLVMGetBasicBlockTerminator(LLVMGetInsertBlock(data->builder))) {
                    LLVMBuildBr(data->builder, merge_bb);
                }
                
                // Generate else block if present
                if (else_bb) {
                    LLVMPositionBuilderAtEnd(data->builder, else_bb);
                    generate_statement(data, node->data.if_stmt.else_block);
                    if (!LLVMGetBasicBlockTerminator(LLVMGetInsertBlock(data->builder))) {
                        LLVMBuildBr(data->builder, merge_bb);
                    }
                }
                
                // Continue with merge block
                LLVMPositionBuilderAtEnd(data->builder, merge_bb);
            }
            break;
            
        case AST_FOR_STMT:
            {
                // Asthra for loops iterate over collections/ranges
                // for i in range(10) { ... }
                
                // Create basic blocks
                LLVMValueRef function = data->current_function;
                LLVMBasicBlockRef loop_bb = LLVMAppendBasicBlockInContext(
                    data->context, function, "loop");
                LLVMBasicBlockRef loop_body_bb = LLVMAppendBasicBlockInContext(
                    data->context, function, "loop_body");
                LLVMBasicBlockRef loop_end_bb = LLVMAppendBasicBlockInContext(
                    data->context, function, "loop_end");
                
                // TODO: Handle iterator initialization based on iterable type
                // For now, assume numeric range starting at 0
                
                // Create loop variable
                LLVMTypeRef loop_var_type = data->i32_type;
                LLVMValueRef loop_var = LLVMBuildAlloca(data->builder, loop_var_type, 
                                                        node->data.for_stmt.variable);
                LLVMBuildStore(data->builder, LLVMConstInt(loop_var_type, 0, false), loop_var);
                
                // Jump to loop header
                LLVMBuildBr(data->builder, loop_bb);
                
                // Loop header - check condition
                LLVMPositionBuilderAtEnd(data->builder, loop_bb);
                LLVMValueRef current_val = LLVMBuildLoad2(data->builder, loop_var_type, loop_var, "loop_var");
                
                // TODO: Get actual loop bound from iterable
                LLVMValueRef loop_bound = LLVMConstInt(loop_var_type, 10, false);
                LLVMValueRef cond = LLVMBuildICmp(data->builder, LLVMIntSLT, 
                                                   current_val, loop_bound, "loopcond");
                LLVMBuildCondBr(data->builder, cond, loop_body_bb, loop_end_bb);
                
                // Loop body
                LLVMPositionBuilderAtEnd(data->builder, loop_body_bb);
                generate_statement(data, node->data.for_stmt.body);
                
                // Increment loop variable
                LLVMValueRef inc_val = LLVMBuildAdd(data->builder, current_val,
                                                     LLVMConstInt(loop_var_type, 1, false), "inc");
                LLVMBuildStore(data->builder, inc_val, loop_var);
                LLVMBuildBr(data->builder, loop_bb);
                
                // Continue after loop
                LLVMPositionBuilderAtEnd(data->builder, loop_end_bb);
            }
            break;
            
        case AST_BLOCK:
            {
                // Create lexical scope for debug info
                LLVMMetadataRef old_scope = data->current_debug_scope;
                if (data->di_builder && data->current_debug_scope && node->location.line > 0) {
                    data->current_debug_scope = LLVMDIBuilderCreateLexicalBlock(
                        data->di_builder,
                        data->current_debug_scope,
                        data->di_file,
                        node->location.line,
                        node->location.column
                    );
                }
                
                // Generate all statements in the block
                if (node->data.block.statements) {
                    for (size_t i = 0; i < node->data.block.statements->count; i++) {
                        generate_statement(data, node->data.block.statements->nodes[i]);
                    }
                }
                
                // Restore previous scope
                data->current_debug_scope = old_scope;
            }
            break;
            
        case AST_EXPR_STMT:
            {
                // Expression statement - evaluate for side effects
                generate_expression(data, node->data.expr_stmt.expression);
            }
            break;
            
        case AST_BREAK_STMT:
            // TODO: Implement break statement (need loop context)
            break;
            
        case AST_CONTINUE_STMT:
            // TODO: Implement continue statement (need loop context)
            break;
            
        case AST_MATCH_STMT:
            // TODO: Implement pattern matching
            break;
            
        default:
            break;
    }
}

// Generate code for functions
static void generate_function(LLVMBackendData *data, const ASTNode *node) {
    if (!node || node->type != AST_FUNCTION_DECL) return;
    
    // Get function type from type_info
    TypeInfo *func_type = node->type_info;
    
    if (!func_type || func_type->category != TYPE_INFO_FUNCTION) {
        return;
    }
    
    // Handle function without proper type info (e.g., during early compilation)
    LLVMTypeRef ret_type = data->void_type;
    
    if (func_type->category == TYPE_INFO_FUNCTION && func_type->data.function.return_type) {
        ret_type = asthra_type_to_llvm(data, func_type->data.function.return_type);
    } else {
        // Fallback to void if no proper return type info
        ret_type = data->void_type;
    }
    
    // Convert parameter types
    int param_count = node->data.function_decl.params ? node->data.function_decl.params->count : 0;
    LLVMTypeRef *param_types = NULL;
    if (param_count > 0) {
        param_types = malloc(param_count * sizeof(LLVMTypeRef));
        for (int i = 0; i < param_count; i++) {
            ASTNode *param = node->data.function_decl.params->nodes[i];
            if (param && param->type == AST_PARAM_DECL) {
                param_types[i] = asthra_type_to_llvm(data, param->type_info);
            } else {
                param_types[i] = data->void_type;
            }
        }
    }
    
    // Create function type
    LLVMTypeRef fn_type = LLVMFunctionType(ret_type, param_types, param_count, false);
    
    // Create function
    LLVMValueRef function = LLVMAddFunction(data->module, node->data.function_decl.name, fn_type);
    
    // Set parameter names
    for (int i = 0; i < param_count; i++) {
        LLVMValueRef param = LLVMGetParam(function, i);
        ASTNode *param_node = node->data.function_decl.params->nodes[i];
        if (param_node && param_node->type == AST_PARAM_DECL) {
            LLVMSetValueName2(param, param_node->data.param_decl.name, 
                              strlen(param_node->data.param_decl.name));
        }
    }
    
    // Generate debug info for function if enabled
    if (data->di_builder && node->location.line > 0) {
        // Create function debug type
        LLVMMetadataRef fn_di_type = NULL;
        if (func_type && func_type->category == TYPE_INFO_FUNCTION) {
            fn_di_type = asthra_type_to_debug_type(data, func_type);
        }
        
        // Create function debug info
        LLVMMetadataRef di_function = LLVMDIBuilderCreateFunction(
            data->di_builder,
            data->di_file,  // scope
            node->data.function_decl.name,
            strlen(node->data.function_decl.name),
            node->data.function_decl.name,  // linkage name
            strlen(node->data.function_decl.name),
            data->di_file,
            node->location.line,
            fn_di_type ? fn_di_type : data->di_void_type,
            false,  // is local
            true,   // is definition
            node->location.line,
            LLVMDIFlagPrototyped,
            false   // is optimized
        );
        
        // Attach debug info to function
        LLVMSetSubprogram(function, di_function);
        data->current_debug_scope = di_function;
    }
    
    // Generate function body if present
    if (node->data.function_decl.body) {
        // Create entry basic block
        LLVMBasicBlockRef entry = LLVMAppendBasicBlockInContext(data->context, function, "entry");
        LLVMPositionBuilderAtEnd(data->builder, entry);
        
        // Set current function
        data->current_function = function;
        
        // Generate body statements
        LLVMValueRef last_value = NULL;
        if (node->data.function_decl.body->type == AST_BLOCK) {
            ASTNodeList *statements = node->data.function_decl.body->data.block.statements;
            for (size_t i = 0; i < statements->count; i++) {
                ASTNode *stmt = statements->nodes[i];
                
                // Check if this is the last statement and it's an expression statement
                if (i == statements->count - 1 && stmt->type == AST_EXPR_STMT) {
                    // For the last expression, generate it as a value (potential return)
                    last_value = generate_expression(data, stmt->data.expr_stmt.expression);
                } else {
                    // Generate as regular statement
                    generate_statement(data, stmt);
                }
            }
        }
        
        // Add implicit return if needed
        if (!LLVMGetBasicBlockTerminator(LLVMGetInsertBlock(data->builder))) {
            if (ret_type == data->void_type) {
                LLVMBuildRetVoid(data->builder);
            } else if (ret_type == data->unit_type) {
                LLVMValueRef unit = LLVMConstNamedStruct(data->unit_type, NULL, 0);
                LLVMBuildRet(data->builder, unit);
            } else if (last_value) {
                // Return the last expression value (with type coercion if needed)
                LLVMTypeRef last_value_type = LLVMTypeOf(last_value);
                if (last_value_type != ret_type) {
                    // Coerce integer types
                    if (LLVMGetTypeKind(last_value_type) == LLVMIntegerTypeKind && 
                        LLVMGetTypeKind(ret_type) == LLVMIntegerTypeKind) {
                        unsigned src_bits = LLVMGetIntTypeWidth(last_value_type);
                        unsigned dst_bits = LLVMGetIntTypeWidth(ret_type);
                        if (src_bits < dst_bits) {
                            // Sign extend for larger type
                            last_value = LLVMBuildSExt(data->builder, last_value, ret_type, "sext");
                        } else if (src_bits > dst_bits) {
                            // Truncate for smaller type
                            last_value = LLVMBuildTrunc(data->builder, last_value, ret_type, "trunc");
                        }
                    }
                }
                LLVMBuildRet(data->builder, last_value);
            } else {
                // No value to return, use default
                if (ret_type == data->i32_type) {
                    LLVMBuildRet(data->builder, LLVMConstInt(data->i32_type, 0, false));
                } else {
                    LLVMBuildRetVoid(data->builder);
                }
            }
        }
        
        // Verify function
        char *error = NULL;
        if (LLVMVerifyFunction(function, LLVMReturnStatusAction)) {
            // Function verification failed - this should not happen with our corrected implementation
            fprintf(stderr, "LLVM function verification failed for %s\n", node->data.function_decl.name);
            LLVMDeleteFunction(function);
        }
    }
    
    if (param_types) free(param_types);
}

// Generate code for top-level declarations
static void generate_top_level(LLVMBackendData *data, const ASTNode *node) {
    if (!node) return;
    
    switch (node->type) {
        case AST_FUNCTION_DECL:
            generate_function(data, node);
            // TODO: Update statistics when backend pointer is available
            break;
            
        // TODO: Implement other top-level declarations
        case AST_STRUCT_DECL:
        case AST_ENUM_DECL:
        case AST_CONST_DECL:
            // Not yet implemented
            break;
            
        default:
            break;
    }
}

#endif // ASTHRA_ENABLE_LLVM_BACKEND

// Generate LLVM IR from AST
static int llvm_backend_generate(AsthraBackend *backend, 
                                AsthraCompilerContext *ctx,
                                const ASTNode *ast,
                                const char *output_file) {
#ifndef ASTHRA_ENABLE_LLVM_BACKEND
    backend->last_error = "LLVM backend support not compiled in";
    return -1;
#else
    if (!backend || !backend->private_data || !ctx || !ast || !output_file) {
        backend->last_error = "Invalid parameters for LLVM code generation";
        return -1;
    }
    
    LLVMBackendData *data = (LLVMBackendData*)backend->private_data;
    data->compiler_ctx = ctx;
    data->output_filename = strdup(output_file);
    
    // Start timing
    clock_t start_time = clock();
    
    // Initialize debug info if requested
    if (ctx->options.debug_info) {
        const char *source_filename = ctx->options.input_file ? ctx->options.input_file : output_file;
        bool is_optimized = ctx->options.opt_level > ASTHRA_OPT_NONE;
        initialize_debug_info(data, source_filename, is_optimized);
        cache_basic_debug_types(data);
    }
    
    // Generate code for the AST
    if (ast->type == AST_PROGRAM) {
        // Process all top-level declarations
        for (size_t i = 0; i < ast->data.program.declarations->count; i++) {
            generate_top_level(data, ast->data.program.declarations->nodes[i]);
            // backend->stats.functions_generated++;
        }
    } else {
        // Single top-level declaration
        generate_top_level(data, ast);
        // backend->stats.functions_generated++;
    }
    
    // Finalize debug info if enabled
    if (data->di_builder) {
        finalize_debug_info(data);
    }
    
    // Verify the module
    char *error = NULL;
    if (LLVMVerifyModule(data->module, LLVMReturnStatusAction, &error)) {
        backend->last_error = error ? error : "Module verification failed";
        if (error) LLVMDisposeMessage(error);
        return -1;
    }
    
    // Write output based on file extension
    int result = 0;
    const char *ext = strrchr(output_file, '.');
    
    if (ext && strcmp(ext, ".bc") == 0) {
        // Write bitcode
        if (LLVMWriteBitcodeToFile(data->module, output_file) != 0) {
            backend->last_error = "Failed to write LLVM bitcode";
            result = -1;
        }
    } else {
        // Write text IR (default)
        char *ir_string = LLVMPrintModuleToString(data->module);
        if (!ir_string) {
            backend->last_error = "Failed to generate LLVM IR string";
            result = -1;
        } else {
            FILE *fp = fopen(output_file, "w");
            if (!fp) {
                backend->last_error = "Failed to open output file";
                result = -1;
            } else {
                fprintf(fp, "%s", ir_string);
                fclose(fp);
                
                // Update statistics
                backend->stats.lines_generated = 1; // Count module as 1 unit for now
                for (char *p = ir_string; *p; p++) {
                    if (*p == '\n') backend->stats.lines_generated++;
                }
            }
            LLVMDisposeMessage(ir_string);
        }
    }
    
    // Update timing
    clock_t end_time = clock();
    backend->stats.generation_time = ((double)(end_time - start_time) / CLOCKS_PER_SEC) * 1000.0;
    
    return result;
#endif
}

// Optimize LLVM IR
static int llvm_backend_optimize(AsthraBackend *backend, void *ir, int opt_level) {
    backend->last_error = "LLVM backend optimization is not yet implemented";
    return -1;
    
    // Future implementation will:
    // 1. Create optimization pass manager
    // 2. Add appropriate optimization passes based on level
    // 3. Run passes on the module
}

// Cleanup LLVM backend
static void llvm_backend_cleanup(AsthraBackend *backend) {
    if (!backend || !backend->private_data) return;
    
    LLVMBackendData *data = (LLVMBackendData*)backend->private_data;
    
#ifdef ASTHRA_ENABLE_LLVM_BACKEND
    // Dispose of LLVM resources in reverse order
    if (data->di_builder) {
        LLVMDisposeDIBuilder(data->di_builder);
    }
    
    if (data->builder) {
        LLVMDisposeBuilder(data->builder);
    }
    
    if (data->module) {
        LLVMDisposeModule(data->module);
    }
    
    if (data->context) {
        LLVMContextDispose(data->context);
    }
#endif
    
    if (data->output_filename) {
        free(data->output_filename);
    }
    
    free(data);
    backend->private_data = NULL;
}

// Check if LLVM backend supports a feature
static bool llvm_backend_supports_feature(AsthraBackend *backend, const char *feature) {
#ifndef ASTHRA_ENABLE_LLVM_BACKEND
    return false;
#else
    if (!feature) return false;
    
    // List of supported features
    const char *supported[] = {
        "optimization",
        "debug-info",
        "cross-compilation",
        "bitcode-output",
        "ir-output",
        "native-codegen",
        NULL
    };
    
    for (int i = 0; supported[i]; i++) {
        if (strcmp(feature, supported[i]) == 0) {
            return true;
        }
    }
    
    return false;
#endif
}

// Get backend version
static const char* llvm_backend_get_version(AsthraBackend *backend) {
#ifdef ASTHRA_ENABLE_LLVM_BACKEND
    static char version[128];
    snprintf(version, sizeof(version), "1.0.0 (LLVM %d.%d.%d)", 
             LLVM_VERSION_MAJOR, LLVM_VERSION_MINOR, LLVM_VERSION_PATCH);
    return version;
#else
    return "0.0.1-stub";
#endif
}

// Get backend name
static const char* llvm_backend_get_name(AsthraBackend *backend) {
#ifdef ASTHRA_ENABLE_LLVM_BACKEND
    return "Asthra LLVM IR Generator Backend";
#else
    return "Asthra LLVM IR Generator Backend (Not Compiled)";
#endif
}

// LLVM Backend operations structure
const AsthraBackendOps llvm_backend_ops = {
    .initialize = llvm_backend_initialize,
    .generate = llvm_backend_generate,
    .optimize = llvm_backend_optimize,
    .cleanup = llvm_backend_cleanup,
    .supports_feature = llvm_backend_supports_feature,
    .get_version = llvm_backend_get_version,
    .get_name = llvm_backend_get_name
};

/*
 * IMPLEMENTATION NOTES FOR FUTURE LLVM BACKEND:
 * 
 * 1. Dependencies:
 *    - LLVM-C API headers and libraries
 *    - Update CMakeLists.txt to find and link LLVM
 * 
 * 2. Key Components Needed:
 *    - AST to LLVM IR translation
 *    - Type mapping (Asthra types to LLVM types)
 *    - Function and variable management
 *    - Memory management integration
 *    - Debug information generation
 * 
 * 3. Example LLVM-C API usage:
 *    ```c
 *    LLVMContextRef context = LLVMContextCreate();
 *    LLVMModuleRef module = LLVMModuleCreateWithNameInContext("asthra_module", context);
 *    LLVMBuilderRef builder = LLVMCreateBuilderInContext(context);
 *    
 *    // Generate functions, types, instructions...
 *    
 *    LLVMWriteBitcodeToFile(module, output_file);
 *    ```
 * 
 * 4. Integration Points:
 *    - Symbol table integration for function/variable lookup
 *    - Type system integration for proper LLVM type generation
 *    - Error reporting through compiler context
 * 
 * 5. Testing Strategy:
 *    - Unit tests for IR generation
 *    - Integration tests comparing with C backend output
 *    - Validation using LLVM tools (llc, opt, lli)
 */