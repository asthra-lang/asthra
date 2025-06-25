#include "dependency_tracker.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#define INITIAL_CAPACITY 64
#define HASH_TABLE_SIZE 256

// Simple hash function for file paths
uint32_t dependency_graph_hash(const char *str) {
    uint32_t hash = 5381;
    int c;

    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + (uint32_t)c;
    }

    return hash % HASH_TABLE_SIZE;
}

// Calculate content hash for a file
size_t dependency_graph_calculate_content_hash(const char *file_path) {
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

DependencyGraph *dependency_graph_create(void) {
    DependencyGraph *graph = malloc(sizeof(DependencyGraph));
    if (!graph)
        return NULL;

    graph->nodes = malloc(sizeof(FileDependencyNode) * INITIAL_CAPACITY);
    if (!graph->nodes) {
        free(graph);
        return NULL;
    }

    graph->node_count = 0;
    graph->capacity = INITIAL_CAPACITY;

    // Initialize hash table for file lookup
    graph->file_to_node_map = calloc(HASH_TABLE_SIZE, sizeof(FileDependencyNode *));
    if (!graph->file_to_node_map) {
        free(graph->nodes);
        free(graph);
        return NULL;
    }

    if (pthread_rwlock_init(&graph->graph_lock, NULL) != 0) {
        free(graph->file_to_node_map);
        free(graph->nodes);
        free(graph);
        return NULL;
    }

    return graph;
}

void dependency_graph_destroy(DependencyGraph *graph) {
    if (!graph)
        return;

    pthread_rwlock_wrlock(&graph->graph_lock);

    // Free all nodes and their data
    for (size_t i = 0; i < graph->node_count; i++) {
        FileDependencyNode *node = &graph->nodes[i];
        free(node->file_path);

        if (node->dependencies) {
            for (size_t j = 0; j < node->dependency_count; j++) {
                free(node->dependencies[j]);
            }
            free(node->dependencies);
        }

        if (node->dependents) {
            for (size_t j = 0; j < node->dependent_count; j++) {
                free(node->dependents[j]);
            }
            free(node->dependents);
        }
    }

    free(graph->nodes);
    free(graph->file_to_node_map);

    pthread_rwlock_unlock(&graph->graph_lock);
    pthread_rwlock_destroy(&graph->graph_lock);
    free(graph);
}

FileDependencyNode *dependency_graph_find_node(DependencyGraph *graph, const char *file_path) {
    if (!graph || !file_path)
        return NULL;

    for (size_t i = 0; i < graph->node_count; i++) {
        if (strcmp(graph->nodes[i].file_path, file_path) == 0) {
            return &graph->nodes[i];
        }
    }

    return NULL;
}

bool dependency_graph_add_file(DependencyGraph *graph, const char *file_path) {
    if (!graph || !file_path)
        return false;

    pthread_rwlock_wrlock(&graph->graph_lock);

    // Check if file already exists
    if (dependency_graph_find_node(graph, file_path)) {
        pthread_rwlock_unlock(&graph->graph_lock);
        return true; // Already exists
    }

    // Expand capacity if needed
    if (graph->node_count >= graph->capacity) {
        size_t new_capacity = graph->capacity * 2;
        FileDependencyNode *new_nodes =
            realloc(graph->nodes, sizeof(FileDependencyNode) * new_capacity);
        if (!new_nodes) {
            pthread_rwlock_unlock(&graph->graph_lock);
            return false;
        }
        graph->nodes = new_nodes;
        graph->capacity = new_capacity;
    }

    // Get file modification time
    struct stat file_stat;
    time_t last_modified = 0;
    if (stat(file_path, &file_stat) == 0) {
        last_modified = file_stat.st_mtime;
    }

    // Initialize new node
    FileDependencyNode *node = &graph->nodes[graph->node_count];
    node->file_path = strdup(file_path);
    if (!node->file_path) {
        pthread_rwlock_unlock(&graph->graph_lock);
        return false;
    }

    node->dependencies = NULL;
    node->dependency_count = 0;
    node->dependents = NULL;
    node->dependent_count = 0;
    node->last_modified = last_modified;
    node->content_hash = dependency_graph_calculate_content_hash(file_path);

    graph->node_count++;

    pthread_rwlock_unlock(&graph->graph_lock);
    return true;
}

bool dependency_graph_add_dependency(DependencyGraph *graph, const char *file,
                                     const char *dependency) {
    if (!graph || !file || !dependency)
        return false;

    pthread_rwlock_wrlock(&graph->graph_lock);

    // Ensure both files exist in the graph
    if (!dependency_graph_add_file(graph, file) || !dependency_graph_add_file(graph, dependency)) {
        pthread_rwlock_unlock(&graph->graph_lock);
        return false;
    }

    FileDependencyNode *file_node = dependency_graph_find_node(graph, file);
    FileDependencyNode *dep_node = dependency_graph_find_node(graph, dependency);

    if (!file_node || !dep_node) {
        pthread_rwlock_unlock(&graph->graph_lock);
        return false;
    }

    // Check if dependency already exists
    for (size_t i = 0; i < file_node->dependency_count; i++) {
        if (strcmp(file_node->dependencies[i], dependency) == 0) {
            pthread_rwlock_unlock(&graph->graph_lock);
            return true; // Already exists
        }
    }

    // Add dependency to file_node
    char **new_deps =
        realloc(file_node->dependencies, sizeof(char *) * (file_node->dependency_count + 1));
    if (!new_deps) {
        pthread_rwlock_unlock(&graph->graph_lock);
        return false;
    }
    file_node->dependencies = new_deps;
    file_node->dependencies[file_node->dependency_count] = strdup(dependency);
    if (!file_node->dependencies[file_node->dependency_count]) {
        pthread_rwlock_unlock(&graph->graph_lock);
        return false;
    }
    file_node->dependency_count++;

    // Add file as dependent to dep_node
    char **new_dependents =
        realloc(dep_node->dependents, sizeof(char *) * (dep_node->dependent_count + 1));
    if (!new_dependents) {
        pthread_rwlock_unlock(&graph->graph_lock);
        return false;
    }
    dep_node->dependents = new_dependents;
    dep_node->dependents[dep_node->dependent_count] = strdup(file);
    if (!dep_node->dependents[dep_node->dependent_count]) {
        pthread_rwlock_unlock(&graph->graph_lock);
        return false;
    }
    dep_node->dependent_count++;

    pthread_rwlock_unlock(&graph->graph_lock);
    return true;
}

bool dependency_graph_update_file_timestamp(DependencyGraph *graph, const char *file_path) {
    if (!graph || !file_path)
        return false;

    pthread_rwlock_wrlock(&graph->graph_lock);

    FileDependencyNode *node = dependency_graph_find_node(graph, file_path);
    if (!node) {
        pthread_rwlock_unlock(&graph->graph_lock);
        return false;
    }

    struct stat file_stat;
    if (stat(file_path, &file_stat) == 0) {
        node->last_modified = file_stat.st_mtime;
        node->content_hash = dependency_graph_calculate_content_hash(file_path);
    }

    pthread_rwlock_unlock(&graph->graph_lock);
    return true;
}

bool dependency_graph_is_file_stale(DependencyGraph *graph, const char *file) {
    if (!graph || !file)
        return true;

    pthread_rwlock_rdlock(&graph->graph_lock);

    FileDependencyNode *node = dependency_graph_find_node(graph, file);
    if (!node) {
        pthread_rwlock_unlock(&graph->graph_lock);
        return true; // Unknown files are considered stale
    }

    // Check if file itself has changed
    struct stat file_stat;
    if (stat(file, &file_stat) != 0) {
        pthread_rwlock_unlock(&graph->graph_lock);
        return true; // File doesn't exist
    }

    if (file_stat.st_mtime > node->last_modified) {
        pthread_rwlock_unlock(&graph->graph_lock);
        return true; // File has been modified
    }

    // Check if any dependencies have changed
    for (size_t i = 0; i < node->dependency_count; i++) {
        if (dependency_graph_is_file_stale(graph, node->dependencies[i])) {
            pthread_rwlock_unlock(&graph->graph_lock);
            return true; // Dependency is stale
        }
    }

    pthread_rwlock_unlock(&graph->graph_lock);
    return false;
}

char **dependency_graph_get_changed_files(DependencyGraph *graph, const char *file, size_t *count) {
    if (!graph || !file || !count)
        return NULL;

    *count = 0;

    pthread_rwlock_rdlock(&graph->graph_lock);

    if (!dependency_graph_is_file_stale(graph, file)) {
        pthread_rwlock_unlock(&graph->graph_lock);
        return NULL; // No changes
    }

    // For now, return just the file itself as changed
    // In a more sophisticated implementation, this would return
    // all files in the dependency chain that need reprocessing
    char **changed_files = malloc(sizeof(char *));
    if (!changed_files) {
        pthread_rwlock_unlock(&graph->graph_lock);
        return NULL;
    }

    changed_files[0] = strdup(file);
    if (!changed_files[0]) {
        free(changed_files);
        pthread_rwlock_unlock(&graph->graph_lock);
        return NULL;
    }

    *count = 1;

    pthread_rwlock_unlock(&graph->graph_lock);
    return changed_files;
}
