/**
 * Asthra Concurrency Thread Management - Modular Architecture
 * Complete thread management functionality via modular includes
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * MODULAR ARCHITECTURE:
 * This header provides complete backward compatibility with the original
 * monolithic asthra_concurrency_threads.c through focused modular components.
 *
 * MODULES:
 * - Thread Registry: Thread registration and management for GC integration
 * - GC Root Management: Thread-local GC root registration and management
 * - Mutex Operations: Mutex creation, locking, and management primitives
 * - Condition Variables: Condition variable creation and signaling primitives
 * - Read-Write Locks: Read-write lock creation and locking primitives
 */

#ifndef ASTHRA_CONCURRENCY_THREADS_MODULAR_H
#define ASTHRA_CONCURRENCY_THREADS_MODULAR_H

// Include all modular components for complete functionality
#include "concurrency/asthra_concurrency_condvar.h"
#include "concurrency/asthra_concurrency_gc_roots.h"
#include "concurrency/asthra_concurrency_mutex.h"
#include "concurrency/asthra_concurrency_rwlock.h"
#include "concurrency/asthra_concurrency_thread_registry.h"
#include "concurrency/asthra_concurrency_threads_common.h"

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// MODULAR ARCHITECTURE COMPLETE API
// =============================================================================

// All functionality is now available through the included modules:
//
// Thread Registry (asthra_concurrency_thread_registry.h):
// - AsthraResult Asthra_register_c_thread(void);
// - void Asthra_unregister_c_thread(void);
// - bool Asthra_thread_is_registered(void);
// - AsthraConcurrencyThreadRegistryStats Asthra_get_thread_registry_stats(void);
//
// GC Root Management (asthra_concurrency_gc_roots.h):
// - AsthraResult Asthra_register_thread_gc_root(void *ptr);
// - AsthraResult Asthra_unregister_thread_gc_root(void *ptr);
//
// Mutex Operations (asthra_concurrency_mutex.h):
// - AsthraConcurrencyMutex* Asthra_mutex_create(const char* name, bool recursive);
// - AsthraResult Asthra_mutex_lock(AsthraConcurrencyMutex* mutex);
// - AsthraResult Asthra_mutex_trylock(AsthraConcurrencyMutex* mutex);
// - AsthraResult Asthra_mutex_unlock(AsthraConcurrencyMutex* mutex);
// - void Asthra_mutex_destroy(AsthraConcurrencyMutex* mutex);
//
// Condition Variables (asthra_concurrency_condvar.h):
// - AsthraConcurrencyCondVar* Asthra_condvar_create(const char* name);
// - AsthraResult Asthra_condvar_wait(AsthraConcurrencyCondVar* condvar, AsthraConcurrencyMutex*
// mutex);
// - AsthraResult Asthra_condvar_wait_timeout(AsthraConcurrencyCondVar* condvar,
// AsthraConcurrencyMutex* mutex, uint64_t timeout_ms);
// - AsthraResult Asthra_condvar_signal(AsthraConcurrencyCondVar* condvar);
// - AsthraResult Asthra_condvar_broadcast(AsthraConcurrencyCondVar* condvar);
// - void Asthra_condvar_destroy(AsthraConcurrencyCondVar* condvar);
//
// Read-Write Locks (asthra_concurrency_rwlock.h):
// - AsthraConcurrencyRWLock* Asthra_rwlock_create(const char* name);
// - AsthraResult Asthra_rwlock_read_lock(AsthraConcurrencyRWLock* rwlock);
// - AsthraResult Asthra_rwlock_write_lock(AsthraConcurrencyRWLock* rwlock);
// - AsthraResult Asthra_rwlock_try_read_lock(AsthraConcurrencyRWLock* rwlock);
// - AsthraResult Asthra_rwlock_try_write_lock(AsthraConcurrencyRWLock* rwlock);
// - AsthraResult Asthra_rwlock_unlock(AsthraConcurrencyRWLock* rwlock);
// - void Asthra_rwlock_destroy(AsthraConcurrencyRWLock* rwlock);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_CONCURRENCY_THREADS_MODULAR_H
