/**
 * Asthra Programming Language Task System Threads v1.2
 * Thread Registration and GC Integration Implementation
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include <stdio.h>
#include <stdlib.h>

#include "platform/asthra_platform.h"
#include "asthra_tasks_threads.h"

// =============================================================================
// GLOBAL VARIABLES
// =============================================================================

asthra_tls_t g_thread_registered_key;
bool g_thread_key_initialized = false;

// =============================================================================
// THREAD REGISTRATION FOR GC
// =============================================================================

void init_thread_key(void) {
    if (g_thread_key_initialized)
        return;

    #ifdef ASTHRA_PLATFORM_WINDOWS
    g_thread_registered_key = TlsAlloc();
    g_thread_key_initialized = (g_thread_registered_key != TLS_OUT_OF_INDEXES);
#else
    pthread_key_create(&g_thread_registered_key, NULL);
    g_thread_key_initialized = true;
#endif
    g_thread_key_initialized = true;
}

int asthra_thread_register(void) {
    init_thread_key();

    #ifdef ASTHRA_PLATFORM_WINDOWS
    TlsSetValue(g_thread_registered_key, (void *)1);
#else
    pthread_setspecific(g_thread_registered_key, (void *)1);
#endif
    return 0;
}

void asthra_thread_unregister(void) {
    init_thread_key();

    pthread_setspecific(g_thread_registered_key, NULL);
}

bool asthra_thread_is_registered(void) {
    init_thread_key();

    #ifdef ASTHRA_PLATFORM_WINDOWS
    return TlsGetValue(g_thread_registered_key) != NULL;
#else
    return pthread_getspecific(g_thread_registered_key) != NULL;
#endif
}
