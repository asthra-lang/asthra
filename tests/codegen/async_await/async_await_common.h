/**
 * Common header for async/await test utilities
 * Shared structures and helper functions for async/await tests
 */

#ifndef ASYNC_AWAIT_COMMON_H
#define ASYNC_AWAIT_COMMON_H

#include "../framework/test_framework_minimal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

// Async/await system structures
typedef enum {
    ASYNC_STATE_PENDING,
    ASYNC_STATE_READY,
    ASYNC_STATE_COMPLETED,
    ASYNC_STATE_CANCELLED,
    ASYNC_STATE_ERROR,
    ASYNC_STATE_COUNT
} AsyncState;

typedef struct {
    uint64_t future_id;
    AsyncState state;
    void* result_value;
    const char* error_message;
    bool is_awaited;
    int continuation_count;
    uint64_t continuations[16];
    uint64_t created_timestamp;
    uint64_t completed_timestamp;
} AsyncFuture;

typedef struct {
    const char* function_name;
    bool is_async;
    AsyncFuture* return_future;
    AsyncFuture* awaited_futures[16];
    int awaited_count;
    int yield_points[32];
    int yield_count;
    AsyncState current_state;
    uint64_t function_id;
    bool is_generator;
} AsyncFunction;

// Main context structure
typedef struct {
    AsyncFunction async_functions[32];
    int async_function_count;
    AsyncFuture futures[128];
    int future_count;
    
    // Runtime state
    uint64_t next_future_id;
    uint64_t next_function_id;
    uint64_t current_timestamp;
    
    // Statistics
    int completed_futures;
    int cancelled_futures;
    int error_futures;
    int total_await_operations;
} AsyncSystemContext;

// Helper functions (inline to avoid multiple definition errors)
static inline void init_async_system_context(AsyncSystemContext* ctx) {
    ctx->async_function_count = 0;
    ctx->future_count = 0;
    ctx->next_future_id = 6000;
    ctx->next_function_id = 7000;
    ctx->current_timestamp = 1000000;
    ctx->completed_futures = 0;
    ctx->cancelled_futures = 0;
    ctx->error_futures = 0;
    ctx->total_await_operations = 0;
    
    for (int i = 0; i < 32; i++) {
        ctx->async_functions[i] = (AsyncFunction){0};
    }
    
    for (int i = 0; i < 128; i++) {
        ctx->futures[i] = (AsyncFuture){0};
    }
}

static inline AsyncFunction* create_async_function(AsyncSystemContext* ctx, const char* function_name) {
    if (ctx->async_function_count >= 32) {
        return NULL;
    }
    
    AsyncFunction* func = &ctx->async_functions[ctx->async_function_count++];
    func->function_name = function_name;
    func->is_async = true;
    func->return_future = NULL;
    func->awaited_count = 0;
    func->yield_count = 0;
    func->current_state = ASYNC_STATE_PENDING;
    func->function_id = ctx->next_function_id++;
    func->is_generator = false;
    
    return func;
}

static inline AsyncFuture* create_future(AsyncSystemContext* ctx) {
    if (ctx->future_count >= 128) {
        return NULL;
    }
    
    AsyncFuture* future = &ctx->futures[ctx->future_count++];
    future->future_id = ctx->next_future_id++;
    future->state = ASYNC_STATE_PENDING;
    future->result_value = NULL;
    future->error_message = NULL;
    future->is_awaited = false;
    future->continuation_count = 0;
    future->created_timestamp = ctx->current_timestamp++;
    future->completed_timestamp = 0;
    
    return future;
}

static inline void await_future(AsyncSystemContext* ctx, AsyncFunction* func, AsyncFuture* future) {
    if (func && future && func->awaited_count < 16) {
        func->awaited_futures[func->awaited_count++] = future;
        future->is_awaited = true;
        ctx->total_await_operations++;
    }
}

static inline void complete_future(AsyncSystemContext* ctx, AsyncFuture* future, void* result) {
    if (future && future->state == ASYNC_STATE_PENDING) {
        future->state = ASYNC_STATE_COMPLETED;
        future->result_value = result;
        future->completed_timestamp = ctx->current_timestamp++;
        ctx->completed_futures++;
    }
}

static inline void cancel_future(AsyncSystemContext* ctx, AsyncFuture* future, const char* reason) {
    if (future && future->state == ASYNC_STATE_PENDING) {
        future->state = ASYNC_STATE_CANCELLED;
        future->error_message = reason;
        future->completed_timestamp = ctx->current_timestamp++;
        ctx->cancelled_futures++;
    }
}

static inline void error_future(AsyncSystemContext* ctx, AsyncFuture* future, const char* error_msg) {
    if (future && future->state == ASYNC_STATE_PENDING) {
        future->state = ASYNC_STATE_ERROR;
        future->error_message = error_msg;
        future->completed_timestamp = ctx->current_timestamp++;
        ctx->error_futures++;
    }
}

static inline void add_continuation(AsyncFuture* future, uint64_t continuation_id) {
    if (future && future->continuation_count < 16) {
        future->continuations[future->continuation_count++] = continuation_id;
    }
}

static inline void add_yield_point(AsyncFunction* func, int yield_point) {
    if (func && func->yield_count < 32) {
        func->yield_points[func->yield_count++] = yield_point;
    }
}

static inline bool is_future_complete(AsyncFuture* future) {
    return future && (future->state == ASYNC_STATE_COMPLETED || 
                     future->state == ASYNC_STATE_CANCELLED || 
                     future->state == ASYNC_STATE_ERROR);
}

static inline int count_pending_futures(AsyncSystemContext* ctx) {
    int count = 0;
    for (int i = 0; i < ctx->future_count; i++) {
        if (ctx->futures[i].state == ASYNC_STATE_PENDING) {
            count++;
        }
    }
    return count;
}

#endif // ASYNC_AWAIT_COMMON_H