#include "test_complex_struct_operations_common.h"

// Test metadata
static AsthraTestMetadata test_metadata[] = {
    {"test_struct_inheritance_simulation", __FILE__, __LINE__, "Test struct inheritance-like behavior", ASTHRA_TEST_SEVERITY_HIGH, 5000000000ULL, false, NULL}
};

AsthraTestResult test_struct_inheritance_simulation(AsthraTestContext* context) {
    ComplexStructContext ctx;
    init_complex_struct_context(&ctx);
    
    // Define a base "Animal" struct
    add_struct_definition(&ctx, "Animal", STRUCT_TYPE_SIMPLE, 24, 8, true);
    add_struct_field(&ctx, 0, "name", "char*", 0, 8, true);
    add_struct_field(&ctx, 0, "age", "int", 8, 4, false);
    add_struct_field(&ctx, 0, "vtable", "void*", 16, 8, true);
    
    // Define a "Dog" struct that "inherits" from Animal
    add_struct_definition(&ctx, "Dog", STRUCT_TYPE_NESTED, 32, 8, true);
    add_struct_field(&ctx, 1, "animal", "Animal", 0, 24, false);  // Base struct
    add_struct_field(&ctx, 1, "breed", "char*", 24, 8, true);
    
    // Define a "Cat" struct that "inherits" from Animal
    add_struct_definition(&ctx, "Cat", STRUCT_TYPE_NESTED, 32, 8, true);
    add_struct_field(&ctx, 2, "animal", "Animal", 0, 24, false);  // Base struct
    add_struct_field(&ctx, 2, "indoor", "bool", 24, 1, false);
    
    // Add virtual methods
    const char* make_sound_params[] = {"void"};
    const char* get_info_params[] = {"void"};
    
    add_struct_method(&ctx, "make_sound", "void", 1, make_sound_params, true);
    add_struct_method(&ctx, "get_info", "char*", 1, get_info_params, true);
    add_struct_method(&ctx, "dog_specific", "void", 1, make_sound_params, false);
    add_struct_method(&ctx, "cat_specific", "void", 1, make_sound_params, false);
    
    // Verify inheritance structure
    if (ctx.definition_count != 3) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Check that derived structs contain base struct
    const StructDefinition* dog_def = &ctx.definitions[1];
    const StructDefinition* cat_def = &ctx.definitions[2];
    
    if (dog_def->size != 32 || cat_def->size != 32) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Both should have the base Animal struct as first field
    const StructField* dog_base = find_struct_field(&ctx, 1, "animal");
    const StructField* cat_base = find_struct_field(&ctx, 2, "animal");
    
    if (!dog_base || !cat_base || dog_base->offset != 0 || cat_base->offset != 0) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Verify virtual method table
    if (ctx.method_count != 4 || ctx.vtable_size != 2) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Create instances
    void* dog = allocate_struct_instance(&ctx, 1);
    void* cat = allocate_struct_instance(&ctx, 2);
    
    if (!dog || !cat) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Test polymorphic behavior simulation
    // Set base Animal fields in Dog
    const StructField* age_field = find_struct_field(&ctx, 0, "age");
    if (!age_field) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Access age field through the base struct offset in Dog
    int dog_age = 5;
    uint8_t* dog_base_ptr = (uint8_t*)dog + dog_base->offset;
    write_struct_field(dog_base_ptr, age_field, &dog_age);
    
    // Read it back
    int read_age;
    read_struct_field(dog_base_ptr, age_field, &read_age);
    
    if (read_age != 5) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Test derived-specific fields
    const StructField* breed_field = find_struct_field(&ctx, 1, "breed");
    const StructField* indoor_field = find_struct_field(&ctx, 2, "indoor");
    
    if (!breed_field || !indoor_field) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Simulate setting breed for dog
    const char* breed = "Golden Retriever";
    write_struct_field(dog, breed_field, &breed);
    
    const char* read_breed;
    read_struct_field(dog, breed_field, &read_breed);
    
    if (read_breed != breed) {
        return ASTHRA_TEST_FAIL;
    }
    
    return ASTHRA_TEST_PASS;
}

// Standalone test runner for this module
int main(void) {
    printf("=== Struct Inheritance Simulation Test ===\n");
    
    AsthraTestContext context = {0};
    
    if (test_struct_inheritance_simulation(&context) == ASTHRA_TEST_PASS) {
        printf("✅ Struct Inheritance Simulation: PASS\n");
        return 0;
    } else {
        printf("❌ Struct Inheritance Simulation: FAIL\n");
        return 1;
    }
} 
