#ifndef TEST_COMPLEX_STRUCT_OPERATIONS_COMMON_H
#define TEST_COMPLEX_STRUCT_OPERATIONS_COMMON_H

#include "../framework/test_framework_minimal.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Complex struct type definitions
typedef enum {
    STRUCT_TYPE_SIMPLE,
    STRUCT_TYPE_NESTED,
    STRUCT_TYPE_VARIANT,
    STRUCT_TYPE_GENERIC,
    STRUCT_TYPE_COUNT
} StructType;

typedef struct {
    const char *name;
    StructType type;
    size_t size;
    size_t alignment;
    int field_count;
    bool has_methods;
    bool is_packed;
    bool is_union;
} StructDefinition;

typedef struct {
    const char *name;
    const char *type_name;
    size_t offset;
    size_t size;
    bool is_pointer;
    bool is_array;
    int array_size;
} StructField;

typedef struct {
    const char *name;
    const char *return_type;
    int param_count;
    const char *param_types[8];
    bool is_virtual;
    bool is_static;
    uint64_t address;
} StructMethod;

typedef struct {
    StructDefinition definitions[32];
    int definition_count;
    StructField fields[128];
    int field_count;
    StructMethod methods[64];
    int method_count;

    // Runtime data
    uint8_t memory_pool[4096];
    size_t memory_used;

    // Method dispatch table
    uint64_t vtable[32];
    int vtable_size;
} ComplexStructContext;

// Function declarations
void init_complex_struct_context(ComplexStructContext *ctx);
void add_struct_definition(ComplexStructContext *ctx, const char *name, StructType type,
                           size_t size, size_t alignment, bool has_methods);
void add_struct_field(ComplexStructContext *ctx, int struct_idx, const char *field_name,
                      const char *type_name, size_t offset, size_t size, bool is_pointer);
void add_struct_method(ComplexStructContext *ctx, const char *method_name, const char *return_type,
                       int param_count, const char **param_types, bool is_virtual);
void *allocate_struct_instance(ComplexStructContext *ctx, int struct_idx);
bool write_struct_field(void *instance, const StructField *field, const void *value);
bool read_struct_field(const void *instance, const StructField *field, void *value);
const StructField *find_struct_field(const ComplexStructContext *ctx, int struct_idx,
                                     const char *field_name);

// Test function declarations
AsthraTestResult test_nested_struct_operations(AsthraTestContext *context);
AsthraTestResult test_struct_inheritance_simulation(AsthraTestContext *context);
AsthraTestResult test_struct_method_dispatch(AsthraTestContext *context);
AsthraTestResult test_complex_struct_layout(AsthraTestContext *context);

#endif // TEST_COMPLEX_STRUCT_OPERATIONS_COMMON_H
