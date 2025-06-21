/**
 * Asthra Programming Language Compiler
 * Label Manager Implementation
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "code_generator.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// =============================================================================
// INTERNAL HELPER FUNCTIONS
// =============================================================================

// Hash function for label names
static size_t hash_string(const char *str) {
    size_t hash = 5381;
    int c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + (size_t)c;
    }
    return hash;
}

// =============================================================================
// LABEL MANAGEMENT WITH THREAD SAFETY
// =============================================================================

LabelManager *label_manager_create(size_t initial_capacity) {
    LabelManager *manager = malloc(sizeof(LabelManager));
    if (!manager) return NULL;
    
    manager->buckets = calloc(initial_capacity, sizeof(LabelEntry*));
    if (!manager->buckets) {
        free(manager);
        return NULL;
    }
    
    // C17 designated initializer
    *manager = (LabelManager) {
        .buckets = manager->buckets,
        .bucket_count = initial_capacity,
        .label_count = 0,
        .next_label_id = 1
    };
    
    if (pthread_rwlock_init(&manager->rwlock, NULL) != 0) {
        free(manager->buckets);
        free(manager);
        return NULL;
    }
    
    return manager;
}

void label_manager_destroy(LabelManager *manager) {
    if (!manager) return;
    
    pthread_rwlock_wrlock(&manager->rwlock);
    
    // Free all label entries
    for (size_t i = 0; i < manager->bucket_count; i++) {
        LabelEntry *entry = manager->buckets[i];
        while (entry) {
            LabelEntry *next = entry->next;
            free(entry->name);
            free(entry);
            entry = next;
        }
    }
    
    free(manager->buckets);
    pthread_rwlock_unlock(&manager->rwlock);
    pthread_rwlock_destroy(&manager->rwlock);
    free(manager);
}

char *label_manager_create_label(LabelManager *manager, LabelType type, const char *prefix) {
    if (!manager || !prefix) return NULL;
    
    uint32_t label_id = atomic_fetch_add(&manager->next_label_id, 1);
    
    // Create unique label name
    size_t name_len = strlen(prefix) + 32; // Enough for prefix + "_" + number + null
    char *name = malloc(name_len);
    if (!name) return NULL;
    
    snprintf(name, name_len, "%s_%u", prefix, label_id);
    
    // Create label entry
    LabelEntry *entry = malloc(sizeof(LabelEntry));
    if (!entry) {
        free(name);
        return NULL;
    }
    
    // C17 designated initializer
    *entry = (LabelEntry) {
        .name = strdup(name),
        .type = type,
        .address = 0,
        .is_resolved = false,
        .next = NULL
    };
    
    if (!entry->name) {
        free(entry);
        free(name);
        return NULL;
    }
    
    // Insert into hash table
    pthread_rwlock_wrlock(&manager->rwlock);
    
    size_t bucket = hash_string(name) % manager->bucket_count;
    entry->next = manager->buckets[bucket];
    manager->buckets[bucket] = entry;
    atomic_fetch_add(&manager->label_count, 1);
    
    pthread_rwlock_unlock(&manager->rwlock);
    
    return name;
}

bool label_manager_define_label(LabelManager *manager, const char *name, size_t address) {
    if (!manager || !name) return false;
    
    pthread_rwlock_wrlock(&manager->rwlock);
    
    size_t bucket = hash_string(name) % manager->bucket_count;
    LabelEntry *entry = manager->buckets[bucket];
    
    while (entry) {
        if (strcmp(entry->name, name) == 0) {
            entry->address = address;
            entry->is_resolved = true;
            pthread_rwlock_unlock(&manager->rwlock);
            return true;
        }
        entry = entry->next;
    }
    
    pthread_rwlock_unlock(&manager->rwlock);
    return false;
}

bool label_manager_resolve_label(LabelManager *manager, const char *name, size_t *address) {
    if (!manager || !name || !address) return false;
    
    pthread_rwlock_rdlock(&manager->rwlock);
    
    size_t bucket = hash_string(name) % manager->bucket_count;
    LabelEntry *entry = manager->buckets[bucket];
    
    while (entry) {
        if (strcmp(entry->name, name) == 0) {
            if (entry->is_resolved) {
                *address = entry->address;
                pthread_rwlock_unlock(&manager->rwlock);
                return true;
            } else {
                pthread_rwlock_unlock(&manager->rwlock);
                return false; // Label exists but not resolved yet
            }
        }
        entry = entry->next;
    }
    
    pthread_rwlock_unlock(&manager->rwlock);
    return false; // Label not found
}

bool label_manager_is_defined(LabelManager *manager, const char *name) {
    if (!manager || !name) return false;
    
    pthread_rwlock_rdlock(&manager->rwlock);
    
    size_t bucket = hash_string(name) % manager->bucket_count;
    LabelEntry *entry = manager->buckets[bucket];
    
    while (entry) {
        if (strcmp(entry->name, name) == 0) {
            pthread_rwlock_unlock(&manager->rwlock);
            return true;
        }
        entry = entry->next;
    }
    
    pthread_rwlock_unlock(&manager->rwlock);
    return false;
} 
