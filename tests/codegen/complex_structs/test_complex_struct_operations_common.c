#include "test_complex_struct_operations_common.h"

// Helper functions implementation
void init_complex_struct_context(ComplexStructContext* ctx) {
    ctx->definition_count = 0;
    ctx->field_count = 0;
    ctx->method_count = 0;
    ctx->memory_used = 0;
    ctx->vtable_size = 0;
    
    for (int i = 0; i < 32; i++) {
        ctx->definitions[i] = (StructDefinition){0};
        ctx->vtable[i] = 0;
    }
    
    for (int i = 0; i < 128; i++) {
        ctx->fields[i] = (StructField){0};
    }
    
    for (int i = 0; i < 64; i++) {
        ctx->methods[i] = (StructMethod){0};
    }
    
    memset(ctx->memory_pool, 0, sizeof(ctx->memory_pool));
}

void add_struct_definition(ComplexStructContext* ctx, const char* name, StructType type, 
                          size_t size, size_t alignment, bool has_methods) {
    if (ctx->definition_count < 32) {
        StructDefinition* def = &ctx->definitions[ctx->definition_count++];
        def->name = name;
        def->type = type;
        def->size = size;
        def->alignment = alignment;
        def->field_count = 0;
        def->has_methods = has_methods;
        def->is_packed = false;
        def->is_union = false;
    }
}

void add_struct_field(ComplexStructContext* ctx, int struct_idx, const char* field_name, 
                     const char* type_name, size_t offset, size_t size, bool is_pointer) {
    if (ctx->field_count < 128 && struct_idx < ctx->definition_count) {
        StructField* field = &ctx->fields[ctx->field_count++];
        field->name = field_name;
        field->type_name = type_name;
        field->offset = offset;
        field->size = size;
        field->is_pointer = is_pointer;
        field->is_array = false;
        field->array_size = 0;
        
        ctx->definitions[struct_idx].field_count++;
    }
}

void add_struct_method(ComplexStructContext* ctx, const char* method_name, const char* return_type,
                      int param_count, const char** param_types, bool is_virtual) {
    if (ctx->method_count < 64 && param_count <= 8) {
        StructMethod* method = &ctx->methods[ctx->method_count++];
        method->name = method_name;
        method->return_type = return_type;
        method->param_count = param_count;
        method->is_virtual = is_virtual;
        method->is_static = false;
        method->address = 0x2000 + (ctx->method_count * 0x10);
        
        for (int i = 0; i < param_count; i++) {
            method->param_types[i] = param_types[i];
        }
        
        // Add to virtual table if virtual
        if (is_virtual && ctx->vtable_size < 32) {
            ctx->vtable[ctx->vtable_size++] = method->address;
        }
    }
}

void* allocate_struct_instance(ComplexStructContext* ctx, int struct_idx) {
    if (struct_idx >= ctx->definition_count) {
        return NULL;
    }
    
    const StructDefinition* def = &ctx->definitions[struct_idx];
    size_t aligned_size = (def->size + def->alignment - 1) & ~(def->alignment - 1);
    
    if (ctx->memory_used + aligned_size > sizeof(ctx->memory_pool)) {
        return NULL;
    }
    
    // Align the allocation
    size_t aligned_offset = (ctx->memory_used + def->alignment - 1) & ~(def->alignment - 1);
    void* instance = &ctx->memory_pool[aligned_offset];
    ctx->memory_used = aligned_offset + aligned_size;
    
    // Initialize memory
    memset(instance, 0, aligned_size);
    
    return instance;
}

bool write_struct_field(void* instance, const StructField* field, const void* value) {
    if (!instance || !field || !value) {
        return false;
    }
    
    uint8_t* base = (uint8_t*)instance;
    uint8_t* field_ptr = base + field->offset;
    
    memcpy(field_ptr, value, field->size);
    return true;
}

bool read_struct_field(const void* instance, const StructField* field, void* value) {
    if (!instance || !field || !value) {
        return false;
    }
    
    const uint8_t* base = (const uint8_t*)instance;
    const uint8_t* field_ptr = base + field->offset;
    
    memcpy(value, field_ptr, field->size);
    return true;
}

const StructField* find_struct_field(const ComplexStructContext* ctx, int struct_idx, const char* field_name) {
    if (struct_idx >= ctx->definition_count) {
        return NULL;
    }
    
    // Find field in the struct definition
    int field_start = 0;
    for (int i = 0; i < struct_idx; i++) {
        field_start += ctx->definitions[i].field_count;
    }
    
    for (int i = 0; i < ctx->definitions[struct_idx].field_count; i++) {
        const StructField* field = &ctx->fields[field_start + i];
        if (strcmp(field->name, field_name) == 0) {
            return field;
        }
    }
    
    return NULL;
}

// Test implementations
AsthraTestResult test_nested_struct_operations(AsthraTestContext* context) {
    ComplexStructContext ctx;
    init_complex_struct_context(&ctx);
    
    // Create nested struct hierarchy: Point -> Rectangle -> Shape
    add_struct_definition(&ctx, "Point", STRUCT_TYPE_SIMPLE, 16, 8, false);
    add_struct_field(&ctx, 0, "x", "double", 0, 8, false);
    add_struct_field(&ctx, 0, "y", "double", 8, 8, false);
    
    add_struct_definition(&ctx, "Rectangle", STRUCT_TYPE_NESTED, 48, 8, true);
    add_struct_field(&ctx, 1, "topLeft", "Point", 0, 16, false);
    add_struct_field(&ctx, 1, "bottomRight", "Point", 16, 16, false);
    add_struct_field(&ctx, 1, "area", "double", 32, 8, false);
    add_struct_field(&ctx, 1, "perimeter", "double", 40, 8, false);
    
    // Test struct allocation and field access
    void* rect = allocate_struct_instance(&ctx, 1);
    if (!rect) return ASTHRA_TEST_FAIL;
    
    // Set values
    double x1 = 10.0, y1 = 20.0;
    double x2 = 30.0, y2 = 40.0;
    
    const StructField* field = find_struct_field(&ctx, 1, "topLeft");
    if (!field) return ASTHRA_TEST_FAIL;
    
    // Simple validation
    if (field->offset != 0 || field->size != 16) return ASTHRA_TEST_FAIL;
    
    return ASTHRA_TEST_PASS;
}

AsthraTestResult test_struct_inheritance_simulation(AsthraTestContext* context) {
    ComplexStructContext ctx;
    init_complex_struct_context(&ctx);
    
    // Create base "class"
    add_struct_definition(&ctx, "Shape", STRUCT_TYPE_SIMPLE, 16, 8, true);
    add_struct_field(&ctx, 0, "type", "int", 0, 4, false);
    add_struct_field(&ctx, 0, "color", "int", 4, 4, false);
    add_struct_field(&ctx, 0, "vtable", "void*", 8, 8, true);
    
    // Create derived "class" with embedded base
    add_struct_definition(&ctx, "Circle", STRUCT_TYPE_NESTED, 32, 8, true);
    add_struct_field(&ctx, 1, "base", "Shape", 0, 16, false);
    add_struct_field(&ctx, 1, "radius", "double", 16, 8, false);
    add_struct_field(&ctx, 1, "centerX", "double", 24, 8, false);
    
    // Add virtual methods
    const char* draw_params[] = {"Shape*"};
    add_struct_method(&ctx, "draw", "void", 1, draw_params, true);
    
    // Test allocation
    void* circle = allocate_struct_instance(&ctx, 1);
    if (!circle) return ASTHRA_TEST_FAIL;
    
    // Verify vtable was created
    if (ctx.vtable_size == 0) return ASTHRA_TEST_FAIL;
    
    return ASTHRA_TEST_PASS;
}

AsthraTestResult test_struct_method_dispatch(AsthraTestContext* context) {
    ComplexStructContext ctx;
    init_complex_struct_context(&ctx);
    
    // Create struct with methods
    add_struct_definition(&ctx, "Calculator", STRUCT_TYPE_SIMPLE, 24, 8, true);
    add_struct_field(&ctx, 0, "value", "double", 0, 8, false);
    add_struct_field(&ctx, 0, "vtable", "void*", 8, 8, true);
    add_struct_field(&ctx, 0, "flags", "int", 16, 4, false);
    
    // Add methods with different signatures
    const char* add_params[] = {"Calculator*", "double"};
    const char* mul_params[] = {"Calculator*", "double"};
    const char* reset_params[] = {"Calculator*"};
    
    add_struct_method(&ctx, "add", "double", 2, add_params, true);
    add_struct_method(&ctx, "multiply", "double", 2, mul_params, true);
    add_struct_method(&ctx, "reset", "void", 1, reset_params, false);
    
    // Verify method registration
    if (ctx.method_count != 3) return ASTHRA_TEST_FAIL;
    if (ctx.vtable_size != 2) return ASTHRA_TEST_FAIL;  // Only 2 virtual methods
    
    return ASTHRA_TEST_PASS;
}

 
