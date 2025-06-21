/**
 * Asthra Programming Language Compiler
 * Generic Instantiation Registry Management Implementation
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Registry management for generic struct instantiations with thread-safe operations
 */

#include "generic_instantiation_registry.h"
#include "generic_instantiation_memory.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

// =============================================================================
// GENERIC REGISTRY MANAGEMENT
// =============================================================================

GenericRegistry *generic_registry_create(void) {
    GenericRegistry *registry = malloc(sizeof(GenericRegistry));
    if (!registry) return NULL;
    
    registry->struct_capacity = 16;
    registry->structs = calloc(registry->struct_capacity, sizeof(GenericStructInfo*));
    if (!registry->structs) {
        free(registry);
        return NULL;
    }
    
    registry->struct_count = 0;
    
    // Initialize thread safety
    if (pthread_rwlock_init(&registry->rwlock, NULL) != 0) {
        free(registry->structs);
        free(registry);
        return NULL;
    }
    
    // Initialize statistics
    atomic_init(&registry->total_instantiations, 0);
    atomic_init(&registry->concrete_structs_generated, 0);
    atomic_init(&registry->bytes_generated, 0);
    
    return registry;
}

void generic_registry_destroy(GenericRegistry *registry) {
    if (!registry) return;
    
    // Destroy all generic struct infos
    for (size_t i = 0; i < registry->struct_count; i++) {
        if (registry->structs[i]) {
            generic_struct_info_destroy(registry->structs[i]);
        }
    }
    
    free(registry->structs);
    pthread_rwlock_destroy(&registry->rwlock);
    free(registry);
}

bool generic_registry_register_struct(GenericRegistry *registry, 
                                     const char *struct_name,
                                     ASTNode *struct_decl,
                                     TypeDescriptor *struct_type) {
    if (!registry || !struct_name || !struct_decl || !struct_type) {
        return false;
    }
    
    pthread_rwlock_wrlock(&registry->rwlock);
    
    // Check if already registered
    for (size_t i = 0; i < registry->struct_count; i++) {
        if (registry->structs[i] && strcmp(registry->structs[i]->generic_name, struct_name) == 0) {
            pthread_rwlock_unlock(&registry->rwlock);
            return true; // Already registered
        }
    }
    
    // Resize if needed
    if (registry->struct_count >= registry->struct_capacity) {
        size_t new_capacity = registry->struct_capacity * 2;
        GenericStructInfo **new_structs = realloc(registry->structs, 
                                                 new_capacity * sizeof(GenericStructInfo*));
        if (!new_structs) {
            pthread_rwlock_unlock(&registry->rwlock);
            return false;
        }
        registry->structs = new_structs;
        registry->struct_capacity = new_capacity;
    }
    
    // Create new generic struct info
    GenericStructInfo *struct_info = generic_struct_info_create(struct_name, struct_decl, struct_type);
    if (!struct_info) {
        pthread_rwlock_unlock(&registry->rwlock);
        return false;
    }
    
    registry->structs[registry->struct_count++] = struct_info;
    
    pthread_rwlock_unlock(&registry->rwlock);
    return true;
}

GenericStructInfo *generic_registry_lookup_struct(GenericRegistry *registry, 
                                                 const char *struct_name) {
    if (!registry || !struct_name) return NULL;
    
    pthread_rwlock_rdlock(&registry->rwlock);
    
    for (size_t i = 0; i < registry->struct_count; i++) {
        if (registry->structs[i] && strcmp(registry->structs[i]->generic_name, struct_name) == 0) {
            GenericStructInfo *result = registry->structs[i];
            pthread_rwlock_unlock(&registry->rwlock);
            return result;
        }
    }
    
    pthread_rwlock_unlock(&registry->rwlock);
    return NULL;
}

void generic_registry_print_stats(GenericRegistry *registry) {
    if (!registry) return;
    
    printf("Generic Registry Statistics:\n");
    printf("  Total generic structs: %zu\n", registry->struct_count);
    printf("  Total instantiations: %u\n", atomic_load(&registry->total_instantiations));
    printf("  Concrete structs generated: %u\n", atomic_load(&registry->concrete_structs_generated));
    printf("  Total bytes generated: %llu\n", (unsigned long long)atomic_load(&registry->bytes_generated));
    
    pthread_rwlock_rdlock(&registry->rwlock);
    for (size_t i = 0; i < registry->struct_count; i++) {
        GenericStructInfo *info = registry->structs[i];
        if (info) {
            printf("  %s: %zu instantiations\n", info->generic_name, info->instantiation_count);
        }
    }
    pthread_rwlock_unlock(&registry->rwlock);
} 
