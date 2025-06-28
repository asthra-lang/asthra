/**
 * Asthra Programming Language Compiler
 * Cross-Platform Abstraction Layer Implementation
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "platform.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if ASTHRA_PLATFORM_WINDOWS
#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")
#elif ASTHRA_PLATFORM_MACOS
#include <libgen.h>
#include <mach/mach.h>
#include <mach/mach_time.h>
#include <sys/stat.h>
#include <sys/sysctl.h>
#include <time.h>
#include <unistd.h>
#else
#include <libgen.h>
#include <sys/stat.h>
#include <time.h>
#endif

// =============================================================================
// PATH UTILITIES
// =============================================================================

int asthra_build_path(char *buffer, size_t buffer_size, const char **components) {
    if (!buffer || !components || buffer_size == 0) {
        return -1;
    }

    size_t total_length = 0;
    buffer[0] = '\0';

    // Calculate total length needed
    for (size_t i = 0; components[i] != NULL; i++) {
        if (i > 0) {
            total_length += 1; // separator
        }
        total_length += strlen(components[i]);
    }

    if (total_length >= buffer_size) {
        return -1; // Buffer too small
    }

    // Build the path
    for (size_t i = 0; components[i] != NULL; i++) {
        if (i > 0) {
            strncat(buffer, ASTHRA_PATH_SEPARATOR_STR, buffer_size - strlen(buffer) - 1);
        }
        strncat(buffer, components[i], buffer_size - strlen(buffer) - 1);
    }

    return (int)strlen(buffer);
}

void asthra_normalize_path(char *path) {
    if (!path) {
        return;
    }

    char *p = path;
    while (*p) {
#if ASTHRA_PLATFORM_WINDOWS
        if (*p == '/') {
            *p = '\\';
        }
#else
        if (*p == '\\') {
            *p = '/';
        }
#endif
        p++;
    }
}

bool asthra_file_exists(const char *path) {
    if (!path) {
        return false;
    }

#if ASTHRA_PLATFORM_WINDOWS
    DWORD attributes = GetFileAttributesA(path);
    return (attributes != INVALID_FILE_ATTRIBUTES && !(attributes & FILE_ATTRIBUTE_DIRECTORY));
#else
    struct stat st;
    return (stat(path, &st) == 0 && S_ISREG(st.st_mode));
#endif
}

bool asthra_create_directory(const char *path) {
    if (!path) {
        return false;
    }

    // Check if directory already exists
#if ASTHRA_PLATFORM_WINDOWS
    DWORD attributes = GetFileAttributesA(path);
    if (attributes != INVALID_FILE_ATTRIBUTES && (attributes & FILE_ATTRIBUTE_DIRECTORY)) {
        return true; // Already exists
    }
#else
    struct stat st;
    if (stat(path, &st) == 0 && S_ISDIR(st.st_mode)) {
        return true; // Already exists
    }
#endif

    // Create parent directories first
    char *path_copy = malloc(strlen(path) + 1);
    if (!path_copy) {
        return false;
    }
    strcpy(path_copy, path);

#if ASTHRA_PLATFORM_WINDOWS
    char *parent = path_copy;
    char *last_separator = strrchr(parent, '\\');
    if (!last_separator) {
        last_separator = strrchr(parent, '/');
    }

    if (last_separator && last_separator != parent) {
        *last_separator = '\0';
        if (!asthra_create_directory(parent)) {
            free(path_copy);
            return false;
        }
    }

    bool result = CreateDirectoryA(path, NULL) != 0 || GetLastError() == ERROR_ALREADY_EXISTS;
#else
    char *parent = dirname(path_copy);
    if (strcmp(parent, ".") != 0 && strcmp(parent, "/") != 0) {
        if (!asthra_create_directory(parent)) {
            free(path_copy);
            return false;
        }
    }

    bool result = mkdir(path, 0755) == 0 || errno == EEXIST;
#endif

    free(path_copy);
    return result;
}

// =============================================================================
// THREAD UTILITIES
// =============================================================================

#if ASTHRA_PLATFORM_WINDOWS

asthra_thread_id_t asthra_get_current_thread_id(void) {
    return GetCurrentThreadId();
}

asthra_process_id_t asthra_get_current_process_id(void) {
    return GetCurrentProcessId();
}

#else

asthra_thread_id_t asthra_get_current_thread_id(void) {
    return pthread_self();
}

asthra_process_id_t asthra_get_current_process_id(void) {
    return getpid();
}

#endif

// =============================================================================
// MEMORY UTILITIES
// =============================================================================

void *asthra_realloc_safe(void *ptr, size_t old_size, size_t new_size) {
    void *new_ptr = realloc(ptr, new_size);
    if (new_ptr && new_size > old_size) {
        // Zero the new memory
        memset((char *)new_ptr + old_size, 0, new_size - old_size);
    }
    return new_ptr;
}

// =============================================================================
// STRING UTILITIES
// =============================================================================

char *asthra_strdup(const char *str) {
    if (!str) {
        return NULL;
    }

    size_t len = strlen(str) + 1;
    char *copy = malloc(len);
    if (copy) {
        memcpy(copy, str, len);
    }
    return copy;
}

bool asthra_cstring_ends_with(const char *str, const char *suffix) {
    if (!str || !suffix) {
        return false;
    }

    size_t str_len = strlen(str);
    size_t suffix_len = strlen(suffix);

    if (suffix_len > str_len) {
        return false;
    }

    return strcmp(str + str_len - suffix_len, suffix) == 0;
}

bool asthra_string_starts_with(const char *str, const char *prefix) {
    if (!str || !prefix) {
        return false;
    }

    return strncmp(str, prefix, strlen(prefix)) == 0;
}

// =============================================================================
// PLATFORM INFO UTILITIES
// =============================================================================

const char *asthra_get_platform_info(void) {
    static char info[256];
    snprintf(info, sizeof(info), "%s (%s %d)", ASTHRA_PLATFORM_NAME, ASTHRA_COMPILER_NAME,
             ASTHRA_COMPILER_VERSION);
    return info;
}

// =============================================================================
// ERROR HANDLING UTILITIES
// =============================================================================

const char *asthra_get_error_string(asthra_error_t error) {
#if ASTHRA_PLATFORM_WINDOWS
    static char buffer[256];
    DWORD result =
        FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, error,
                       MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), buffer, sizeof(buffer), NULL);

    if (result == 0) {
        snprintf(buffer, sizeof(buffer), "Unknown error %lu", error);
    } else {
        // Remove trailing newline
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';
            if (len > 1 && buffer[len - 2] == '\r') {
                buffer[len - 2] = '\0';
            }
        }
    }

    return buffer;
#else
    return strerror(error);
#endif
}

// =============================================================================
// UNICODE UTILITIES (Windows)
// =============================================================================

#if ASTHRA_PLATFORM_WINDOWS

char *asthra_wchar_to_utf8(const wchar_t *wstr) {
    if (!wstr) {
        return NULL;
    }

    int size = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
    if (size <= 0) {
        return NULL;
    }

    char *utf8_str = malloc(size);
    if (!utf8_str) {
        return NULL;
    }

    if (WideCharToMultiByte(CP_UTF8, 0, wstr, -1, utf8_str, size, NULL, NULL) <= 0) {
        free(utf8_str);
        return NULL;
    }

    return utf8_str;
}

wchar_t *asthra_utf8_to_wchar(const char *utf8_str) {
    if (!utf8_str) {
        return NULL;
    }

    int size = MultiByteToWideChar(CP_UTF8, 0, utf8_str, -1, NULL, 0);
    if (size <= 0) {
        return NULL;
    }

    wchar_t *wstr = malloc(size * sizeof(wchar_t));
    if (!wstr) {
        return NULL;
    }

    if (MultiByteToWideChar(CP_UTF8, 0, utf8_str, -1, wstr, size) <= 0) {
        free(wstr);
        return NULL;
    }

    return wstr;
}

#endif

// =============================================================================
// PERFORMANCE UTILITIES
// =============================================================================

uint64_t asthra_get_current_time_ms(void) {
#if ASTHRA_PLATFORM_WINDOWS
    LARGE_INTEGER counter, frequency;
    QueryPerformanceCounter(&counter);
    QueryPerformanceFrequency(&frequency);
    return (uint64_t)((counter.QuadPart * 1000) / frequency.QuadPart);
#elif ASTHRA_PLATFORM_MACOS
    static mach_timebase_info_data_t timebase = {0};
    if (timebase.denom == 0) {
        mach_timebase_info(&timebase);
    }
    uint64_t nanos = mach_absolute_time() * timebase.numer / timebase.denom;
    return nanos / 1000000ULL;
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000ULL + (uint64_t)ts.tv_nsec / 1000000ULL;
#endif
}

uint64_t asthra_get_high_resolution_time(void) {
#if ASTHRA_PLATFORM_WINDOWS
    LARGE_INTEGER counter;
    QueryPerformanceCounter(&counter);
    return (uint64_t)counter.QuadPart;
#elif ASTHRA_PLATFORM_MACOS
    return mach_absolute_time();
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec;
#endif
}

uint64_t asthra_get_high_resolution_frequency(void) {
#if ASTHRA_PLATFORM_WINDOWS
    LARGE_INTEGER frequency;
    QueryPerformanceFrequency(&frequency);
    return (uint64_t)frequency.QuadPart;
#elif ASTHRA_PLATFORM_MACOS
    static mach_timebase_info_data_t timebase = {0};
    if (timebase.denom == 0) {
        mach_timebase_info(&timebase);
    }
    return 1000000000ULL * timebase.denom / timebase.numer;
#else
    return 1000000000ULL; // nanoseconds
#endif
}

double asthra_get_elapsed_seconds(uint64_t start_time, uint64_t end_time) {
    uint64_t frequency = asthra_get_high_resolution_frequency();
    return (double)(end_time - start_time) / (double)frequency;
}

// =============================================================================
// SYSTEM INFORMATION
// =============================================================================

size_t asthra_get_system_memory(void) {
#if ASTHRA_PLATFORM_WINDOWS
    MEMORYSTATUSEX status;
    status.dwLength = sizeof(status);
    if (GlobalMemoryStatusEx(&status)) {
        return (size_t)status.ullTotalPhys;
    }
    return 0;
#elif ASTHRA_PLATFORM_MACOS
    int64_t memory;
    size_t size = sizeof(memory);
    if (sysctlbyname("hw.memsize", &memory, &size, NULL, 0) == 0) {
        return (size_t)memory;
    }
    return 0;
#else
    long pages = sysconf(_SC_PHYS_PAGES);
    long page_size = sysconf(_SC_PAGE_SIZE);
    if (pages > 0 && page_size > 0) {
        return (size_t)pages * (size_t)page_size;
    }
    return 0;
#endif
}

int asthra_get_cpu_count(void) {
#if ASTHRA_PLATFORM_WINDOWS
    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);
    return (int)sysinfo.dwNumberOfProcessors;
#else
    return (int)sysconf(_SC_NPROCESSORS_ONLN);
#endif
}
