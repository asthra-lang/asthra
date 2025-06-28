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
    #ifndef EAGAIN
        #define EAGAIN 11
    #endif
    #ifndef EBUSY
        #define EBUSY 16
    #endif
    #ifndef ETIMEDOUT
        #define ETIMEDOUT 138  // Use Windows SDK value
    #endif
    
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

// Time function compatibility
#ifdef ASTHRA_PLATFORM_WINDOWS
    // Windows doesn't have clock_gettime, provide compatibility
    #define CLOCK_REALTIME 0
    #define CLOCK_MONOTONIC 1
    
    static inline int clock_gettime(int clk_id, struct timespec *tp) {
        LARGE_INTEGER frequency, counter;
        static LARGE_INTEGER start_counter = {0};
        static int initialized = 0;
        
        if (!initialized) {
            QueryPerformanceCounter(&start_counter);
            initialized = 1;
        }
        
        QueryPerformanceFrequency(&frequency);
        QueryPerformanceCounter(&counter);
        
        if (clk_id == CLOCK_MONOTONIC) {
            // Monotonic clock - time since some unspecified starting point
            LONGLONG elapsed = counter.QuadPart - start_counter.QuadPart;
            tp->tv_sec = (time_t)(elapsed / frequency.QuadPart);
            tp->tv_nsec = (long)((elapsed % frequency.QuadPart) * 1000000000 / frequency.QuadPart);
        } else {
            // Real time clock
            FILETIME ft;
            ULARGE_INTEGER uli;
            GetSystemTimeAsFileTime(&ft);
            uli.LowPart = ft.dwLowDateTime;
            uli.HighPart = ft.dwHighDateTime;
            
            // Convert to Unix epoch (1/1/1970)
            uli.QuadPart -= 116444736000000000ULL; // Difference between 1601 and 1970
            tp->tv_sec = (time_t)(uli.QuadPart / 10000000);
            tp->tv_nsec = (long)((uli.QuadPart % 10000000) * 100);
        }
        
        return 0;
    }
#endif

// String function compatibility
#ifdef ASTHRA_PLATFORM_WINDOWS
    // Use secure versions on Windows when available
    #define asthra_strncpy(dst, src, size) strncpy_s(dst, size, src, _TRUNCATE)
#else
    #define asthra_strncpy(dst, src, size) strncpy(dst, src, size)
#endif

#endif // ASTHRA_PLATFORM_H