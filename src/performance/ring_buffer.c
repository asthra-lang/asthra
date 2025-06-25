#include "memory_pool_internal.h"

// =============================================================================
// RING BUFFER IMPLEMENTATION
// =============================================================================

AsthraRingBuffer *asthra_ring_buffer_create(const char *name, size_t buffer_size, size_t alignment,
                                            bool allow_overwrite) {
    if (!name || buffer_size == 0)
        return NULL;

    // Ensure minimum alignment requirements for aligned_alloc
    if (alignment == 0 || (alignment & (alignment - 1)) != 0) {
        alignment = sizeof(void *); // Use pointer size as minimum alignment
    }
    if (alignment < sizeof(void *)) {
        alignment = sizeof(void *);
    }

    // Ensure buffer size is aligned to the alignment requirement
    buffer_size = asthra_align_up(buffer_size, alignment);

    AsthraRingBuffer *buffer =
        aligned_alloc(ASTHRA_MEMORY_POOL_CACHE_LINE_SIZE, sizeof(AsthraRingBuffer));
    if (!buffer)
        return NULL;

    // Initialize buffer structure
    memset(buffer, 0, sizeof(AsthraRingBuffer));
    strncpy(buffer->name, name, sizeof(buffer->name) - 1);
    buffer->buffer_size = buffer_size;
    buffer->alignment = alignment;
    buffer->allow_overwrite = allow_overwrite;

    // Allocate memory region with proper alignment
    buffer->memory_region = aligned_alloc(alignment, buffer_size);
    if (!buffer->memory_region) {
        free(buffer);
        return NULL;
    }

    // Initialize atomic counters
    atomic_store(&buffer->write_offset, 0);
    atomic_store(&buffer->read_offset, 0);
    atomic_store(&buffer->available_bytes, 0);

    // Initialize statistics
    asthra_pool_statistics_reset(&buffer->stats);

    return buffer;
}

void asthra_ring_buffer_destroy(AsthraRingBuffer *buffer) {
    if (!buffer)
        return;

    free(buffer->memory_region);
    free(buffer);
}

size_t asthra_ring_buffer_write(AsthraRingBuffer *buffer, const void *data, size_t size) {
    if (!buffer || !data || size == 0)
        return 0;

    size_t available_write = asthra_ring_buffer_available_write(buffer);
    if (size > available_write && !buffer->allow_overwrite) {
        return 0; // Not enough space and overwrite not allowed
    }

    size_t write_offset = atomic_load(&buffer->write_offset);
    size_t bytes_to_write = (size <= buffer->buffer_size) ? size : buffer->buffer_size;

    // Handle wrap-around
    size_t bytes_to_end = buffer->buffer_size - write_offset;
    if (bytes_to_write <= bytes_to_end) {
        // No wrap-around needed
        memcpy((char *)buffer->memory_region + write_offset, data, bytes_to_write);
    } else {
        // Wrap-around needed
        memcpy((char *)buffer->memory_region + write_offset, data, bytes_to_end);
        memcpy(buffer->memory_region, (const char *)data + bytes_to_end,
               bytes_to_write - bytes_to_end);
    }

    // Update write offset
    size_t new_write_offset = (write_offset + bytes_to_write) % buffer->buffer_size;
    atomic_store(&buffer->write_offset, new_write_offset);

    // Update available bytes
    atomic_fetch_add(&buffer->available_bytes, bytes_to_write);

    // Update statistics
    atomic_fetch_add(&buffer->stats.total_allocations, 1);
    atomic_fetch_add(&buffer->stats.total_memory_allocated, bytes_to_write);

    return bytes_to_write;
}

size_t asthra_ring_buffer_read(AsthraRingBuffer *buffer, void *data, size_t size) {
    if (!buffer || !data || size == 0)
        return 0;

    size_t available_read = asthra_ring_buffer_available_read(buffer);
    if (available_read == 0)
        return 0;

    size_t bytes_to_read = (size <= available_read) ? size : available_read;
    size_t read_offset = atomic_load(&buffer->read_offset);

    // Handle wrap-around
    size_t bytes_to_end = buffer->buffer_size - read_offset;
    if (bytes_to_read <= bytes_to_end) {
        // No wrap-around needed
        memcpy(data, (char *)buffer->memory_region + read_offset, bytes_to_read);
    } else {
        // Wrap-around needed
        memcpy(data, (char *)buffer->memory_region + read_offset, bytes_to_end);
        memcpy((char *)data + bytes_to_end, buffer->memory_region, bytes_to_read - bytes_to_end);
    }

    // Update read offset
    size_t new_read_offset = (read_offset + bytes_to_read) % buffer->buffer_size;
    atomic_store(&buffer->read_offset, new_read_offset);

    // Update available bytes
    atomic_fetch_sub(&buffer->available_bytes, bytes_to_read);

    // Update statistics
    atomic_fetch_add(&buffer->stats.total_deallocations, 1);

    return bytes_to_read;
}

size_t asthra_ring_buffer_available_write(const AsthraRingBuffer *buffer) {
    if (!buffer)
        return 0;
    return buffer->buffer_size - atomic_load(&buffer->available_bytes);
}

size_t asthra_ring_buffer_available_read(const AsthraRingBuffer *buffer) {
    if (!buffer)
        return 0;
    return atomic_load(&buffer->available_bytes);
}

bool asthra_ring_buffer_is_empty(const AsthraRingBuffer *buffer) {
    return asthra_ring_buffer_available_read(buffer) == 0;
}

bool asthra_ring_buffer_is_full(const AsthraRingBuffer *buffer) {
    return asthra_ring_buffer_available_write(buffer) == 0;
}
