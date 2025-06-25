#include "fast_semantic_cache.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

#define INITIAL_FILE_CAPACITY 128
#define DEFAULT_TTL_SECONDS 300 // 5 minutes

// Create a semantic analysis cache entry
SemanticAnalysisCache *semantic_analysis_cache_create(void) {
    SemanticAnalysisCache *cache = malloc(sizeof(SemanticAnalysisCache));
    if (!cache)
        return NULL;

    cache->symbols = NULL;
    cache->types = NULL;
    cache->type_count = 0;
    cache->diagnostics = NULL;
    cache->diagnostic_count = 0;
    cache->cache_timestamp = time(NULL);
    cache->file_content_hash = 0;
    cache->is_valid = false;
    cache->analysis_time_ms = 0.0;
    cache->memory_used_bytes = sizeof(SemanticAnalysisCache);

    return cache;
}

// Destroy a semantic analysis cache entry
void semantic_analysis_cache_destroy(SemanticAnalysisCache *cache) {
    if (!cache)
        return;

    // Note: We don't free symbols, types, or diagnostics here
    // as they might be shared or owned by other components
    // The caller is responsible for managing these resources

    free(cache);
}

// Calculate file content hash for cache validation
size_t fast_semantic_cache_calculate_file_hash(const char *file_path) {
    if (!file_path)
        return 0;

    FILE *file = fopen(file_path, "rb");
    if (!file)
        return 0;

    size_t hash = 5381;
    int c;

    while ((c = fgetc(file)) != EOF) {
        hash = ((hash << 5) + hash) + (size_t)c;
    }

    fclose(file);
    return hash;
}

// Create fast semantic cache manager
FastSemanticCacheManager *fast_semantic_cache_create(size_t max_memory_mb) {
    FastSemanticCacheManager *manager = malloc(sizeof(FastSemanticCacheManager));
    if (!manager)
        return NULL;

    // Create underlying cache with reasonable defaults
    CacheConfig config = semantic_cache_default_config();
    config.max_memory_mb = max_memory_mb / 2; // Reserve half for file cache

    manager->base_cache = semantic_cache_create(&config);
    if (!manager->base_cache) {
        free(manager);
        return NULL;
    }

    // Initialize file-specific cache
    manager->file_entries = malloc(sizeof(SemanticAnalysisCache *) * INITIAL_FILE_CAPACITY);
    if (!manager->file_entries) {
        semantic_cache_destroy(manager->base_cache);
        free(manager);
        return NULL;
    }

    // Initialize all entries to NULL
    for (size_t i = 0; i < INITIAL_FILE_CAPACITY; i++) {
        manager->file_entries[i] = NULL;
    }

    manager->file_entry_count = 0;
    manager->file_capacity = INITIAL_FILE_CAPACITY;
    manager->max_memory_usage = max_memory_mb * 1024 * 1024; // Convert to bytes
    manager->current_memory_usage = 0;
    manager->default_ttl = DEFAULT_TTL_SECONDS;

    // Initialize statistics
    manager->total_file_checks = 0;
    manager->cache_hits = 0;
    manager->cache_misses = 0;
    manager->total_analysis_time = 0.0;

    // Initialize thread safety
    if (pthread_rwlock_init(&manager->fast_cache_lock, NULL) != 0) {
        free(manager->file_entries);
        semantic_cache_destroy(manager->base_cache);
        free(manager);
        return NULL;
    }

    return manager;
}

// Destroy fast semantic cache manager
void fast_semantic_cache_destroy(FastSemanticCacheManager *cache) {
    if (!cache)
        return;

    pthread_rwlock_wrlock(&cache->fast_cache_lock);

    // Destroy all file entries
    for (size_t i = 0; i < cache->file_entry_count; i++) {
        if (cache->file_entries[i]) {
            semantic_analysis_cache_destroy(cache->file_entries[i]);
        }
    }
    free(cache->file_entries);

    // Destroy base cache
    semantic_cache_destroy(cache->base_cache);

    pthread_rwlock_unlock(&cache->fast_cache_lock);
    pthread_rwlock_destroy(&cache->fast_cache_lock);

    free(cache);
}

// Find file entry by path
static SemanticAnalysisCache *find_file_entry(FastSemanticCacheManager *cache,
                                              const char *file_path) {
    // Simple linear search for now - could be optimized with hash table
    // For now, we'll use the file hash as the primary identifier
    size_t target_hash = fast_semantic_cache_calculate_file_hash(file_path);

    for (size_t i = 0; i < cache->file_entry_count; i++) {
        if (cache->file_entries[i] && cache->file_entries[i]->file_content_hash == target_hash) {
            // Additional validation would be nice but for now this is sufficient
            return cache->file_entries[i];
        }
    }
    return NULL;
}

// Check if cache has memory capacity
bool fast_semantic_cache_check_memory_limit(FastSemanticCacheManager *cache,
                                            size_t additional_bytes) {
    if (!cache)
        return false;
    return (cache->current_memory_usage + additional_bytes) <= cache->max_memory_usage;
}

// Get cached semantic analysis for file
SemanticAnalysisCache *fast_semantic_cache_get_file(FastSemanticCacheManager *cache,
                                                    const char *file_path) {
    if (!cache || !file_path)
        return NULL;

    pthread_rwlock_rdlock(&cache->fast_cache_lock);

    cache->total_file_checks++;

    SemanticAnalysisCache *entry = find_file_entry(cache, file_path);
    if (!entry) {
        cache->cache_misses++;
        pthread_rwlock_unlock(&cache->fast_cache_lock);
        return NULL;
    }

    // Check if entry is still valid
    struct stat file_stat;
    if (stat(file_path, &file_stat) != 0) {
        // File doesn't exist, invalidate cache
        entry->is_valid = false;
        cache->cache_misses++;
        pthread_rwlock_unlock(&cache->fast_cache_lock);
        return NULL;
    }

    // Check modification time
    if (file_stat.st_mtime > entry->cache_timestamp) {
        entry->is_valid = false;
        cache->cache_misses++;
        pthread_rwlock_unlock(&cache->fast_cache_lock);
        return NULL;
    }

    // Check content hash
    size_t current_hash = fast_semantic_cache_calculate_file_hash(file_path);
    if (current_hash != entry->file_content_hash) {
        entry->is_valid = false;
        cache->cache_misses++;
        pthread_rwlock_unlock(&cache->fast_cache_lock);
        return NULL;
    }

    cache->cache_hits++;
    pthread_rwlock_unlock(&cache->fast_cache_lock);
    return entry;
}

// Store semantic analysis result in cache
bool fast_semantic_cache_store_file(FastSemanticCacheManager *cache, const char *file_path,
                                    SemanticAnalysisCache *result) {
    if (!cache || !file_path || !result)
        return false;

    pthread_rwlock_wrlock(&cache->fast_cache_lock);

    // Check memory limit
    if (!fast_semantic_cache_check_memory_limit(cache, result->memory_used_bytes)) {
        // Try to cleanup expired entries first
        fast_semantic_cache_cleanup_expired(cache);

        if (!fast_semantic_cache_check_memory_limit(cache, result->memory_used_bytes)) {
            pthread_rwlock_unlock(&cache->fast_cache_lock);
            return false; // Still not enough memory
        }
    }

    // Check if we need to expand capacity
    if (cache->file_entry_count >= cache->file_capacity) {
        size_t new_capacity = cache->file_capacity * 2;
        SemanticAnalysisCache **new_entries =
            realloc(cache->file_entries, sizeof(SemanticAnalysisCache *) * new_capacity);
        if (!new_entries) {
            pthread_rwlock_unlock(&cache->fast_cache_lock);
            return false;
        }

        // Initialize new entries to NULL
        for (size_t i = cache->file_capacity; i < new_capacity; i++) {
            new_entries[i] = NULL;
        }

        cache->file_entries = new_entries;
        cache->file_capacity = new_capacity;
    }

    // Find existing entry or create new one
    SemanticAnalysisCache *existing = find_file_entry(cache, file_path);
    if (existing) {
        // Update existing entry
        cache->current_memory_usage -= existing->memory_used_bytes;
        semantic_analysis_cache_destroy(existing);
    } else {
        cache->file_entry_count++;
    }

    cache->file_entries[cache->file_entry_count - 1] = result;
    cache->current_memory_usage += result->memory_used_bytes;

    // Update result metadata
    result->cache_timestamp = time(NULL);
    result->file_content_hash = fast_semantic_cache_calculate_file_hash(file_path);
    result->is_valid = true;

    pthread_rwlock_unlock(&cache->fast_cache_lock);
    return true;
}

// Invalidate cached analysis for file
void fast_semantic_cache_invalidate_file(FastSemanticCacheManager *cache, const char *file_path) {
    if (!cache || !file_path)
        return;

    pthread_rwlock_wrlock(&cache->fast_cache_lock);

    SemanticAnalysisCache *entry = find_file_entry(cache, file_path);
    if (entry) {
        entry->is_valid = false;
    }

    pthread_rwlock_unlock(&cache->fast_cache_lock);
}

// Check if file cache is valid
bool fast_semantic_cache_is_file_valid(FastSemanticCacheManager *cache, const char *file_path,
                                       time_t file_mtime) {
    if (!cache || !file_path)
        return false;

    pthread_rwlock_rdlock(&cache->fast_cache_lock);

    SemanticAnalysisCache *entry = find_file_entry(cache, file_path);
    if (!entry || !entry->is_valid) {
        pthread_rwlock_unlock(&cache->fast_cache_lock);
        return false;
    }

    bool valid = entry->cache_timestamp >= file_mtime;

    pthread_rwlock_unlock(&cache->fast_cache_lock);
    return valid;
}

// Invalidate cache for dependent files
void fast_semantic_cache_invalidate_dependents(FastSemanticCacheManager *cache,
                                               const char *file_path, char **dependent_files,
                                               size_t dependent_count) {
    if (!cache || !file_path)
        return;

    // Invalidate the file itself
    fast_semantic_cache_invalidate_file(cache, file_path);

    // Invalidate all dependent files
    for (size_t i = 0; i < dependent_count; i++) {
        if (dependent_files[i]) {
            fast_semantic_cache_invalidate_file(cache, dependent_files[i]);
        }
    }
}

// Cleanup expired cache entries
void fast_semantic_cache_cleanup_expired(FastSemanticCacheManager *cache) {
    if (!cache)
        return;

    time_t current_time = time(NULL);

    for (size_t i = 0; i < cache->file_entry_count; i++) {
        SemanticAnalysisCache *entry = cache->file_entries[i];
        if (entry && (current_time - entry->cache_timestamp) > cache->default_ttl) {
            cache->current_memory_usage -= entry->memory_used_bytes;
            semantic_analysis_cache_destroy(entry);
            cache->file_entries[i] = NULL;
        }
    }
}

// Get cache statistics
FastCacheStatistics fast_semantic_cache_get_statistics(FastSemanticCacheManager *cache) {
    FastCacheStatistics stats = {0};

    if (!cache)
        return stats;

    pthread_rwlock_rdlock(&cache->fast_cache_lock);

    stats.total_files_cached = cache->file_entry_count;
    stats.cache_hits = cache->cache_hits;
    stats.cache_misses = cache->cache_misses;
    stats.hit_rate = (cache->total_file_checks > 0)
                         ? (double)cache->cache_hits / (double)cache->total_file_checks
                         : 0.0;
    stats.memory_usage_bytes = cache->current_memory_usage;
    stats.average_analysis_time_ms =
        (cache->total_file_checks > 0)
            ? (double)cache->total_analysis_time / (double)cache->total_file_checks
            : 0.0;

    // Calculate cache efficiency (hit rate * memory efficiency)
    double memory_efficiency =
        (cache->max_memory_usage > 0)
            ? 1.0 - ((double)cache->current_memory_usage / (double)cache->max_memory_usage)
            : 0.0;
    stats.cache_efficiency = stats.hit_rate * memory_efficiency;

    pthread_rwlock_unlock(&cache->fast_cache_lock);

    return stats;
}

// Reset cache statistics
void fast_semantic_cache_reset_statistics(FastSemanticCacheManager *cache) {
    if (!cache)
        return;

    pthread_rwlock_wrlock(&cache->fast_cache_lock);

    cache->total_file_checks = 0;
    cache->cache_hits = 0;
    cache->cache_misses = 0;
    cache->total_analysis_time = 0.0;

    pthread_rwlock_unlock(&cache->fast_cache_lock);
}

// Populate semantic analysis cache with results
bool semantic_analysis_cache_populate(SemanticAnalysisCache *cache, SymbolTable *symbols,
                                      TypeDescriptor **types, size_t type_count,
                                      EnhancedDiagnostic *diagnostics, size_t diagnostic_count) {
    if (!cache)
        return false;

    cache->symbols = symbols;
    cache->types = types;
    cache->type_count = type_count;
    cache->diagnostics = diagnostics;
    cache->diagnostic_count = diagnostic_count;

    // Calculate memory usage
    cache->memory_used_bytes = sizeof(SemanticAnalysisCache);
    if (symbols) {
        cache->memory_used_bytes += sizeof(SymbolTable); // Simplified calculation
    }
    if (types) {
        cache->memory_used_bytes += sizeof(TypeDescriptor *) * type_count;
    }
    if (diagnostics) {
        cache->memory_used_bytes += sizeof(EnhancedDiagnostic) * diagnostic_count;
    }

    cache->is_valid = true;
    cache->cache_timestamp = time(NULL);

    return true;
}
