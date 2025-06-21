#ifndef ASTHRA_AI_SEMANTIC_CACHE_H
#define ASTHRA_AI_SEMANTIC_CACHE_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include <pthread.h>

// Cache entry structure
typedef struct CacheEntry {
    char *key;                    // Cache key (symbol name, expression, etc.)
    void *data;                   // Cached data
    size_t data_size;             // Size of cached data
    time_t timestamp;             // When entry was created/accessed
    bool is_valid;                // Whether entry is still valid
    struct CacheEntry *prev;      // LRU doubly-linked list
    struct CacheEntry *next;      // LRU doubly-linked list
} CacheEntry;

// Semantic cache structure with LRU eviction
typedef struct {
    CacheEntry **hash_table;      // Hash table for O(1) lookup
    CacheEntry *lru_head;         // Most recently used
    CacheEntry *lru_tail;         // Least recently used
    size_t capacity;              // Maximum number of entries
    size_t count;                 // Current number of entries
    size_t hash_size;             // Hash table size
    pthread_rwlock_t cache_lock;  // Reader-writer lock for thread safety
    
    // Performance statistics
    size_t hits;                  // Cache hits
    size_t misses;                // Cache misses
    size_t evictions;             // Number of evictions
    size_t memory_usage;          // Current memory usage in bytes
    size_t max_memory;            // Maximum memory limit
} SemanticCache;

// Cache configuration
typedef struct {
    size_t max_entries;           // Maximum number of cache entries
    size_t max_memory_mb;         // Maximum memory usage in MB
    time_t ttl_seconds;           // Time-to-live for entries
    bool enable_statistics;       // Whether to track performance stats
} CacheConfig;

// Cache statistics
typedef struct {
    size_t total_hits;
    size_t total_misses;
    size_t total_evictions;
    double hit_rate;              // hits / (hits + misses)
    size_t current_entries;
    size_t memory_usage_bytes;
    double average_lookup_time_ms;
} CacheStatistics;

// Cache lifecycle functions
SemanticCache *semantic_cache_create(const CacheConfig *config);
void semantic_cache_destroy(SemanticCache *cache);

// Cache operations
bool semantic_cache_get(SemanticCache *cache, const char *key, void **data, size_t *size);
bool semantic_cache_put(SemanticCache *cache, const char *key, const void *data, size_t size);
void semantic_cache_invalidate(SemanticCache *cache, const char *key);
void semantic_cache_clear(SemanticCache *cache);

// Cache management
bool semantic_cache_is_valid_entry(SemanticCache *cache, const char *key);
void semantic_cache_update_access_time(SemanticCache *cache, const char *key);
size_t semantic_cache_evict_lru(SemanticCache *cache, size_t target_count);

// Performance monitoring
CacheStatistics semantic_cache_get_statistics(SemanticCache *cache);
void semantic_cache_reset_statistics(SemanticCache *cache);
double semantic_cache_get_hit_rate(SemanticCache *cache);
size_t semantic_cache_get_memory_usage(SemanticCache *cache);

// Utility functions
uint32_t semantic_cache_hash(const char *key);
CacheEntry *semantic_cache_find_entry(SemanticCache *cache, const char *key);
void semantic_cache_move_to_head(SemanticCache *cache, CacheEntry *entry);
void semantic_cache_remove_entry(SemanticCache *cache, CacheEntry *entry);

// Default configuration
CacheConfig semantic_cache_default_config(void);

#endif // ASTHRA_AI_SEMANTIC_CACHE_H
