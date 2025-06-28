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
#ifdef _WIN32
    #define ASTHRA_PLATFORM_WINDOWS
    #ifdef _WIN64
        #define ASTHRA_PLATFORM_WINDOWS64
    #else
        #define ASTHRA_PLATFORM_WINDOWS32
    #endif
#elif defined(__APPLE__)
    #define ASTHRA_PLATFORM_MACOS
    #define ASTHRA_PLATFORM_POSIX
#elif defined(__linux__)
    #define ASTHRA_PLATFORM_LINUX
    #define ASTHRA_PLATFORM_POSIX
#else
    #define ASTHRA_PLATFORM_POSIX
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

// Platform-specific includes and type definitions
#ifdef ASTHRA_PLATFORM_WINDOWS
    #include <windows.h>
    #include <process.h>
    #include <io.h>
    
    // POSIX compatibility types
    typedef SSIZE_T ssize_t;
    
    // Thread types
    typedef HANDLE asthra_thread_t;
    typedef CRITICAL_SECTION asthra_mutex_t;
    typedef CONDITION_VARIABLE asthra_cond_t;
    typedef INIT_ONCE asthra_once_t;
    typedef DWORD asthra_tls_t;
    
    // Windows doesn't have direct equivalents for these
    typedef int asthra_mutexattr_t;
    typedef int asthra_condattr_t;
    typedef int asthra_rwlockattr_t;
    typedef struct {
        SRWLOCK lock;
        int state;
    } asthra_rwlock_t;
    
    // Define missing constants
    #ifndef SSIZE_T_MAX
        #define SSIZE_T_MAX LONG_MAX
    #endif
    
    // pthread compatibility defines
    #define PTHREAD_MUTEX_RECURSIVE 1
    #define PTHREAD_ONCE_INIT INIT_ONCE_STATIC_INIT
    #define EAGAIN 11
    #define EBUSY 16
    #define ETIMEDOUT 110
    
#else
    // POSIX systems
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
#endif

// Platform-specific function abstractions
#ifdef ASTHRA_PLATFORM_WINDOWS
    #define asthra_thread_yield() SwitchToThread()
#else
    #define asthra_thread_yield() sched_yield()
#endif

#endif // ASTHRA_PLATFORM_H