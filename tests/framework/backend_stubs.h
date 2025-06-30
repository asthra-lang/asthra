/**
 * Backend API Stubs for Tests
 * 
 * Provides stub implementations for the removed backend abstraction layer
 * to maintain test compatibility during the transition to direct LLVM usage.
 */

#ifndef BACKEND_STUBS_H
#define BACKEND_STUBS_H

#ifdef __cplusplus
extern "C" {
#endif

// Compatibility defines for removed backend types
#define ASTHRA_BACKEND_LLVM_IR 0
#define ASTHRA_BACKEND_C 1

// Stub functions for removed backend API
static inline void *asthra_backend_create(void *options) {
    (void)options;
    return (void *)1; // Non-NULL placeholder
}

static inline void asthra_backend_destroy(void *backend) {
    (void)backend;
}

static inline int asthra_backend_initialize(void *backend, void *options) {
    (void)backend;
    (void)options;
    return 0; // Success
}

static inline int asthra_backend_generate(void *backend, void *ctx, void *ast, const char *output) {
    (void)backend;
    (void)ctx;
    (void)ast;
    (void)output;
    return 0; // Success
}

#ifdef __cplusplus
}
#endif

#endif // BACKEND_STUBS_H