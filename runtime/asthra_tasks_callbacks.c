/**
 * Asthra Programming Language Task System Callbacks v1.2
 * Callback Queue Management Implementation
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "asthra_tasks_callbacks.h"

// =============================================================================
// GLOBAL VARIABLES
// =============================================================================

AsthraCallbackQueue g_callback_queue = {0};
bool g_callback_queue_initialized = false;

// =============================================================================
// CALLBACK QUEUE IMPLEMENTATION
// =============================================================================

void init_callback_queue(void) {
    if (g_callback_queue_initialized) return;
    
    pthread_mutex_init(&g_callback_queue.mutex, NULL);
    #if ASTHRA_HAS_ATOMICS
    atomic_init(&g_callback_queue.queue_size, 0);
    #else
    g_callback_queue.queue_size = 0;
    #endif
    g_callback_queue_initialized = true;
}

void asthra_callback_queue_push(AsthraCallback callback, void *data, size_t data_size) {
    init_callback_queue();
    
    if (!callback) return;
    
    AsthraCallbackNode *node = asthra_alloc(sizeof(AsthraCallbackNode), ASTHRA_ZONE_GC);
    if (!node) return;
    
    node->callback = callback;
    node->data = NULL;
    node->data_size = data_size;
    node->next = NULL;
    
    // Copy data if provided
    if (data && data_size > 0) {
        node->data = asthra_alloc(data_size, ASTHRA_ZONE_GC);
        if (node->data) {
            memcpy(node->data, data, data_size);
        }
    }
    
    pthread_mutex_lock(&g_callback_queue.mutex);
    
    if (g_callback_queue.tail) {
        g_callback_queue.tail->next = node;
    } else {
        g_callback_queue.head = node;
    }
    g_callback_queue.tail = node;
    
    ASTHRA_ATOMIC_FETCH_ADD(&g_callback_queue.queue_size, 1);
    
    pthread_mutex_unlock(&g_callback_queue.mutex);
}

void asthra_callback_queue_process(void) {
    init_callback_queue();
    
    pthread_mutex_lock(&g_callback_queue.mutex);
    
    AsthraCallbackNode *current = g_callback_queue.head;
    g_callback_queue.head = NULL;
    g_callback_queue.tail = NULL;
    ASTHRA_ATOMIC_STORE(&g_callback_queue.queue_size, 0);
    
    pthread_mutex_unlock(&g_callback_queue.mutex);
    
    // Process callbacks outside of lock
    while (current) {
        AsthraCallbackNode *next = current->next;
        
        if (current->callback) {
            current->callback(current->data);
        }
        
        if (current->data) {
            asthra_free(current->data, ASTHRA_ZONE_GC);
        }
        asthra_free(current, ASTHRA_ZONE_GC);
        
        current = next;
    }
} 
