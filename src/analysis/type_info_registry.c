#include "semantic_analyzer.h"
#include "type_info.h"
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

// =============================================================================
// GLOBAL TYPE REGISTRY
// =============================================================================

TypeInfo **g_type_registry = NULL;
size_t g_type_registry_size = 0;
size_t g_type_registry_capacity = 0;
static uint32_t g_next_type_id = 1000; // Start after built-in types
pthread_mutex_t g_type_registry_mutex = PTHREAD_MUTEX_INITIALIZER;

// =============================================================================
// TYPE ID ALLOCATION
// =============================================================================

uint32_t type_info_allocate_id(void) {
    pthread_mutex_lock(&g_type_registry_mutex);
    uint32_t id = g_next_type_id++;
    pthread_mutex_unlock(&g_type_registry_mutex);
    return id;
}

// =============================================================================
// REGISTRY MANAGEMENT
// =============================================================================

static bool add_to_registry(TypeInfo *type_info) {
    pthread_mutex_lock(&g_type_registry_mutex);

    // Expand registry if needed
    if (g_type_registry_size >= g_type_registry_capacity) {
        size_t new_capacity = g_type_registry_capacity == 0 ? 64 : g_type_registry_capacity * 2;
        TypeInfo **new_registry = realloc(g_type_registry, new_capacity * sizeof(TypeInfo *));
        if (!new_registry) {
            pthread_mutex_unlock(&g_type_registry_mutex);
            return false;
        }
        g_type_registry = new_registry;
        g_type_registry_capacity = new_capacity;
    }

    g_type_registry[g_type_registry_size++] = type_info;
    type_info_retain(type_info); // Registry holds a reference

    pthread_mutex_unlock(&g_type_registry_mutex);
    return true;
}

bool type_info_register(TypeInfo *type_info) {
    if (!type_info)
        return false;
    return add_to_registry(type_info);
}

TypeInfo *type_info_lookup_by_name(const char *name) {
    if (!name)
        return NULL;

    pthread_mutex_lock(&g_type_registry_mutex);
    for (size_t i = 0; i < g_type_registry_size; i++) {
        if (g_type_registry[i] && strcmp(g_type_registry[i]->name, name) == 0) {
            TypeInfo *result = g_type_registry[i];
            type_info_retain(result);
            pthread_mutex_unlock(&g_type_registry_mutex);
            return result;
        }
    }
    pthread_mutex_unlock(&g_type_registry_mutex);
    return NULL;
}

TypeInfo *type_info_lookup_by_id(uint32_t type_id) {
    pthread_mutex_lock(&g_type_registry_mutex);
    for (size_t i = 0; i < g_type_registry_size; i++) {
        if (g_type_registry[i] && g_type_registry[i]->type_id == type_id) {
            TypeInfo *result = g_type_registry[i];
            type_info_retain(result);
            pthread_mutex_unlock(&g_type_registry_mutex);
            return result;
        }
    }
    pthread_mutex_unlock(&g_type_registry_mutex);
    return NULL;
}

void type_info_registry_cleanup(void) {
    pthread_mutex_lock(&g_type_registry_mutex);
    if (g_type_registry) {
        for (size_t i = 0; i < g_type_registry_size; i++) {
            type_info_release(g_type_registry[i]);
        }
        free(g_type_registry);
        g_type_registry = NULL;
        g_type_registry_size = 0;
        g_type_registry_capacity = 0;
    }
    pthread_mutex_unlock(&g_type_registry_mutex);
}
