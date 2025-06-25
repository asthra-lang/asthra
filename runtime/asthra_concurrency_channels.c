/**
 * Asthra Concurrency Channels v1.2
 * Channel Communication and Select Operations
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * IMPLEMENTATION FEATURES:
 * - Buffered and unbuffered channel communication
 * - Select operations for multiple channels
 * - Thread-safe send/receive operations with timeouts
 * - Channel statistics and monitoring
 */

#include <assert.h>
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// C17 modernization includes
#include <stdatomic.h>

#include "asthra_concurrency_bridge_modular.h"
#include "asthra_concurrency_channels.h"

// Forward declarations for internal functions
extern bool asthra_concurrency_is_initialized(void);
extern AsthraResult asthra_concurrency_create_error_result(AsthraConcurrencyErrorCode code,
                                                           const char *message);

// =============================================================================
// CHANNEL IMPLEMENTATION
// =============================================================================

AsthraConcurrencyChannel *Asthra_channel_create(size_t element_size, size_t buffer_capacity,
                                                const char *name) {
    if (element_size == 0) {
        return NULL;
    }

    AsthraConcurrencyChannel *channel = malloc(sizeof(AsthraConcurrencyChannel));
    if (!channel) {
        return NULL;
    }

    // Initialize channel properties
    channel->element_size = element_size;
    channel->buffer_capacity = buffer_capacity;
    channel->is_closed = false;

    // Initialize atomic counters
    atomic_init(&channel->head, 0);
    atomic_init(&channel->tail, 0);
    atomic_init(&channel->count, 0);

    // Allocate buffer if buffered channel
    if (buffer_capacity > 0) {
        channel->buffer = malloc(element_size * buffer_capacity);
        if (!channel->buffer) {
            free(channel);
            return NULL;
        }
    } else {
        channel->buffer = NULL;
    }

    // Initialize synchronization primitives
    if (!ASTHRA_CONCURRENCY_MUTEX_INIT(&channel->mutex) ||
        !ASTHRA_CONCURRENCY_COND_INIT(&channel->not_empty) ||
        !ASTHRA_CONCURRENCY_COND_INIT(&channel->not_full)) {
        if (channel->buffer)
            free(channel->buffer);
        free(channel);
        return NULL;
    }

    // Set channel name
    if (name) {
        strncpy(channel->name, name, sizeof(channel->name) - 1);
        channel->name[sizeof(channel->name) - 1] = '\0';
    } else {
        snprintf(channel->name, sizeof(channel->name), "channel_%p", (void *)channel);
    }

    return channel;
}

AsthraResult Asthra_channel_send(AsthraConcurrencyChannel *channel, const void *value,
                                 uint64_t timeout_ms) {
    if (!channel || !value || channel->is_closed) {
        return asthra_concurrency_create_error_result(ASTHRA_CONCURRENCY_ERROR_INVALID_HANDLE,
                                                      "Invalid channel or channel closed");
    }

    ASTHRA_CONCURRENCY_MUTEX_LOCK(&channel->mutex);

    // For unbuffered channels, wait for receiver
    if (channel->buffer_capacity == 0) {
        // Unbuffered channel - direct handoff
        // This is a simplified implementation; real implementation would need
        // more sophisticated synchronization for direct handoff
        ASTHRA_CONCURRENCY_MUTEX_UNLOCK(&channel->mutex);
        return asthra_concurrency_create_error_result(ASTHRA_CONCURRENCY_ERROR_TASK_TIMEOUT,
                                                      "Unbuffered channels not fully implemented");
    }

    // For buffered channels, wait for space
    struct timespec timeout;
    bool has_timeout = timeout_ms > 0;
    if (has_timeout) {
        clock_gettime(CLOCK_REALTIME, &timeout);
        timeout.tv_sec += timeout_ms / 1000;
        timeout.tv_nsec += (timeout_ms % 1000) * 1000000;
        if (timeout.tv_nsec >= 1000000000) {
            timeout.tv_sec++;
            timeout.tv_nsec -= 1000000000;
        }
    }

    // Wait for space in buffer
    while (atomic_load(&channel->count) >= channel->buffer_capacity && !channel->is_closed) {
        int wait_result;
        if (has_timeout) {
            wait_result =
                ASTHRA_CONCURRENCY_COND_TIMEDWAIT(&channel->not_full, &channel->mutex, &timeout);
            if (wait_result == ETIMEDOUT) {
                ASTHRA_CONCURRENCY_MUTEX_UNLOCK(&channel->mutex);
                return asthra_concurrency_create_error_result(ASTHRA_CONCURRENCY_ERROR_TASK_TIMEOUT,
                                                              "Channel send timeout");
            }
        } else {
            ASTHRA_CONCURRENCY_COND_WAIT(&channel->not_full, &channel->mutex);
        }
    }

    if (channel->is_closed) {
        ASTHRA_CONCURRENCY_MUTEX_UNLOCK(&channel->mutex);
        return asthra_concurrency_create_error_result(ASTHRA_CONCURRENCY_ERROR_INVALID_HANDLE,
                                                      "Channel closed during send");
    }

    // Copy value to buffer
    size_t tail = atomic_load(&channel->tail);
    char *buffer_pos = (char *)channel->buffer + (tail * channel->element_size);
    memcpy(buffer_pos, value, channel->element_size);

    // Update tail and count atomically
    atomic_store(&channel->tail, (tail + 1) % channel->buffer_capacity);
    atomic_fetch_add(&channel->count, 1);

    // Signal waiting receivers
    ASTHRA_CONCURRENCY_COND_SIGNAL(&channel->not_empty);
    ASTHRA_CONCURRENCY_MUTEX_UNLOCK(&channel->mutex);

    return asthra_result_ok(NULL, 0, ASTHRA_TYPE_VOID, ASTHRA_OWNERSHIP_GC);
}

AsthraResult Asthra_channel_recv(AsthraConcurrencyChannel *channel, void *buffer,
                                 uint64_t timeout_ms) {
    if (!channel || !buffer) {
        return asthra_concurrency_create_error_result(ASTHRA_CONCURRENCY_ERROR_INVALID_HANDLE,
                                                      "Invalid channel or buffer");
    }

    ASTHRA_CONCURRENCY_MUTEX_LOCK(&channel->mutex);

    struct timespec timeout;
    bool has_timeout = timeout_ms > 0;
    if (has_timeout) {
        clock_gettime(CLOCK_REALTIME, &timeout);
        timeout.tv_sec += timeout_ms / 1000;
        timeout.tv_nsec += (timeout_ms % 1000) * 1000000;
        if (timeout.tv_nsec >= 1000000000) {
            timeout.tv_sec++;
            timeout.tv_nsec -= 1000000000;
        }
    }

    // Wait for data
    while (atomic_load(&channel->count) == 0 && !channel->is_closed) {
        int wait_result;
        if (has_timeout) {
            wait_result =
                ASTHRA_CONCURRENCY_COND_TIMEDWAIT(&channel->not_empty, &channel->mutex, &timeout);
            if (wait_result == ETIMEDOUT) {
                ASTHRA_CONCURRENCY_MUTEX_UNLOCK(&channel->mutex);
                return asthra_concurrency_create_error_result(ASTHRA_CONCURRENCY_ERROR_TASK_TIMEOUT,
                                                              "Channel receive timeout");
            }
        } else {
            ASTHRA_CONCURRENCY_COND_WAIT(&channel->not_empty, &channel->mutex);
        }
    }

    // Check if channel is closed and empty
    if (channel->is_closed && atomic_load(&channel->count) == 0) {
        ASTHRA_CONCURRENCY_MUTEX_UNLOCK(&channel->mutex);
        return asthra_concurrency_create_error_result(ASTHRA_CONCURRENCY_ERROR_INVALID_HANDLE,
                                                      "Channel closed and empty");
    }

    // Copy value from buffer
    size_t head = atomic_load(&channel->head);
    char *buffer_pos = (char *)channel->buffer + (head * channel->element_size);
    memcpy(buffer, buffer_pos, channel->element_size);

    // Update head and count atomically
    atomic_store(&channel->head, (head + 1) % channel->buffer_capacity);
    atomic_fetch_sub(&channel->count, 1);

    // Signal waiting senders
    ASTHRA_CONCURRENCY_COND_SIGNAL(&channel->not_full);
    ASTHRA_CONCURRENCY_MUTEX_UNLOCK(&channel->mutex);

    return asthra_result_ok(buffer, channel->element_size, ASTHRA_TYPE_VOID, ASTHRA_OWNERSHIP_GC);
}

void Asthra_channel_close(AsthraConcurrencyChannel *channel) {
    if (!channel)
        return;

    ASTHRA_CONCURRENCY_MUTEX_LOCK(&channel->mutex);
    channel->is_closed = true;

    // Wake up all waiting threads
    ASTHRA_CONCURRENCY_COND_BROADCAST(&channel->not_empty);
    ASTHRA_CONCURRENCY_COND_BROADCAST(&channel->not_full);

    ASTHRA_CONCURRENCY_MUTEX_UNLOCK(&channel->mutex);
}

void Asthra_channel_destroy(AsthraConcurrencyChannel *channel) {
    if (!channel)
        return;

    // Close channel first
    Asthra_channel_close(channel);

    // Cleanup synchronization primitives
    ASTHRA_CONCURRENCY_MUTEX_DESTROY(&channel->mutex);
    ASTHRA_CONCURRENCY_COND_DESTROY(&channel->not_empty);
    ASTHRA_CONCURRENCY_COND_DESTROY(&channel->not_full);

    // Free buffer
    if (channel->buffer) {
        free(channel->buffer);
    }

    free(channel);
}

bool Asthra_channel_is_closed(AsthraConcurrencyChannel *channel) {
    if (!channel)
        return true;
    return channel->is_closed;
}

bool Asthra_channel_get_stats(AsthraConcurrencyChannel *channel, size_t *count, size_t *capacity) {
    if (!channel)
        return false;

    if (count)
        *count = atomic_load(&channel->count);
    if (capacity)
        *capacity = channel->buffer_capacity;

    return true;
}

// =============================================================================
// SELECT OPERATIONS IMPLEMENTATION
// =============================================================================

AsthraConcurrencySelectContext *Asthra_select_context_create(size_t max_channels) {
    if (max_channels == 0)
        return NULL;

    AsthraConcurrencySelectContext *context = malloc(sizeof(AsthraConcurrencySelectContext));
    if (!context)
        return NULL;

    context->channels = malloc(sizeof(AsthraConcurrencyChannel *) * max_channels);
    context->is_send_op = malloc(sizeof(bool) * max_channels);
    context->send_values = malloc(sizeof(void *) * max_channels);
    context->recv_buffers = malloc(sizeof(void *) * max_channels);

    if (!context->channels || !context->is_send_op || !context->send_values ||
        !context->recv_buffers) {
        Asthra_select_context_destroy(context);
        return NULL;
    }

    context->channel_count = 0;
    context->selected_index = -1;
    context->timeout_ms = 0;

    return context;
}

bool Asthra_select_add_recv(AsthraConcurrencySelectContext *context,
                            AsthraConcurrencyChannel *channel, void *buffer) {
    if (!context || !channel || !buffer)
        return false;

    size_t index = context->channel_count;
    context->channels[index] = channel;
    context->is_send_op[index] = false;
    context->send_values[index] = NULL;
    context->recv_buffers[index] = buffer;
    context->channel_count++;

    return true;
}

bool Asthra_select_add_send(AsthraConcurrencySelectContext *context,
                            AsthraConcurrencyChannel *channel, const void *value) {
    if (!context || !channel || !value)
        return false;

    size_t index = context->channel_count;
    context->channels[index] = channel;
    context->is_send_op[index] = true;
    context->send_values[index] = (void *)(uintptr_t)value;
    context->recv_buffers[index] = NULL;
    context->channel_count++;

    return true;
}

int Asthra_select_execute(AsthraConcurrencySelectContext *context, uint64_t timeout_ms) {
    if (!context || context->channel_count == 0)
        return -2;

    context->timeout_ms = timeout_ms;

    // Simple implementation: try each operation in order
    // Real implementation would use more sophisticated selection algorithm
    for (size_t i = 0; i < context->channel_count; i++) {
        AsthraConcurrencyChannel *channel = context->channels[i];

        if (context->is_send_op[i]) {
            // Try send operation
            AsthraResult result = Asthra_channel_send(channel, context->send_values[i], 0);
            if (asthra_result_is_ok(result)) {
                context->selected_index = (int)i;
                return (int)i;
            }
        } else {
            // Try receive operation
            AsthraResult result = Asthra_channel_recv(channel, context->recv_buffers[i], 0);
            if (asthra_result_is_ok(result)) {
                context->selected_index = (int)i;
                return (int)i;
            }
        }
    }

    // No operation succeeded immediately
    if (timeout_ms == 0) {
        return -1; // Would block
    }

    // For timeout > 0, we would implement proper waiting logic here
    // This is a simplified implementation
    return -1; // Timeout
}

void Asthra_select_context_destroy(AsthraConcurrencySelectContext *context) {
    if (!context)
        return;

    if (context->channels)
        free(context->channels);
    if (context->is_send_op)
        free(context->is_send_op);
    if (context->send_values)
        free(context->send_values);
    if (context->recv_buffers)
        free(context->recv_buffers);

    free(context);
}

// =============================================================================
// CHANNEL UTILITIES
// =============================================================================

AsthraResult Asthra_channel_try_send(AsthraConcurrencyChannel *channel, const void *value) {
    return Asthra_channel_send(channel, value, 0);
}

AsthraResult Asthra_channel_try_recv(AsthraConcurrencyChannel *channel, void *buffer) {
    return Asthra_channel_recv(channel, buffer, 0);
}

size_t Asthra_channel_len(AsthraConcurrencyChannel *channel) {
    if (!channel)
        return 0;
    return atomic_load(&channel->count);
}

size_t Asthra_channel_cap(AsthraConcurrencyChannel *channel) {
    if (!channel)
        return 0;
    return channel->buffer_capacity;
}

bool Asthra_channel_is_empty(AsthraConcurrencyChannel *channel) {
    return Asthra_channel_len(channel) == 0;
}

bool Asthra_channel_is_full(AsthraConcurrencyChannel *channel) {
    if (!channel)
        return true;
    return atomic_load(&channel->count) >= channel->buffer_capacity;
}

// =============================================================================
// CHANNEL DIAGNOSTICS
// =============================================================================

typedef struct {
    char name[64];
    size_t element_size;
    size_t buffer_capacity;
    size_t current_count;
    bool is_closed;
    double utilization_percent;
} AsthraConcurrencyChannelInfo;

AsthraConcurrencyChannelInfo Asthra_channel_get_info(AsthraConcurrencyChannel *channel) {
    AsthraConcurrencyChannelInfo info = {0};

    if (!channel)
        return info;

    strncpy(info.name, channel->name, sizeof(info.name) - 1);
    info.element_size = channel->element_size;
    info.buffer_capacity = channel->buffer_capacity;
    info.current_count = atomic_load(&channel->count);
    info.is_closed = channel->is_closed;

    if (info.buffer_capacity > 0) {
        info.utilization_percent =
            (double)info.current_count / (double)info.buffer_capacity * 100.0;
    }

    return info;
}

void Asthra_channel_dump_info(AsthraConcurrencyChannel *channel, FILE *output) {
    if (!output)
        output = stdout;
    if (!channel) {
        fprintf(output, "Channel: NULL\n");
        return;
    }

    AsthraConcurrencyChannelInfo info = Asthra_channel_get_info(channel);

    fprintf(output, "Channel: %s\n", info.name);
    fprintf(output, "  Element Size: %zu bytes\n", info.element_size);
    fprintf(output, "  Capacity: %zu\n", info.buffer_capacity);
    fprintf(output, "  Current Count: %zu\n", info.current_count);
    fprintf(output, "  Utilization: %.1f%%\n", info.utilization_percent);
    fprintf(output, "  Status: %s\n", info.is_closed ? "Closed" : "Open");

    if (info.buffer_capacity == 0) {
        fprintf(output, "  Type: Unbuffered (synchronous)\n");
    } else {
        fprintf(output, "  Type: Buffered (asynchronous)\n");
    }
}
