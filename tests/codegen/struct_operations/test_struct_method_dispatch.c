#include "test_complex_struct_operations_common.h"

// Test metadata
static AsthraTestMetadata test_metadata[] = {
    {"test_struct_method_dispatch", __FILE__, __LINE__, "Test method dispatch on structs", ASTHRA_TEST_SEVERITY_MEDIUM, 5000000000ULL, false, NULL}
};

AsthraTestResult test_struct_method_dispatch(AsthraTestContext* context) {
    ComplexStructContext ctx;
    init_complex_struct_context(&ctx);
    
    // Define a struct with virtual methods
    add_struct_definition(&ctx, "Drawable", STRUCT_TYPE_SIMPLE, 16, 8, true);
    add_struct_field(&ctx, 0, "x", "int", 0, 4, false);
    add_struct_field(&ctx, 0, "y", "int", 4, 4, false);
    add_struct_field(&ctx, 0, "vtable", "void*", 8, 8, true);
    
    // Add virtual methods
    const char* draw_params[] = {"void"};
    const char* move_params[] = {"int", "int"};
    const char* area_params[] = {"void"};
    
    add_struct_method(&ctx, "draw", "void", 1, draw_params, true);
    add_struct_method(&ctx, "move", "void", 3, move_params, true);
    add_struct_method(&ctx, "get_area", "double", 1, area_params, true);
    add_struct_method(&ctx, "static_method", "int", 1, draw_params, false);
    
    // Create method dispatch simulation
    if (ctx.method_count != 4 || ctx.vtable_size != 3) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Verify virtual methods are in vtable
    bool found_draw = false, found_move = false, found_area = false;
    
    for (int i = 0; i < ctx.method_count; i++) {
        const StructMethod* method = &ctx.methods[i];
        
        if (method->is_virtual) {
            // Check if method address is in vtable
            bool in_vtable = false;
            for (int j = 0; j < ctx.vtable_size; j++) {
                if (ctx.vtable[j] == method->address) {
                    in_vtable = true;
                    break;
                }
            }
            
            if (!in_vtable) {
                return ASTHRA_TEST_FAIL;
            }
            
            if (strcmp(method->name, "draw") == 0) found_draw = true;
            if (strcmp(method->name, "move") == 0) found_move = true;
            if (strcmp(method->name, "get_area") == 0) found_area = true;
        }
    }
    
    if (!found_draw || !found_move || !found_area) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Create instance and simulate method calls
    void* drawable = allocate_struct_instance(&ctx, 0);
    if (!drawable) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Set up vtable pointer in instance
    const StructField* vtable_field = find_struct_field(&ctx, 0, "vtable");
    if (!vtable_field) {
        return ASTHRA_TEST_FAIL;
    }
    
    void* vtable_ptr = (void*)ctx.vtable;
    write_struct_field(drawable, vtable_field, &vtable_ptr);
    
    // Verify vtable was set
    void* read_vtable;
    read_struct_field(drawable, vtable_field, &read_vtable);
    
    if (read_vtable != vtable_ptr) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Simulate virtual method dispatch
    uint64_t* vtable_array = (uint64_t*)read_vtable;
    
    // Check that we can "call" virtual methods through vtable
    for (int i = 0; i < ctx.vtable_size; i++) {
        uint64_t method_addr = vtable_array[i];
        
        // Verify method address is valid
        bool found_method = false;
        for (int j = 0; j < ctx.method_count; j++) {
            if (ctx.methods[j].address == method_addr && ctx.methods[j].is_virtual) {
                found_method = true;
                break;
            }
        }
        
        if (!found_method) {
            return ASTHRA_TEST_FAIL;
        }
    }
    
    // Test method signature validation
    for (int i = 0; i < ctx.method_count; i++) {
        const StructMethod* method = &ctx.methods[i];
        
        if (strcmp(method->name, "move") == 0) {
            if (method->param_count != 3 || 
                strcmp(method->param_types[1], "int") != 0 ||
                strcmp(method->param_types[2], "int") != 0) {
                return ASTHRA_TEST_FAIL;
            }
        }
        
        if (strcmp(method->name, "get_area") == 0) {
            if (strcmp(method->return_type, "double") != 0) {
                return ASTHRA_TEST_FAIL;
            }
        }
    }
    
    return ASTHRA_TEST_PASS;
}

// Standalone test runner for this module
int main(void) {
    printf("=== Struct Method Dispatch Test ===\n");
    
    AsthraTestContext context = {0};
    
    if (test_struct_method_dispatch(&context) == ASTHRA_TEST_PASS) {
        printf("✅ Struct Method Dispatch: PASS\n");
        return 0;
    } else {
        printf("❌ Struct Method Dispatch: FAIL\n");
        return 1;
    }
} 
