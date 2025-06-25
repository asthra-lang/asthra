/**
 * Asthra Programming Language Runtime v1.2 - Synchronization Module
 * Synchronization Primitives and Thread Safety
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * This module provides synchronization primitives including
 * mutexes, condition variables, and read-write locks.
 */

#ifndef ASTHRA_RUNTIME_SYNC_H
#define ASTHRA_RUNTIME_SYNC_H

#include "../core/asthra_runtime_core.h"

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// C17 ENHANCED SYNCHRONIZATION PRIMITIVES
// =============================================================================

typedef struct AsthraMutex AsthraMutex;
typedef struct AsthraCondVar AsthraCondVar;
typedef struct AsthraRWLock AsthraRWLock;

// =============================================================================
// MUTEX OPERATIONS
// =============================================================================

// Mutex operations
AsthraMutex *asthra_mutex_create(void);
void asthra_mutex_destroy(AsthraMutex *mutex);
void asthra_mutex_lock(AsthraMutex *mutex);
bool asthra_mutex_trylock(AsthraMutex *mutex);
void asthra_mutex_unlock(AsthraMutex *mutex);

// =============================================================================
// CONDITION VARIABLE OPERATIONS
// =============================================================================

// Condition variable operations
AsthraCondVar *asthra_condvar_create(void);
void asthra_condvar_destroy(AsthraCondVar *condvar);
void asthra_condvar_wait(AsthraCondVar *condvar, AsthraMutex *mutex);
bool asthra_condvar_timedwait(AsthraCondVar *condvar, AsthraMutex *mutex, uint64_t timeout_ms);
void asthra_condvar_signal(AsthraCondVar *condvar);
void asthra_condvar_broadcast(AsthraCondVar *condvar);

// =============================================================================
// READ-WRITE LOCK OPERATIONS
// =============================================================================

// Read-write lock operations
AsthraRWLock *asthra_rwlock_create(void);
void asthra_rwlock_destroy(AsthraRWLock *rwlock);
void asthra_rwlock_read_lock(AsthraRWLock *rwlock);
void asthra_rwlock_write_lock(AsthraRWLock *rwlock);
bool asthra_rwlock_try_read_lock(AsthraRWLock *rwlock);
bool asthra_rwlock_try_write_lock(AsthraRWLock *rwlock);
void asthra_rwlock_read_unlock(AsthraRWLock *rwlock);
void asthra_rwlock_write_unlock(AsthraRWLock *rwlock);

// =============================================================================
// VARIADIC FUNCTION SUPPORT
// =============================================================================

typedef enum {
    ASTHRA_VARARG_INT,
    ASTHRA_VARARG_UINT,
    ASTHRA_VARARG_FLOAT,
    ASTHRA_VARARG_DOUBLE,
    ASTHRA_VARARG_PTR,
    ASTHRA_VARARG_STRING
} AsthraVarArgType;

typedef struct {
    AsthraVarArgType type;
    union {
        int int_val;
        unsigned int uint_val;
        float float_val;
        double double_val;
        void *ptr_val;
        const char *str_val;
    } value;
} AsthraVarArg;

// Variadic function calls
int asthra_call_variadic_int(void *func_ptr, AsthraVarArg *args, size_t arg_count);
void *asthra_call_variadic_ptr(void *func_ptr, AsthraVarArg *args, size_t arg_count);
double asthra_call_variadic_double(void *func_ptr, AsthraVarArg *args, size_t arg_count);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_RUNTIME_SYNC_H
