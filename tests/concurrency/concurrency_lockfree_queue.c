/**
 * Asthra Programming Language - Lock-Free Queue Implementation
 * 
 * Phase 4: Core Infrastructure Implementation
 * Copyright (c) 2025 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Lock-free queue implementation for concurrency testing
 */

#include "concurrency_integration_common.h"

// =============================================================================
// LOCK-FREE DATA STRUCTURE IMPLEMENTATION
// =============================================================================

/**
 * Create lock-free queue for concurrency testing
 */
LockFreeQueue* lockfree_queue_create(void) {
    LockFreeQueue* queue = calloc(1, sizeof(LockFreeQueue));
    if (!queue) return NULL;
    
    atomic_store(&queue->head, (uintptr_t)NULL);
    atomic_store(&queue->size, 0);
    atomic_store(&queue->operations_count, 0);
    
    return queue;
}

/**
 * Enqueue operation (lock-free)
 */
bool lockfree_queue_enqueue(LockFreeQueue* queue, int data) {
    if (!queue) return false;
    
    LockFreeNode* new_node = calloc(1, sizeof(LockFreeNode));
    if (!new_node) return false;
    
    atomic_store(&new_node->data, data);
    atomic_store(&new_node->is_valid, true);
    
    // Lock-free insertion using compare-and-swap
    uintptr_t expected_head;
    do {
        expected_head = atomic_load(&queue->head);
        atomic_store(&new_node->next, expected_head);
    } while (!atomic_compare_exchange_weak(&queue->head, &expected_head, (uintptr_t)new_node));
    
    atomic_fetch_add(&queue->size, 1);
    atomic_fetch_add(&queue->operations_count, 1);
    
    return true;
}

/**
 * Dequeue operation (lock-free)
 */
bool lockfree_queue_dequeue(LockFreeQueue* queue, int* data) {
    if (!queue || !data) return false;
    
    uintptr_t current_head;
    LockFreeNode* head_node;
    
    do {
        current_head = atomic_load(&queue->head);
        if (current_head == (uintptr_t)NULL) {
            return false; // Queue is empty
        }
        
        head_node = (LockFreeNode*)current_head;
        uintptr_t next_head = atomic_load(&head_node->next);
        
        if (atomic_compare_exchange_weak(&queue->head, &current_head, next_head)) {
            *data = atomic_load(&head_node->data);
            atomic_store(&head_node->is_valid, false);
            free(head_node);
            atomic_fetch_sub(&queue->size, 1);
            atomic_fetch_add(&queue->operations_count, 1);
            return true;
        }
    } while (true);
}

/**
 * Get queue size (approximate, for testing)
 */
size_t lockfree_queue_size(LockFreeQueue* queue) {
    if (!queue) return 0;
    return atomic_load(&queue->size);
}

/**
 * Destroy lock-free queue
 */
void lockfree_queue_destroy(LockFreeQueue* queue) {
    if (!queue) return;
    
    // Drain remaining items
    int dummy_data;
    while (lockfree_queue_dequeue(queue, &dummy_data)) {
        // Continue draining
    }
    
    free(queue);
}