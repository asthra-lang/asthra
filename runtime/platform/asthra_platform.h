/**
 * Asthra Platform Compatibility Layer
 * Provides cross-platform abstractions for system includes and types
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef ASTHRA_PLATFORM_H
#define ASTHRA_PLATFORM_H

// Platform detection
#if defined(__APPLE__)
    #define ASTHRA_PLATFORM_MACOS
    #define ASTHRA_PLATFORM_POSIX
#elif defined(__linux__)
    #define ASTHRA_PLATFORM_LINUX
    #define ASTHRA_PLATFORM_POSIX
#else
    #error "Unsupported platform. Asthra only supports macOS and Linux."
#endif

// Standard includes that work on all platforms
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdarg.h>

// POSIX systems includes and type definitions
#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>
#include <sched.h>

// Thread types
typedef pthread_t asthra_thread_t;
typedef pthread_mutex_t asthra_mutex_t;
typedef pthread_cond_t asthra_cond_t;
typedef pthread_once_t asthra_once_t;
typedef pthread_key_t asthra_tls_t;

// POSIX attribute types
typedef pthread_mutexattr_t asthra_mutexattr_t;
typedef pthread_condattr_t asthra_condattr_t;
typedef pthread_rwlockattr_t asthra_rwlockattr_t;
typedef pthread_rwlock_t asthra_rwlock_t;

// Platform-specific function abstractions
#define asthra_thread_yield() sched_yield()

#endif // ASTHRA_PLATFORM_H