/**
 * Type system stub implementations
 * Minimal implementations to satisfy the linker for testing
 */

#include <stdbool.h>
#include <stdlib.h>

// Type system stubs
void *create_type_info_from_descriptor(void *desc) {
    (void)desc;
    return malloc(1);
}

void *type_info_from_descriptor(void *desc) {
    (void)desc;
    return malloc(1);
}

void *get_builtin_type_descriptor(int type) {
    (void)type;
    return malloc(1);
}

void *get_promoted_type(void *type1, void *type2) {
    (void)type1;
    (void)type2;
    return malloc(1);
}

void *get_promoted_integer_type(void *type) {
    (void)type;
    return type;
}

// Type descriptor stubs
void *type_descriptor_create_function(void *return_type, void *params) {
    (void)return_type;
    (void)params;
    return malloc(1);
}

void type_descriptor_release(void *desc) {
    free(desc);
}

void type_descriptor_retain(void *desc) {
    (void)desc;
}