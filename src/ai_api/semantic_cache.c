#include "semantic_cache.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Default cache configuration
CacheConfig semantic_cache_default_config(void) {
    CacheConfig config = {
        .max_entries = 1000,      // 1000 cache entries
        .max_memory_mb = 50,      // 50MB memory limit
        .ttl_seconds = 300,       // 5 minute TTL
        .enable_statistics = true // Enable performance tracking
    };
    return config;
}

// Hash function for cache keys (djb2 algorithm)
uint32_t semantic_cache_hash(const char *key) {
    if (!key)
        return 0;

    uint32_t hash = 5381;
    unsigned char c;
    while ((c = (unsigned char)*key++)) {
        hash = ((hash << 5) + hash) + (uint32_t)c; // hash * 33 + c
    }
    return hash;
}

// Create semantic cache
SemanticCache *semantic_cache_create(const CacheConfig *config) {
    if (!config)
        return NULL;

    SemanticCache *cache = malloc(sizeof(SemanticCache));
    if (!cache)
        return NULL;

    // Initialize hash table (use prime number for better distribution)
    cache->hash_size = 1009; // Prime number close to 1000
    cache->hash_table = calloc(cache->hash_size, sizeof(CacheEntry *));
    if (!cache->hash_table) {
        free(cache);
        return NULL;
    }

    // Initialize LRU list
    cache->lru_head = NULL;
    cache->lru_tail = NULL;

    // Set configuration
    cache->capacity = config->max_entries;
    cache->count = 0;
    cache->max_memory = config->max_memory_mb * 1024 * 1024; // Convert to bytes

    // Initialize statistics
    cache->hits = 0;
    cache->misses = 0;
    cache->evictions = 0;
    cache->memory_usage = 0;

    // Initialize thread safety
    if (pthread_rwlock_init(&cache->cache_lock, NULL) != 0) {
        free(cache->hash_table);
        free(cache);
        return NULL;
    }

    return cache;
}

// Destroy semantic cache
void semantic_cache_destroy(SemanticCache *cache) {
    if (!cache)
        return;

    pthread_rwlock_wrlock(&cache->cache_lock);

    // Clear all entries
    semantic_cache_clear(cache);

    // Cleanup hash table
    free(cache->hash_table);

    pthread_rwlock_unlock(&cache->cache_lock);
    pthread_rwlock_destroy(&cache->cache_lock);

    free(cache);
}

// Find cache entry by key
CacheEntry *semantic_cache_find_entry(SemanticCache *cache, const char *key) {
    if (!cache || !key)
        return NULL;

    uint32_t hash = semantic_cache_hash(key);
    size_t index = (size_t)hash % cache->hash_size;

    CacheEntry *entry = cache->hash_table[index];
    while (entry) {
        if (strcmp(entry->key, key) == 0) {
            return entry;
        }
        entry = entry->next;
    }

    return NULL;
}

// Move entry to head of LRU list (most recently used)
void semantic_cache_move_to_head(SemanticCache *cache, CacheEntry *entry) {
    if (!cache || !entry)
        return;

    // If already at head, nothing to do
    if (cache->lru_head == entry)
        return;

    // Remove from current position
    if (entry->prev) {
        entry->prev->next = entry->next;
    }
    if (entry->next) {
        entry->next->prev = entry->prev;
    }
    if (cache->lru_tail == entry) {
        cache->lru_tail = entry->prev;
    }

    // Add to head
    entry->prev = NULL;
    entry->next = cache->lru_head;
    if (cache->lru_head) {
        cache->lru_head->prev = entry;
    }
    cache->lru_head = entry;

    // If list was empty, this is also the tail
    if (!cache->lru_tail) {
        cache->lru_tail = entry;
    }
}

// Remove entry from cache
void semantic_cache_remove_entry(SemanticCache *cache, CacheEntry *entry) {
    if (!cache || !entry)
        return;

    // Remove from hash table
    uint32_t hash = semantic_cache_hash(entry->key);
    size_t index = (size_t)hash % cache->hash_size;

    CacheEntry **slot = &cache->hash_table[index];
    while (*slot && *slot != entry) {
        slot = &((*slot)->next);
    }
    if (*slot) {
        *slot = entry->next;
    }

    // Remove from LRU list
    if (entry->prev) {
        entry->prev->next = entry->next;
    } else {
        cache->lru_head = entry->next;
    }

    if (entry->next) {
        entry->next->prev = entry->prev;
    } else {
        cache->lru_tail = entry->prev;
    }

    // Update memory usage
    cache->memory_usage -= entry->data_size + strlen(entry->key) + 1 + sizeof(CacheEntry);

    // Free entry
    free(entry->key);
    free(entry->data);
    free(entry);

    cache->count--;
}

// Evict LRU entries to make space
size_t semantic_cache_evict_lru(SemanticCache *cache, size_t target_count) {
    if (!cache)
        return 0;

    size_t evicted = 0;

    while (cache->count > target_count && cache->lru_tail) {
        CacheEntry *to_evict = cache->lru_tail;
        semantic_cache_remove_entry(cache, to_evict);
        evicted++;
        cache->evictions++;
    }

    return evicted;
}

// Get data from cache
bool semantic_cache_get(SemanticCache *cache, const char *key, void **data, size_t *size) {
    if (!cache || !key || !data || !size)
        return false;

    pthread_rwlock_rdlock(&cache->cache_lock);

    CacheEntry *entry = semantic_cache_find_entry(cache, key);

    if (!entry || !entry->is_valid) {
        cache->misses++;
        pthread_rwlock_unlock(&cache->cache_lock);
        return false;
    }

    // Check TTL (time-to-live)
    time_t now = time(NULL);
    if (now - entry->timestamp > 300) { // 5 minute TTL
        entry->is_valid = false;
        cache->misses++;
        pthread_rwlock_unlock(&cache->cache_lock);
        return false;
    }

    // Copy data
    *data = malloc(entry->data_size);
    if (!*data) {
        cache->misses++;
        pthread_rwlock_unlock(&cache->cache_lock);
        return false;
    }

    memcpy(*data, entry->data, entry->data_size);
    *size = entry->data_size;

    // Update access time and move to head
    entry->timestamp = now;

    pthread_rwlock_unlock(&cache->cache_lock);

    // Upgrade to write lock to update LRU
    pthread_rwlock_wrlock(&cache->cache_lock);
    semantic_cache_move_to_head(cache, entry);
    cache->hits++;
    pthread_rwlock_unlock(&cache->cache_lock);

    return true;
}

// Put data into cache
bool semantic_cache_put(SemanticCache *cache, const char *key, const void *data, size_t size) {
    if (!cache || !key || !data || size == 0)
        return false;

    pthread_rwlock_wrlock(&cache->cache_lock);

    // Check if entry already exists
    CacheEntry *existing = semantic_cache_find_entry(cache, key);
    if (existing) {
        // Update existing entry
        free(existing->data);
        existing->data = malloc(size);
        if (!existing->data) {
            pthread_rwlock_unlock(&cache->cache_lock);
            return false;
        }

        memcpy(existing->data, data, size);
        existing->data_size = size;
        existing->timestamp = time(NULL);
        existing->is_valid = true;

        semantic_cache_move_to_head(cache, existing);
        pthread_rwlock_unlock(&cache->cache_lock);
        return true;
    }

    // Check memory limit
    size_t entry_memory = size + strlen(key) + 1 + sizeof(CacheEntry);
    if (cache->memory_usage + entry_memory > cache->max_memory) {
        // Evict entries to make space
        size_t target_memory = (size_t)((double)cache->max_memory * 0.8); // Evict to 80% capacity
        while (cache->memory_usage + entry_memory > target_memory && cache->lru_tail) {
            semantic_cache_remove_entry(cache, cache->lru_tail);
        }
    }

    // Check capacity limit
    if (cache->count >= cache->capacity) {
        semantic_cache_evict_lru(cache, cache->capacity - 1);
    }

    // Create new entry
    CacheEntry *entry = malloc(sizeof(CacheEntry));
    if (!entry) {
        pthread_rwlock_unlock(&cache->cache_lock);
        return false;
    }

    entry->key = strdup(key);
    entry->data = malloc(size);
    if (!entry->key || !entry->data) {
        free(entry->key);
        free(entry->data);
        free(entry);
        pthread_rwlock_unlock(&cache->cache_lock);
        return false;
    }

    memcpy(entry->data, data, size);
    entry->data_size = size;
    entry->timestamp = time(NULL);
    entry->is_valid = true;
    entry->prev = NULL;
    entry->next = NULL;

    // Add to hash table
    uint32_t hash = semantic_cache_hash(key);
    size_t index = (size_t)hash % cache->hash_size;
    entry->next = cache->hash_table[index];
    cache->hash_table[index] = entry;

    // Add to head of LRU list
    semantic_cache_move_to_head(cache, entry);

    // Update statistics
    cache->count++;
    cache->memory_usage += entry_memory;

    pthread_rwlock_unlock(&cache->cache_lock);
    return true;
}

// Invalidate cache entry
void semantic_cache_invalidate(SemanticCache *cache, const char *key) {
    if (!cache || !key)
        return;

    pthread_rwlock_wrlock(&cache->cache_lock);

    CacheEntry *entry = semantic_cache_find_entry(cache, key);
    if (entry) {
        entry->is_valid = false;
    }

    pthread_rwlock_unlock(&cache->cache_lock);
}

// Clear all cache entries
void semantic_cache_clear(SemanticCache *cache) {
    if (!cache)
        return;

    pthread_rwlock_wrlock(&cache->cache_lock);

    while (cache->lru_head) {
        semantic_cache_remove_entry(cache, cache->lru_head);
    }

    // Reset statistics
    cache->hits = 0;
    cache->misses = 0;
    cache->evictions = 0;
    cache->memory_usage = 0;

    pthread_rwlock_unlock(&cache->cache_lock);
}

// Get cache statistics
CacheStatistics semantic_cache_get_statistics(SemanticCache *cache) {
    CacheStatistics stats = {0};

    if (!cache)
        return stats;

    pthread_rwlock_rdlock(&cache->cache_lock);

    stats.total_hits = cache->hits;
    stats.total_misses = cache->misses;
    stats.total_evictions = cache->evictions;
    stats.current_entries = cache->count;
    stats.memory_usage_bytes = cache->memory_usage;

    if (cache->hits + cache->misses > 0) {
        stats.hit_rate = (double)cache->hits / (double)(cache->hits + cache->misses);
    }

    pthread_rwlock_unlock(&cache->cache_lock);

    return stats;
}

// Get cache hit rate
double semantic_cache_get_hit_rate(SemanticCache *cache) {
    if (!cache)
        return 0.0;

    pthread_rwlock_rdlock(&cache->cache_lock);

    double hit_rate = 0.0;
    if (cache->hits + cache->misses > 0) {
        hit_rate = (double)cache->hits / (double)(cache->hits + cache->misses);
    }

    pthread_rwlock_unlock(&cache->cache_lock);

    return hit_rate;
}

// Get memory usage
size_t semantic_cache_get_memory_usage(SemanticCache *cache) {
    if (!cache)
        return 0;

    pthread_rwlock_rdlock(&cache->cache_lock);
    size_t usage = cache->memory_usage;
    pthread_rwlock_unlock(&cache->cache_lock);

    return usage;
}

// Reset cache statistics
void semantic_cache_reset_statistics(SemanticCache *cache) {
    if (!cache)
        return;

    pthread_rwlock_wrlock(&cache->cache_lock);

    cache->hits = 0;
    cache->misses = 0;
    cache->evictions = 0;

    pthread_rwlock_unlock(&cache->cache_lock);
}
