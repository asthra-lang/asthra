# DWARF Debug Information Generation Plan for Asthra LLVM Backend

## Overview
DWARF is the standard debugging format used by LLVM to provide source-level debugging capabilities. This plan outlines how to add comprehensive debug information to the Asthra LLVM backend.

## Goals
- Enable source-level debugging in GDB/LLDB
- Provide accurate line number mapping
- Support variable inspection during debugging
- Enable stepping through Asthra code
- Maintain performance with minimal overhead

## Technical Requirements

### 1. LLVM Debug Info API Components
```c
// Required LLVM-C debug info functions
LLVMDIBuilderRef - Debug info builder
LLVMMetadataRef - Metadata nodes for debug info
LLVMDIBuilderCreateCompileUnit() - Compilation unit info
LLVMDIBuilderCreateFile() - Source file info
LLVMDIBuilderCreateFunction() - Function debug info
LLVMDIBuilderCreateLocalVariable() - Variable debug info
LLVMDIBuilderCreateDebugLocation() - Line/column info
LLVMSetCurrentDebugLocation() - Attach debug info to instructions
```

### 2. Required Information from AST
- **Source Location**: file, line, column for each AST node
- **Type Information**: Accurate type metadata for variables
- **Scope Information**: Lexical scopes for variables
- **Function Information**: Names, parameters, return types

## Implementation Plan

### Phase 1: Infrastructure Setup
1. **Add Debug Info Builder to Backend**
   ```c
   typedef struct LLVMBackendData {
       // ... existing fields ...
       LLVMDIBuilderRef di_builder;
       LLVMMetadataRef di_compile_unit;
       LLVMMetadataRef di_file;
       
       // Debug type cache
       LLVMMetadataRef di_i32_type;
       LLVMMetadataRef di_i64_type;
       // ... other basic types
   } LLVMBackendData;
   ```

2. **Initialize Debug Info in Module**
   ```c
   static void initialize_debug_info(LLVMBackendData *data, const char *filename) {
       // Create DIBuilder
       data->di_builder = LLVMCreateDIBuilder(data->module);
       
       // Create file descriptor
       data->di_file = LLVMDIBuilderCreateFile(data->di_builder, 
           filename, strlen(filename),
           ".", 1); // directory
       
       // Create compile unit
       data->di_compile_unit = LLVMDIBuilderCreateCompileUnit(
           data->di_builder,
           LLVMDWARFSourceLanguageC,
           data->di_file,
           "Asthra Compiler", 15,
           is_optimized,
           "", 0, // flags
           0, // runtime version
           "", 0, // split name
           LLVMDWARFEmissionFull,
           0, // DWOId
           true, // split debug inlining
           false // debug info for profiling
       );
   }
   ```

### Phase 2: Type Mapping
1. **Create Debug Type Information**
   ```c
   static LLVMMetadataRef asthra_type_to_debug_type(
       LLVMBackendData *data, 
       const TypeInfo *type
   ) {
       switch (type->category) {
           case TYPE_INFO_PRIMITIVE:
               return get_primitive_debug_type(data, type);
           case TYPE_INFO_STRUCT:
               return create_struct_debug_type(data, type);
           case TYPE_INFO_FUNCTION:
               return create_function_debug_type(data, type);
           // ... other types
       }
   }
   ```

2. **Cache Common Debug Types**
   ```c
   static void cache_basic_debug_types(LLVMBackendData *data) {
       // Basic types
       data->di_i32_type = LLVMDIBuilderCreateBasicType(
           data->di_builder, "i32", 3, 32, LLVMDWARFTypeEncoding_signed);
       data->di_i64_type = LLVMDIBuilderCreateBasicType(
           data->di_builder, "i64", 3, 64, LLVMDWARFTypeEncoding_signed);
       // ... other types
   }
   ```

### Phase 3: Function Debug Info
1. **Generate Function Debug Info**
   ```c
   static void generate_function_debug_info(
       LLVMBackendData *data, 
       const ASTNode *node,
       LLVMValueRef function
   ) {
       // Create function type
       LLVMMetadataRef fn_type = create_function_debug_type(data, node->type_info);
       
       // Create function debug info
       LLVMMetadataRef di_function = LLVMDIBuilderCreateFunction(
           data->di_builder,
           data->di_file, // scope
           node->data.function_decl.name,
           strlen(node->data.function_decl.name),
           node->data.function_decl.name, // linkage name
           strlen(node->data.function_decl.name),
           data->di_file,
           node->location.line,
           fn_type,
           false, // is local
           true,  // is definition
           node->location.line,
           LLVMDIFlagPrototyped,
           false  // is optimized
       );
       
       // Attach to function
       LLVMSetSubprogram(function, di_function);
   }
   ```

### Phase 4: Variable Debug Info
1. **Local Variable Debug Info**
   ```c
   static void generate_variable_debug_info(
       LLVMBackendData *data,
       const ASTNode *node,
       LLVMValueRef alloca,
       LLVMMetadataRef scope
   ) {
       LLVMMetadataRef di_type = asthra_type_to_debug_type(data, node->type_info);
       
       LLVMMetadataRef di_var = LLVMDIBuilderCreateAutoVariable(
           data->di_builder,
           scope,
           node->data.let_stmt.name,
           strlen(node->data.let_stmt.name),
           data->di_file,
           node->location.line,
           di_type,
           true, // preserve through opts
           LLVMDIFlagZero,
           0 // align
       );
       
       // Create debug declare
       LLVMDIBuilderInsertDeclareAtEnd(
           data->di_builder,
           alloca,
           di_var,
           LLVMDIBuilderCreateExpression(data->di_builder, NULL, 0),
           LLVMDIBuilderCreateDebugLocation(
               data->context,
               node->location.line,
               node->location.column,
               scope,
               NULL
           ),
           LLVMGetInsertBlock(data->builder)
       );
   }
   ```

### Phase 5: Location Tracking
1. **Set Debug Location for Each Instruction**
   ```c
   static void set_debug_location(
       LLVMBackendData *data,
       const ASTNode *node,
       LLVMMetadataRef scope
   ) {
       if (node && node->location.line > 0) {
           LLVMValueRef loc = LLVMDIBuilderCreateDebugLocation(
               data->context,
               node->location.line,
               node->location.column,
               scope,
               NULL // inlined at
           );
           LLVMSetCurrentDebugLocation(data->builder, loc);
       }
   }
   ```

2. **Update All Code Generation Functions**
   - Add debug location before generating each instruction
   - Pass scope information through generation functions
   - Maintain scope stack for nested blocks

### Phase 6: Integration Points

1. **Update generate_expression()**
   ```c
   static LLVMValueRef generate_expression(
       LLVMBackendData *data, 
       const ASTNode *node,
       LLVMMetadataRef scope  // NEW: debug scope
   ) {
       // Set debug location
       set_debug_location(data, node, scope);
       
       // ... existing code ...
   }
   ```

2. **Update generate_statement()**
   ```c
   static void generate_statement(
       LLVMBackendData *data,
       const ASTNode *node,
       LLVMMetadataRef scope  // NEW: debug scope
   ) {
       // Set debug location
       set_debug_location(data, node, scope);
       
       // Handle scope changes for blocks
       if (node->type == AST_BLOCK) {
           LLVMMetadataRef new_scope = LLVMDIBuilderCreateLexicalBlock(
               data->di_builder, scope, data->di_file,
               node->location.line, node->location.column
           );
           // Use new_scope for nested statements
       }
   }
   ```

### Phase 7: Finalization
1. **Finalize Debug Info**
   ```c
   // In llvm_backend_generate(), after code generation:
   if (data->di_builder) {
       LLVMDIBuilderFinalize(data->di_builder);
   }
   ```

2. **Add Module Flags**
   ```c
   // Set debug info version
   LLVMAddModuleFlag(data->module, LLVMModuleFlagBehaviorWarning,
                     "Debug Info Version", 18,
                     LLVMValueAsMetadata(LLVMConstInt(data->i32_type, 3, false)));
   
   // Set DWARF version
   LLVMAddModuleFlag(data->module, LLVMModuleFlagBehaviorWarning,
                     "Dwarf Version", 13,
                     LLVMValueAsMetadata(LLVMConstInt(data->i32_type, 4, false)));
   ```

## Testing Strategy

1. **Basic Verification**
   - Check LLVM IR contains !dbg metadata
   - Verify debug info with `llvm-dwarfdump`

2. **Debugger Testing**
   - Set breakpoints by line number
   - Step through code
   - Inspect variables
   - View call stack

3. **Test Cases**
   - Simple function with variables
   - Nested scopes
   - Control flow (if/for)
   - Function calls
   - Complex expressions

## Performance Considerations

1. **Conditional Compilation**
   ```c
   if (backend->options.debug_info) {
       // Generate debug info
   }
   ```

2. **Lazy Generation**
   - Only generate debug types when needed
   - Cache frequently used debug types

3. **Memory Management**
   - DIBuilder manages most memory
   - Ensure proper cleanup in backend_cleanup()

## Error Handling

1. **Graceful Degradation**
   - Continue compilation if debug info fails
   - Warn user about missing debug info

2. **Validation**
   - Check for null pointers
   - Verify source locations are valid
   - Handle missing type information

## Implementation Priority

1. **High Priority**
   - Compile unit and file info
   - Function debug info
   - Line number mapping

2. **Medium Priority**
   - Local variable debug info
   - Basic type information
   - Scope tracking

3. **Low Priority**
   - Complex type debug info (structs, enums)
   - Optimization remarks
   - Advanced DWARF features

## Dependencies

- LLVM 15.0+ with debug info support
- Source location tracking in AST
- Type information availability

## Estimated Timeline

- Phase 1-2: 2-3 hours (infrastructure and basic types)
- Phase 3-4: 3-4 hours (function and variable info)
- Phase 5-6: 4-5 hours (location tracking and integration)
- Phase 7: 1-2 hours (finalization and testing)
- Total: 10-14 hours of implementation

## Success Criteria

1. Generated LLVM IR contains debug metadata
2. GDB/LLDB can set breakpoints by line
3. Variable values visible in debugger
4. Source stepping works correctly
5. No significant performance impact when disabled