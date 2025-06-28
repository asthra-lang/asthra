/**
 * Asthra Platform Compatibility Layer - Windows Stub Implementation
 * Provides minimal Windows implementations for POSIX functions
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifdef _WIN32

#include "asthra_platform.h"
#include <windows.h>
#include <process.h>
#include <errno.h>

// Stub implementations for Windows
int pthread_mutex_init(asthra_mutex_t *mutex, const asthra_mutexattr_t *attr) {
    (void)attr;
    InitializeCriticalSection(mutex);
    return 0;
}

int pthread_mutex_destroy(asthra_mutex_t *mutex) {
    DeleteCriticalSection(mutex);
    return 0;
}

int pthread_mutex_lock(asthra_mutex_t *mutex) {
    EnterCriticalSection(mutex);
    return 0;
}

int pthread_mutex_unlock(asthra_mutex_t *mutex) {
    LeaveCriticalSection(mutex);
    return 0;
}

int pthread_mutex_trylock(asthra_mutex_t *mutex) {
    return TryEnterCriticalSection(mutex) ? 0 : EBUSY;
}

int pthread_mutexattr_init(asthra_mutexattr_t *attr) {
    *attr = 0;
    return 0;
}

int pthread_mutexattr_destroy(asthra_mutexattr_t *attr) {
    (void)attr;
    return 0;
}

int pthread_mutexattr_settype(asthra_mutexattr_t *attr, int type) {
    *attr = type;
    return 0;
}

int pthread_cond_init(asthra_cond_t *cond, const asthra_condattr_t *attr) {
    (void)attr;
    InitializeConditionVariable(cond);
    return 0;
}

int pthread_cond_destroy(asthra_cond_t *cond) {
    (void)cond;
    return 0;
}

int pthread_cond_wait(asthra_cond_t *cond, asthra_mutex_t *mutex) {
    return SleepConditionVariableCS(cond, mutex, INFINITE) ? 0 : EINVAL;
}

int pthread_cond_signal(asthra_cond_t *cond) {
    WakeConditionVariable(cond);
    return 0;
}

int pthread_cond_broadcast(asthra_cond_t *cond) {
    WakeAllConditionVariable(cond);
    return 0;
}

int pthread_key_create(asthra_tls_t *key, void (*destructor)(void*)) {
    (void)destructor;
    *key = TlsAlloc();
    return (*key == TLS_OUT_OF_INDEXES) ? EAGAIN : 0;
}

void *pthread_getspecific(asthra_tls_t key) {
    return TlsGetValue(key);
}

int pthread_setspecific(asthra_tls_t key, const void *value) {
    return TlsSetValue(key, (LPVOID)value) ? 0 : EINVAL;
}

int pthread_once(asthra_once_t *once_control, void (*init_routine)(void)) {
    BOOL pending = FALSE;
    if (InitOnceBeginInitialize(once_control, 0, &pending, NULL)) {
        if (pending) {
            init_routine();
        }
        InitOnceComplete(once_control, 0, NULL);
    }
    return 0;
}

// clock_gettime implementation for Windows
#include <time.h>

#ifndef CLOCK_MONOTONIC
#define CLOCK_MONOTONIC 1
#endif

#ifndef CLOCK_REALTIME
#define CLOCK_REALTIME 0
#endif

int clock_gettime(int clk_id, struct timespec *ts) {
    if (!ts) return -1;
    
    if (clk_id == CLOCK_MONOTONIC) {
        // Use QueryPerformanceCounter for monotonic time
        LARGE_INTEGER freq, count;
        if (!QueryPerformanceFrequency(&freq) || !QueryPerformanceCounter(&count)) {
            return -1;
        }
        
        ts->tv_sec = count.QuadPart / freq.QuadPart;
        ts->tv_nsec = (long)((count.QuadPart % freq.QuadPart) * 1000000000LL / freq.QuadPart);
    } else {
        // Use GetSystemTimeAsFileTime for real time
        FILETIME ft;
        ULARGE_INTEGER uli;
        GetSystemTimeAsFileTime(&ft);
        
        uli.LowPart = ft.dwLowDateTime;
        uli.HighPart = ft.dwHighDateTime;
        
        // Convert from Windows epoch (1601) to Unix epoch (1970)
        uli.QuadPart -= 116444736000000000LL;
        
        ts->tv_sec = (time_t)(uli.QuadPart / 10000000LL);
        ts->tv_nsec = (long)((uli.QuadPart % 10000000LL) * 100);
    }
    
    return 0;
}

#endif // _WIN32