#include "fast_check_command.h"
#include <dirent.h>
#include <fnmatch.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

// File Discovery and Utility Functions Implementation

char **fast_check_discover_files(const char *directory_path, const char **include_patterns,
                                 size_t include_count, const char **exclude_patterns,
                                 size_t exclude_count, size_t *file_count) {
    if (!directory_path || !file_count)
        return NULL;

    *file_count = 0;

    DIR *dir = opendir(directory_path);
    if (!dir)
        return NULL;

    // First pass: count files
    size_t capacity = 0;
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG || entry->d_type == DT_UNKNOWN) {
            // Check if it's an Asthra file
            const char *ext = strrchr(entry->d_name, '.');
            if (ext && strcmp(ext, ".asthra") == 0) {
                capacity++;
            }
        }
    }

    if (capacity == 0) {
        closedir(dir);
        return NULL;
    }

    // Allocate array
    char **files = malloc(capacity * sizeof(char *));
    if (!files) {
        closedir(dir);
        return NULL;
    }

    // Second pass: collect files
    rewinddir(dir);
    size_t count = 0;

    while ((entry = readdir(dir)) != NULL && count < capacity) {
        if (entry->d_type == DT_REG || entry->d_type == DT_UNKNOWN) {
            const char *ext = strrchr(entry->d_name, '.');
            if (ext && strcmp(ext, ".asthra") == 0) {
                // Build full path
                size_t path_len = strlen(directory_path) + strlen(entry->d_name) + 2;
                char *full_path = malloc(path_len);
                snprintf(full_path, path_len, "%s/%s", directory_path, entry->d_name);

                // Check patterns
                bool include = true;
                if (include_count > 0) {
                    include = false;
                    for (size_t i = 0; i < include_count; i++) {
                        if (fnmatch(include_patterns[i], entry->d_name, 0) == 0) {
                            include = true;
                            break;
                        }
                    }
                }

                if (include && exclude_count > 0) {
                    for (size_t i = 0; i < exclude_count; i++) {
                        if (fnmatch(exclude_patterns[i], entry->d_name, 0) == 0) {
                            include = false;
                            break;
                        }
                    }
                }

                if (include) {
                    files[count++] = full_path;
                } else {
                    free(full_path);
                }
            }
        }
    }

    closedir(dir);
    *file_count = count;

    return files;
}

char **fast_check_get_modified_files(const char *directory_path, int64_t since_timestamp,
                                     size_t *file_count) {
    if (!directory_path || !file_count)
        return NULL;

    *file_count = 0;

    // Simplified implementation - discover all files
    // Real implementation would check modification times
    return fast_check_discover_files(directory_path, NULL, 0, NULL, 0, file_count);
}

bool fast_check_should_check_file(const char *file_path, const FastCheckConfig *config) {
    if (!file_path || !config)
        return false;

    // Check if it's an Asthra file
    const char *ext = strrchr(file_path, '.');
    if (!ext || strcmp(ext, ".asthra") != 0) {
        return false;
    }

    const char *filename = strrchr(file_path, '/');
    filename = filename ? filename + 1 : file_path;

    // Check include patterns
    if (config->include_count > 0) {
        bool include = false;
        for (size_t i = 0; i < config->include_count; i++) {
            if (fnmatch(config->include_patterns[i], filename, 0) == 0) {
                include = true;
                break;
            }
        }
        if (!include)
            return false;
    }

    // Check exclude patterns
    for (size_t i = 0; i < config->exclude_count; i++) {
        if (fnmatch(config->exclude_patterns[i], filename, 0) == 0) {
            return false;
        }
    }

    return true;
}

double fast_check_get_file_modification_time(const char *file_path) {
    if (!file_path)
        return -1.0;

    struct stat file_stat;
    if (stat(file_path, &file_stat) == 0) {
        return (double)file_stat.st_mtime;
    }

    return -1.0;
}

char *fast_check_get_relative_path(const char *file_path, const char *base_path) {
    if (!file_path || !base_path)
        return NULL;

    // Simplified implementation - just strip base path prefix
    size_t base_len = strlen(base_path);
    if (strncmp(file_path, base_path, base_len) == 0) {
        const char *relative = file_path + base_len;
        if (*relative == '/')
            relative++;
        return strdup(relative);
    }

    return strdup(file_path);
}

void fast_check_free_file_list(char **files, size_t count) {
    if (!files)
        return;

    for (size_t i = 0; i < count; i++) {
        free(files[i]);
    }
    free(files);
}