#include "test_complex_struct_operations_common.h"

// Test metadata
static AsthraTestMetadata test_metadata[] = {
    {"test_nested_struct_operations", __FILE__, __LINE__, "Test nested struct creation and access",
     ASTHRA_TEST_SEVERITY_HIGH, 5000000000ULL, false, NULL}};

AsthraTestResult test_nested_struct_operations(AsthraTestContext *context) {
    ComplexStructContext ctx;
    init_complex_struct_context(&ctx);

    // Define a Point struct
    add_struct_definition(&ctx, "Point", STRUCT_TYPE_SIMPLE, 16, 8, false);
    add_struct_field(&ctx, 0, "x", "double", 0, 8, false);
    add_struct_field(&ctx, 0, "y", "double", 8, 8, false);

    // Define a Rectangle struct containing two Points
    add_struct_definition(&ctx, "Rectangle", STRUCT_TYPE_NESTED, 32, 8, false);
    add_struct_field(&ctx, 1, "top_left", "Point", 0, 16, false);
    add_struct_field(&ctx, 1, "bottom_right", "Point", 16, 16, false);

    // Define a Shape struct containing a Rectangle and metadata
    add_struct_definition(&ctx, "Shape", STRUCT_TYPE_NESTED, 48, 8, true);
    add_struct_field(&ctx, 2, "rect", "Rectangle", 0, 32, false);
    add_struct_field(&ctx, 2, "id", "int", 32, 4, false);
    add_struct_field(&ctx, 2, "visible", "bool", 36, 1, false);

    // Verify struct definitions were created
    if (ctx.definition_count != 3) {
        return ASTHRA_TEST_FAIL;
    }

    // Check Point struct
    const StructDefinition *point_def = &ctx.definitions[0];
    if (point_def->size != 16 || point_def->field_count != 2) {
        return ASTHRA_TEST_FAIL;
    }

    // Check Rectangle struct
    const StructDefinition *rect_def = &ctx.definitions[1];
    if (rect_def->size != 32 || rect_def->field_count != 2) {
        return ASTHRA_TEST_FAIL;
    }

    // Check Shape struct
    const StructDefinition *shape_def = &ctx.definitions[2];
    if (shape_def->size != 48 || shape_def->field_count != 3 || !shape_def->has_methods) {
        return ASTHRA_TEST_FAIL;
    }

    // Create instances and test nested access
    void *point1 = allocate_struct_instance(&ctx, 0);
    void *point2 = allocate_struct_instance(&ctx, 0);
    void *rect = allocate_struct_instance(&ctx, 1);
    void *shape = allocate_struct_instance(&ctx, 2);

    if (!point1 || !point2 || !rect || !shape) {
        return ASTHRA_TEST_FAIL;
    }

    // Set point values
    double x1 = 10.0, y1 = 20.0;
    double x2 = 30.0, y2 = 40.0;

    const StructField *x_field = find_struct_field(&ctx, 0, "x");
    const StructField *y_field = find_struct_field(&ctx, 0, "y");

    if (!x_field || !y_field) {
        return ASTHRA_TEST_FAIL;
    }

    write_struct_field(point1, x_field, &x1);
    write_struct_field(point1, y_field, &y1);
    write_struct_field(point2, x_field, &x2);
    write_struct_field(point2, y_field, &y2);

    // Verify point values
    double read_x, read_y;
    read_struct_field(point1, x_field, &read_x);
    read_struct_field(point1, y_field, &read_y);

    if (read_x != 10.0 || read_y != 20.0) {
        return ASTHRA_TEST_FAIL;
    }

    // Test nested struct field access
    const StructField *rect_field = find_struct_field(&ctx, 2, "rect");
    const StructField *id_field = find_struct_field(&ctx, 2, "id");

    if (!rect_field || !id_field) {
        return ASTHRA_TEST_FAIL;
    }

    int shape_id = 42;
    write_struct_field(shape, id_field, &shape_id);

    int read_id;
    read_struct_field(shape, id_field, &read_id);

    if (read_id != 42) {
        return ASTHRA_TEST_FAIL;
    }

    return ASTHRA_TEST_PASS;
}

// Standalone test runner for this module
int main(void) {
    printf("=== Nested Struct Operations Test ===\n");

    AsthraTestContext context = {0};

    if (test_nested_struct_operations(&context) == ASTHRA_TEST_PASS) {
        printf("✅ Nested Struct Operations: PASS\n");
        return 0;
    } else {
        printf("❌ Nested Struct Operations: FAIL\n");
        return 1;
    }
}
