#ifndef ASTHRA_DEPENDENCY_TRACKER_H
#define ASTHRA_DEPENDENCY_TRACKER_H

#include <pthread.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <time.h>

typedef struct {
    char *file_path;
    char **dependencies; // Files this file depends on
    size_t dependency_count;
    char **dependents; // Files that depend on this file
    size_t dependent_count;
    time_t last_modified;
    size_t content_hash;
} FileDependencyNode;

typedef struct {
    FileDependencyNode *nodes;
    size_t node_count;
    size_t capacity;

    // Quick lookup tables
    void *file_to_node_map; // Hash map for O(1) lookup

    pthread_rwlock_t graph_lock;
} DependencyGraph;

// Core dependency tracking
DependencyGraph *dependency_graph_create(void);
void dependency_graph_destroy(DependencyGraph *graph);
bool dependency_graph_add_file(DependencyGraph *graph, const char *file_path);
bool dependency_graph_add_dependency(DependencyGraph *graph, const char *file,
                                     const char *dependency);
char **dependency_graph_get_changed_files(DependencyGraph *graph, const char *file, size_t *count);
bool dependency_graph_is_file_stale(DependencyGraph *graph, const char *file);

// Utility functions
uint32_t dependency_graph_hash(const char *str);
FileDependencyNode *dependency_graph_find_node(DependencyGraph *graph, const char *file_path);
bool dependency_graph_update_file_timestamp(DependencyGraph *graph, const char *file_path);
size_t dependency_graph_calculate_content_hash(const char *file_path);

#endif // ASTHRA_DEPENDENCY_TRACKER_H
