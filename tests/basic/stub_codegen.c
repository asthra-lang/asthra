/**
 * Code generation stub implementations
 * Minimal implementations to satisfy the linker for testing
 */

#include <stdbool.h>
#include <stdlib.h>

// Code generator stubs
void *generic_registry_create(void) {
    return malloc(1);
}

void generic_registry_destroy(void *registry) {
    free(registry);
}

void *instruction_buffer_create(size_t size) {
    (void)size;
    return malloc(1);
}

void instruction_buffer_destroy(void *buffer) {
    free(buffer);
}

void *label_manager_create(void) {
    return malloc(1);
}

void label_manager_destroy(void *manager) {
    free(manager);
}

void *local_symbol_table_create(void) {
    return malloc(1);
}

void local_symbol_table_destroy(void *table) {
    free(table);
}

bool code_generator_validate_state(void *gen) {
    (void)gen;
    return true;
}

void code_generator_init_defaults(void *gen) {
    (void)gen;
}

// Register allocator stubs
void *register_allocator_create(void) {
    return malloc(1);
}

void register_allocator_destroy(void *alloc) {
    free(alloc);
}