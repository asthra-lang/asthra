#include "test_complex_struct_operations_common.h"

// Test metadata
static AsthraTestMetadata test_metadata[] = {
    {"test_complex_struct_layout", __FILE__, __LINE__, "Test complex struct memory layout", ASTHRA_TEST_SEVERITY_MEDIUM, 5000000000ULL, false, NULL}
};

AsthraTestResult test_complex_struct_layout(AsthraTestContext* context) {
    ComplexStructContext ctx;
    init_complex_struct_context(&ctx);
    
    // Define a complex struct with various field types
    add_struct_definition(&ctx, "ComplexData", STRUCT_TYPE_VARIANT, 64, 16, false);
    add_struct_field(&ctx, 0, "header", "int", 0, 4, false);
    add_struct_field(&ctx, 0, "flags", "short", 4, 2, false);
    add_struct_field(&ctx, 0, "data_ptr", "void*", 8, 8, true);      // Alignment padding after short
    add_struct_field(&ctx, 0, "timestamp", "long", 16, 8, false);
    add_struct_field(&ctx, 0, "coordinates", "double[3]", 24, 24, false); // Array field
    add_struct_field(&ctx, 0, "metadata", "char[16]", 48, 16, false);
    
    // Define a packed struct for comparison
    add_struct_definition(&ctx, "PackedData", STRUCT_TYPE_SIMPLE, 35, 1, false);
    ctx.definitions[1].is_packed = true;
    add_struct_field(&ctx, 1, "header", "int", 0, 4, false);
    add_struct_field(&ctx, 1, "flags", "short", 4, 2, false);
    add_struct_field(&ctx, 1, "data_ptr", "void*", 6, 8, true);      // No padding in packed
    add_struct_field(&ctx, 1, "timestamp", "long", 14, 8, false);
    add_struct_field(&ctx, 1, "small_data", "char[13]", 22, 13, false);
    
    // Define a union-like struct
    add_struct_definition(&ctx, "VariantData", STRUCT_TYPE_VARIANT, 16, 8, false);
    ctx.definitions[2].is_union = true;
    add_struct_field(&ctx, 2, "as_int", "long", 0, 8, false);
    add_struct_field(&ctx, 2, "as_double", "double", 0, 8, false);   // Same offset (union)
    add_struct_field(&ctx, 2, "as_bytes", "char[8]", 0, 8, false);   // Same offset (union)
    add_struct_field(&ctx, 2, "tag", "int", 8, 4, false);            // Separate field
    
    // Verify struct layouts
    if (ctx.definition_count != 3) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Check complex struct layout and alignment
    const StructDefinition* complex_def = &ctx.definitions[0];
    if (complex_def->size != 64 || complex_def->alignment != 16) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Verify field offsets respect alignment
    const StructField* data_ptr_field = find_struct_field(&ctx, 0, "data_ptr");
    const StructField* timestamp_field = find_struct_field(&ctx, 0, "timestamp");
    const StructField* coords_field = find_struct_field(&ctx, 0, "coordinates");
    
    if (!data_ptr_field || !timestamp_field || !coords_field) {
        return ASTHRA_TEST_FAIL;
    }
    
    // data_ptr should be aligned to 8 bytes after the short field
    if (data_ptr_field->offset != 8) {
        return ASTHRA_TEST_FAIL;
    }
    
    // timestamp should follow data_ptr
    if (timestamp_field->offset != 16) {
        return ASTHRA_TEST_FAIL;
    }
    
    // coordinates array should be properly placed
    if (coords_field->offset != 24 || coords_field->size != 24) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Check packed struct has no padding
    const StructDefinition* packed_def = &ctx.definitions[1];
    if (packed_def->size != 35 || packed_def->alignment != 1 || !packed_def->is_packed) {
        return ASTHRA_TEST_FAIL;
    }
    
    const StructField* packed_ptr = find_struct_field(&ctx, 1, "data_ptr");
    if (!packed_ptr || packed_ptr->offset != 6) {  // No alignment padding
        return ASTHRA_TEST_FAIL;
    }
    
    // Check union struct has overlapping fields
    const StructDefinition* union_def = &ctx.definitions[2];
    if (!union_def->is_union) {
        return ASTHRA_TEST_FAIL;
    }
    
    const StructField* as_int = find_struct_field(&ctx, 2, "as_int");
    const StructField* as_double = find_struct_field(&ctx, 2, "as_double");
    const StructField* as_bytes = find_struct_field(&ctx, 2, "as_bytes");
    const StructField* tag = find_struct_field(&ctx, 2, "tag");
    
    if (!as_int || !as_double || !as_bytes || !tag) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Union fields should have same offset
    if (as_int->offset != 0 || as_double->offset != 0 || as_bytes->offset != 0) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Tag should be separate
    if (tag->offset != 8) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Test memory allocation and field access
    void* complex_instance = allocate_struct_instance(&ctx, 0);
    void* variant_instance = allocate_struct_instance(&ctx, 2);
    
    if (!complex_instance || !variant_instance) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Test union behavior
    long int_value = 0x123456789ABCDEF0;
    write_struct_field(variant_instance, as_int, &int_value);
    
    // Should be able to read the same data as bytes
    char byte_array[8];
    read_struct_field(variant_instance, as_bytes, byte_array);
    
    // Verify the bytes match (little-endian)
    long read_int;
    read_struct_field(variant_instance, as_int, &read_int);
    
    if (read_int != int_value) {
        return ASTHRA_TEST_FAIL;
    }
    
    return ASTHRA_TEST_PASS;
}

// Standalone test runner for this module
int main(void) {
    printf("=== Complex Struct Layout Test ===\n");
    
    AsthraTestContext context = {0};
    
    if (test_complex_struct_layout(&context) == ASTHRA_TEST_PASS) {
        printf("✅ Complex Struct Layout: PASS\n");
        return 0;
    } else {
        printf("❌ Complex Struct Layout: FAIL\n");
        return 1;
    }
} 
