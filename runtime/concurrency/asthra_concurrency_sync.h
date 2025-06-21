/**
 * Asthra Concurrency Synchronization - Threading and Synchronization Primitives
 * Part of the Asthra Runtime Modularization Plan - Phase 2
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * DESIGN GOALS:
 * - Mutex, condition variable, RWLock types
 * - Thread registration and GC integration
 * - Platform abstraction layer
 * - Enhanced monitoring and statistics
 */

#ifndef ASTHRA_CONCURRENCY_SYNC_H
#define ASTHRA_CONCURRENCY_SYNC_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include "asthra_concurrency_atomics.h"
#include "../asthra_runtime.h"

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// ENHANCED SYNCHRONIZATION PRIMITIVES
// =============================================================================

/**
 * Enhanced mutex with monitoring capabilities
 */
typedef struct AsthraConcurrencyMutex {
    asthra_concurrency_mutex_t mutex;           // Underlying mutex
#if !ASTHRA_HAS_C11_THREADS
    pthread_mutexattr_t attr;            // Mutex attributes (POSIX only)
#endif
    _Atomic(asthra_concurrency_thread_t) owner; // Atomic current owner
    asthra_concurrency_atomic_int recursion_count; // Atomic recursion count
    asthra_concurrency_atomic_counter_t creation_time; // Creation timestamp
    asthra_concurrency_atomic_counter_t lock_count;    // Total locks
    asthra_concurrency_atomic_counter_t contention_count; // Contention events
    const char *name;                    // Mutex name for debugging
} AsthraConcurrencyMutex;

/**
 * Enhanced condition variable with monitoring
 */
typedef struct AsthraConcurrencyCondVar {
    asthra_concurrency_cond_t cond;             // Underlying condition variable
#if !ASTHRA_HAS_C11_THREADS
    pthread_condattr_t attr;             // Condition variable attributes (POSIX only)
#endif
    asthra_concurrency_atomic_size_t waiter_count; // Atomic waiter count
    asthra_concurrency_atomic_counter_t signal_count; // Total signals
    asthra_concurrency_atomic_counter_t broadcast_count; // Total broadcasts
    const char *name;                    // Name for debugging
} AsthraConcurrencyCondVar;

/**
 * Barrier synchronization primitive for thread coordination
 */
typedef struct AsthraConcurrencyBarrier {
    asthra_concurrency_mutex_t mutex;           // Protects barrier state
    asthra_concurrency_cond_t cond;             // Condition for waiting threads
    asthra_concurrency_atomic_size_t party_count; // Total parties to wait for
    asthra_concurrency_atomic_size_t waiting_count; // Current waiting threads
    asthra_concurrency_atomic_counter_t generation; // Barrier generation number
    asthra_concurrency_atomic_bool is_broken;    // Barrier broken flag
    const char *name;                    // Name for debugging
} AsthraConcurrencyBarrier;

/**
 * Semaphore for counting resource access
 */
typedef struct AsthraConcurrencySemaphore {
    asthra_concurrency_mutex_t mutex;           // Protects semaphore state
    asthra_concurrency_cond_t cond;             // Condition for waiting threads
    asthra_concurrency_atomic_size_t permits;   // Available permits
    asthra_concurrency_atomic_size_t max_permits; // Maximum permits
    asthra_concurrency_atomic_counter_t acquire_count; // Total acquires
    asthra_concurrency_atomic_counter_t release_count; // Total releases
    const char *name;                    // Name for debugging
} AsthraConcurrencySemaphore;

/**
 * Enhanced reader-writer lock with monitoring
 */
typedef struct AsthraConcurrencyRWLock {
#if !ASTHRA_HAS_C11_THREADS
    pthread_rwlock_t rwlock;             // Underlying POSIX rwlock
    pthread_rwlockattr_t attr;           // RWLock attributes
#else
    asthra_concurrency_mutex_t rw_mutex; // Fallback mutex for C11 threads
    asthra_concurrency_cond_t read_cond; // Condition for readers
    asthra_concurrency_cond_t write_cond; // Condition for writers
#endif
    asthra_concurrency_atomic_int reader_count;    // Atomic reader count
    asthra_concurrency_atomic_bool writer_active;  // Atomic writer flag
    asthra_concurrency_atomic_size_t waiting_writers; // Atomic waiting writers
    asthra_concurrency_atomic_counter_t read_lock_count; // Total read locks
    asthra_concurrency_atomic_counter_t write_lock_count; // Total write locks
    const char *name;                    // Name for debugging
} AsthraConcurrencyRWLock;

// =============================================================================
// THREAD REGISTRATION AND GC INTEGRATION
// =============================================================================

/**
 * Thread-specific data for GC integration
 */
typedef struct AsthraConcurrencyThreadData {
    asthra_concurrency_thread_t thread_id;      // Thread identifier
    asthra_concurrency_atomic_bool is_registered; // Atomic registration status
    void **gc_roots;                     // GC root pointers
    asthra_concurrency_atomic_size_t gc_root_count;    // Atomic GC root count
    size_t gc_root_capacity;             // Capacity of GC roots array
    asthra_concurrency_atomic_counter_t registration_time; // Registration timestamp
    asthra_concurrency_atomic_counter_t last_gc_time;      // Last GC time
    struct AsthraConcurrencyThreadData *next; // Next in registry
} AsthraConcurrencyThreadData;

/**
 * Thread registry for managing all registered threads
 */
typedef struct {
    _Atomic(AsthraConcurrencyThreadData*) threads; // Atomic thread list
    asthra_concurrency_mutex_t registry_mutex;    // Protects thread registry
    asthra_concurrency_atomic_size_t thread_count; // Atomic thread count
    asthra_concurrency_atomic_bool shutdown;       // Atomic shutdown flag
    asthra_concurrency_atomic_counter_t total_registered; // Total ever registered
} AsthraConcurrencyThreadRegistry;

// =============================================================================
// MUTEX OPERATIONS
// =============================================================================

/**
 * Create a new mutex
 * @param name Optional name for debugging
 * @param recursive Whether to create a recursive mutex
 * @return New mutex handle or NULL on failure
 */
AsthraConcurrencyMutex* Asthra_mutex_create(const char* name, bool recursive);

/**
 * Lock a mutex with timeout
 * @param mutex Mutex to lock
 * @param timeout_ms Timeout in milliseconds
 * @return Result indicating success or failure
 */
AsthraResult Asthra_mutex_lock_timeout(AsthraConcurrencyMutex* mutex, uint64_t timeout_ms);

/**
 * Lock a mutex
 * @param mutex Mutex to lock
 * @return Result indicating success or failure
 */
AsthraResult Asthra_mutex_lock(AsthraConcurrencyMutex* mutex);

/**
 * Try to lock a mutex without blocking
 * @param mutex Mutex to try locking
 * @return Result indicating success or failure
 */
AsthraResult Asthra_mutex_trylock(AsthraConcurrencyMutex* mutex);

/**
 * Unlock a mutex
 * @param mutex Mutex to unlock
 * @return Result indicating success or failure
 */
AsthraResult Asthra_mutex_unlock(AsthraConcurrencyMutex* mutex);

/**
 * Destroy a mutex
 * @param mutex Mutex to destroy
 */
void Asthra_mutex_destroy(AsthraConcurrencyMutex* mutex);

// =============================================================================
// CONDITION VARIABLE OPERATIONS
// =============================================================================

/**
 * Create a new condition variable
 * @param name Optional name for debugging
 * @return New condition variable handle or NULL on failure
 */
AsthraConcurrencyCondVar* Asthra_condvar_create(const char* name);

/**
 * Wait on a condition variable
 * @param condvar Condition variable to wait on
 * @param mutex Associated mutex
 * @return Result indicating success or failure
 */
AsthraResult Asthra_condvar_wait(AsthraConcurrencyCondVar* condvar, AsthraConcurrencyMutex* mutex);

/**
 * Wait on a condition variable with timeout
 * @param condvar Condition variable to wait on
 * @param mutex Associated mutex
 * @param timeout_ms Timeout in milliseconds
 * @return Result indicating success, timeout, or failure
 */
AsthraResult Asthra_condvar_wait_timeout(AsthraConcurrencyCondVar* condvar, AsthraConcurrencyMutex* mutex, uint64_t timeout_ms);

/**
 * Signal a condition variable
 * @param condvar Condition variable to signal
 * @return Result indicating success or failure
 */
AsthraResult Asthra_condvar_signal(AsthraConcurrencyCondVar* condvar);

/**
 * Broadcast to all waiters on a condition variable
 * @param condvar Condition variable to broadcast to
 * @return Result indicating success or failure
 */
AsthraResult Asthra_condvar_broadcast(AsthraConcurrencyCondVar* condvar);

/**
 * Destroy a condition variable
 * @param condvar Condition variable to destroy
 */
void Asthra_condvar_destroy(AsthraConcurrencyCondVar* condvar);

// =============================================================================
// READER-WRITER LOCK OPERATIONS
// =============================================================================

/**
 * Create a new reader-writer lock
 * @param name Optional name for debugging
 * @return New RWLock handle or NULL on failure
 */
AsthraConcurrencyRWLock* Asthra_rwlock_create(const char* name);

/**
 * Acquire a read lock
 * @param rwlock RWLock to lock for reading
 * @return Result indicating success or failure
 */
AsthraResult Asthra_rwlock_read_lock(AsthraConcurrencyRWLock* rwlock);

/**
 * Acquire a write lock
 * @param rwlock RWLock to lock for writing
 * @return Result indicating success or failure
 */
AsthraResult Asthra_rwlock_write_lock(AsthraConcurrencyRWLock* rwlock);

/**
 * Try to acquire a read lock without blocking
 * @param rwlock RWLock to try locking for reading
 * @return Result indicating success or failure
 */
AsthraResult Asthra_rwlock_try_read_lock(AsthraConcurrencyRWLock* rwlock);

/**
 * Try to acquire a write lock without blocking
 * @param rwlock RWLock to try locking for writing
 * @return Result indicating success or failure
 */
AsthraResult Asthra_rwlock_try_write_lock(AsthraConcurrencyRWLock* rwlock);

/**
 * Release a read lock
 * @param rwlock RWLock to unlock from reading
 * @return Result indicating success or failure
 */
AsthraResult Asthra_rwlock_read_unlock(AsthraConcurrencyRWLock* rwlock);

/**
 * Release a write lock
 * @param rwlock RWLock to unlock from writing
 * @return Result indicating success or failure
 */
AsthraResult Asthra_rwlock_write_unlock(AsthraConcurrencyRWLock* rwlock);

/**
 * Destroy a reader-writer lock
 * @param rwlock RWLock to destroy
 */
void Asthra_rwlock_destroy(AsthraConcurrencyRWLock* rwlock);

// =============================================================================
// BARRIER OPERATIONS
// =============================================================================

/**
 * Create a new barrier
 * @param party_count Number of threads that must wait at the barrier
 * @return New barrier handle or NULL on failure
 */
AsthraConcurrencyBarrier* Asthra_barrier_create(size_t party_count);

/**
 * Wait at the barrier
 * @param barrier Barrier to wait at
 * @param is_leader Will be set to true for one thread (the leader)
 * @return Result indicating success or failure
 */
AsthraResult Asthra_barrier_wait(AsthraConcurrencyBarrier* barrier, bool* is_leader);

/**
 * Get the number of threads currently waiting at the barrier
 * @param barrier Barrier to query
 * @return Number of waiting threads
 */
size_t Asthra_barrier_waiting_count(const AsthraConcurrencyBarrier* barrier);

/**
 * Reset the barrier for reuse
 * @param barrier Barrier to reset
 * @return Result indicating success or failure
 */
AsthraResult Asthra_barrier_reset(AsthraConcurrencyBarrier* barrier);

/**
 * Destroy a barrier
 * @param barrier Barrier to destroy
 */
void Asthra_barrier_destroy(AsthraConcurrencyBarrier* barrier);

// =============================================================================
// SEMAPHORE OPERATIONS
// =============================================================================

/**
 * Create a new semaphore
 * @param permits Initial number of permits
 * @return New semaphore handle or NULL on failure
 */
AsthraConcurrencySemaphore* Asthra_semaphore_create(size_t permits);

/**
 * Acquire a permit from the semaphore (blocking)
 * @param semaphore Semaphore to acquire from
 * @return Result indicating success or failure
 */
AsthraResult Asthra_semaphore_acquire(AsthraConcurrencySemaphore* semaphore);

/**
 * Try to acquire a permit without blocking
 * @param semaphore Semaphore to try acquiring from
 * @return true if permit was acquired, false otherwise
 */
bool Asthra_semaphore_try_acquire(AsthraConcurrencySemaphore* semaphore);

/**
 * Acquire a permit with timeout
 * @param semaphore Semaphore to acquire from
 * @param timeout_ms Timeout in milliseconds
 * @param acquired Will be set to true if permit was acquired
 * @return Result indicating success or failure
 */
AsthraResult Asthra_semaphore_acquire_timeout(AsthraConcurrencySemaphore* semaphore, 
                                              int64_t timeout_ms, bool* acquired);

/**
 * Release a permit to the semaphore
 * @param semaphore Semaphore to release to
 * @return Result indicating success or failure
 */
AsthraResult Asthra_semaphore_release(AsthraConcurrencySemaphore* semaphore);

/**
 * Get the number of available permits
 * @param semaphore Semaphore to query
 * @return Number of available permits
 */
size_t Asthra_semaphore_available_permits(const AsthraConcurrencySemaphore* semaphore);

/**
 * Destroy a semaphore
 * @param semaphore Semaphore to destroy
 */
void Asthra_semaphore_destroy(AsthraConcurrencySemaphore* semaphore);

// =============================================================================
// THREAD REGISTRATION OPERATIONS
// =============================================================================

/**
 * Initialize the synchronization system
 * @return Result indicating success or failure
 */
AsthraResult Asthra_sync_system_init(void);

/**
 * Cleanup the synchronization system
 */
void Asthra_sync_system_cleanup(void);

/**
 * Register the current thread with the runtime system
 * @return Result indicating success or failure
 */
AsthraResult Asthra_register_c_thread(void);

/**
 * Unregister the current thread from the runtime system
 */
void Asthra_unregister_c_thread(void);

/**
 * Check if the current thread is registered
 * @return true if registered, false otherwise
 */
bool Asthra_thread_is_registered(void);

/**
 * Register a GC root for the current thread
 * @param ptr Pointer to register as GC root
 * @return Result indicating success or failure
 */
AsthraResult Asthra_thread_register_gc_root(void* ptr);

/**
 * Unregister a GC root for the current thread
 * @param ptr Pointer to unregister as GC root
 * @return Result indicating success or failure
 */
AsthraResult Asthra_thread_unregister_gc_root(void* ptr);

// =============================================================================
// SYNCHRONIZATION STATISTICS
// =============================================================================

/**
 * Synchronization statistics structure
 */
typedef struct {
    asthra_concurrency_atomic_counter_t threads_registered; // Registered threads
    asthra_concurrency_atomic_counter_t gc_roots_registered; // GC roots
    asthra_concurrency_atomic_counter_t mutex_contentions; // Mutex contentions
    asthra_concurrency_atomic_counter_t rwlock_contentions; // RWLock contentions
    asthra_concurrency_atomic_counter_t condvar_signals; // Condition variable signals
    asthra_concurrency_atomic_counter_t total_locks; // Total lock operations
    asthra_concurrency_atomic_counter_t lock_wait_time; // Total lock wait time
} AsthraConcurrencySyncStats;

/**
 * Get synchronization statistics
 * @return Synchronization statistics structure
 */
AsthraConcurrencySyncStats Asthra_get_sync_stats(void);

/**
 * Reset synchronization statistics
 */
void Asthra_reset_sync_stats(void);

/**
 * Dump synchronization state for debugging
 * @param output Output stream to write to
 */
void Asthra_dump_sync_state(FILE* output);

// =============================================================================
// ERROR CODES
// =============================================================================

#define ASTHRA_SYNC_ERROR_BASE 0x2000

typedef enum {
    ASTHRA_SYNC_ERROR_NONE = 0,
    ASTHRA_SYNC_ERROR_INIT_FAILED = ASTHRA_SYNC_ERROR_BASE + 1,
    ASTHRA_SYNC_ERROR_THREAD_NOT_REGISTERED = ASTHRA_SYNC_ERROR_BASE + 2,
    ASTHRA_SYNC_ERROR_MUTEX_TIMEOUT = ASTHRA_SYNC_ERROR_BASE + 3,
    ASTHRA_SYNC_ERROR_RWLOCK_TIMEOUT = ASTHRA_SYNC_ERROR_BASE + 4,
    ASTHRA_SYNC_ERROR_INVALID_HANDLE = ASTHRA_SYNC_ERROR_BASE + 5,
    ASTHRA_SYNC_ERROR_DEADLOCK_DETECTED = ASTHRA_SYNC_ERROR_BASE + 6,
    ASTHRA_SYNC_ERROR_SYSTEM_ERROR = ASTHRA_SYNC_ERROR_BASE + 7
} AsthraConcurrencySyncErrorCode;

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_CONCURRENCY_SYNC_H 
