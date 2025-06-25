/**
 * Asthra Concurrency Patterns - Worker Pools and Advanced Concurrency Patterns
 * Part of the Asthra Runtime Modularization Plan - Phase 2
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * DESIGN GOALS:
 * - Worker pool management and task distribution
 * - Fan-out/fan-in patterns for parallel processing
 * - Pipeline patterns for stream processing
 * - Advanced concurrency utilities and patterns
 */

#ifndef ASTHRA_CONCURRENCY_PATTERNS_H
#define ASTHRA_CONCURRENCY_PATTERNS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "../asthra_runtime.h"
#include "asthra_concurrency_atomics.h"
#include "asthra_concurrency_channels.h"
#include "asthra_concurrency_tasks.h"

#ifdef __cplusplus
extern "C" {

// Asthra_worker_pool_dump_stats
;

// Asthra_multiplexer_create
AsthraConcurrencyMultiplexer* Asthra_multiplexer_create(AsthraConcurrencyChannel** channels, size_t channel_count,;


// Asthra_multiplexer_start
;


// Asthra_multiplexer_stop
;


// Asthra_multiplexer_destroy
;


// Asthra_load_balancer_create
AsthraConcurrencyLoadBalancer* Asthra_load_balancer_create(AsthraConcurrencyWorkerPool** pools, size_t pool_count,;


// Asthra_load_balancer_submit
AsthraConcurrencyTaskHandleWithAwait* Asthra_load_balancer_submit(AsthraConcurrencyLoadBalancer* balancer, AsthraConcurrencyTaskFunction func, void* args,;


// Asthra_load_balancer_destroy
;


// Asthra_dump_pattern_diagnostics
;

#endif

// =============================================================================
// WORKER POOL STRUCTURES
// =============================================================================

/**
 * Worker pool for managing a pool of worker threads
 */
typedef struct AsthraConcurrencyWorkerPool {
    asthra_concurrency_thread_t *workers;                // Array of worker threads
    size_t num_workers;                                  // Number of worker threads
    AsthraConcurrencyChannel *task_queue;                // Task queue channel
    asthra_concurrency_atomic_bool shutdown;             // Shutdown flag
    asthra_concurrency_atomic_counter_t tasks_submitted; // Total tasks submitted
    asthra_concurrency_atomic_counter_t tasks_completed; // Total tasks completed
    asthra_concurrency_atomic_counter_t tasks_failed;    // Total failed tasks
    asthra_concurrency_mutex_t pool_mutex;               // Protects pool state
    char name[64];                                       // Pool name for debugging
} AsthraConcurrencyWorkerPool;

/**
 * Work item for worker pool task submission
 */
typedef struct {
    AsthraConcurrencyTaskFunction func;           // Task function to execute
    void *args;                                   // Task arguments
    size_t args_size;                             // Size of arguments
    AsthraConcurrencyTaskHandleWithAwait *handle; // Task handle for results
} AsthraConcurrencyWorkItem;

// =============================================================================
// PIPELINE STRUCTURES
// =============================================================================

/**
 * Pipeline stage definition
 */
typedef struct {
    AsthraConcurrencyTaskFunction stage_func; // Stage processing function
    size_t buffer_size;                       // Buffer size for this stage
    int priority;                             // Stage priority
    const char *name;                         // Stage name for debugging
} AsthraConcurrencyPipelineStage;

/**
 * Pipeline configuration
 */
typedef struct {
    AsthraConcurrencyPipelineStage *stages;              // Array of pipeline stages
    size_t num_stages;                                   // Number of stages
    AsthraConcurrencyChannel *input_channel;             // Pipeline input
    AsthraConcurrencyChannel *output_channel;            // Pipeline output
    AsthraConcurrencyChannel **intermediate_channels;    // Intermediate channels
    asthra_concurrency_atomic_bool running;              // Pipeline running flag
    asthra_concurrency_atomic_counter_t items_processed; // Total items processed
    char name[64];                                       // Pipeline name
} AsthraConcurrencyPipeline;

// =============================================================================
// FAN-OUT/FAN-IN STRUCTURES
// =============================================================================

/**
 * Fan-out configuration for distributing work to multiple workers
 */
typedef struct {
    AsthraConcurrencyChannel *input_channel;               // Input channel
    AsthraConcurrencyChannel **output_channels;            // Array of output channels
    size_t num_outputs;                                    // Number of output channels
    AsthraConcurrencyTaskFunction worker_func;             // Worker function
    asthra_concurrency_atomic_bool running;                // Fan-out running flag
    asthra_concurrency_atomic_counter_t items_distributed; // Items distributed
} AsthraConcurrencyFanOut;

/**
 * Fan-in configuration for collecting results from multiple workers
 */
typedef struct {
    AsthraConcurrencyChannel **input_channels;           // Array of input channels
    size_t num_inputs;                                   // Number of input channels
    AsthraConcurrencyChannel *output_channel;            // Output channel
    AsthraConcurrencyTaskFunction collector_func;        // Collector function
    asthra_concurrency_atomic_bool running;              // Fan-in running flag
    asthra_concurrency_atomic_counter_t items_collected; // Items collected
} AsthraConcurrencyFanIn;

// =============================================================================
// WORKER POOL OPERATIONS
// =============================================================================

/**
 * Create a new worker pool
 * @param num_workers Number of worker threads to create
 * @param queue_size Task queue size
 * @param name Optional name for debugging
 * @return Worker pool handle or NULL on failure
 */
AsthraConcurrencyWorkerPool* Asthra_worker_pool_create(size_t num_workers, 
                                                      size_t queue_size,
                                                      const char* name);

/**
 * Submit a task to the worker pool
 * @param pool Worker pool
 * @param func Task function
 * @param args Task arguments
 * @param args_size Size of arguments
 * @return Task handle or NULL on failure
 */
AsthraConcurrencyTaskHandleWithAwait* Asthra_worker_pool_submit(
    AsthraConcurrencyWorkerPool* pool,
    AsthraConcurrencyTaskFunction func,
    void* args,
    size_t args_size);

/**
 * Submit a task with priority to the worker pool
 * @param pool Worker pool
 * @param func Task function
 * @param args Task arguments
 * @param args_size Size of arguments
 * @param priority Task priority
 * @return Task handle or NULL on failure
 */
AsthraConcurrencyTaskHandleWithAwait* Asthra_worker_pool_submit_priority(
    AsthraConcurrencyWorkerPool* pool,
    AsthraConcurrencyTaskFunction func,
    void* args,
    size_t args_size,
    int priority);

/**
 * Get worker pool statistics
 * @param pool Worker pool
 * @param active_workers Number of active workers
 * @param queue_size Current queue size
 * @param completed_tasks Total completed tasks
 * @return Result indicating success or failure
 */
AsthraResult Asthra_worker_pool_get_stats(AsthraConcurrencyWorkerPool* pool,
                                         size_t* active_workers,
                                         size_t* queue_size,
                                         uint64_t* completed_tasks);

/**
 * Shutdown worker pool
 * @param pool Worker pool to shutdown
 * @param wait_for_completion Wait for all tasks to complete
 */
void Asthra_worker_pool_shutdown(AsthraConcurrencyWorkerPool* pool, 
                                bool wait_for_completion);

/**
 * Destroy worker pool
 * @param pool Worker pool to destroy
 */
void Asthra_worker_pool_destroy(AsthraConcurrencyWorkerPool* pool);

// =============================================================================
// PIPELINE OPERATIONS
// =============================================================================

/**
 * Create a processing pipeline
 * @param stages Array of pipeline stage definitions
 * @param num_stages Number of stages
 * @param input_channel Initial input channel
 * @param output_channel Final output channel
 * @param name Optional name for debugging
 * @return Pipeline handle or NULL on failure
 */
AsthraConcurrencyPipeline* Asthra_pipeline_create(
    const AsthraConcurrencyPipelineStage* stages,
    size_t num_stages,
    AsthraConcurrencyChannel* input_channel,
    AsthraConcurrencyChannel* output_channel,
    const char* name);

/**
 * Start a pipeline
 * @param pipeline Pipeline to start
 * @return Result indicating success or failure
 */
AsthraResult Asthra_pipeline_start(AsthraConcurrencyPipeline* pipeline);

/**
 * Stop a pipeline
 * @param pipeline Pipeline to stop
 * @param wait_for_completion Wait for current items to complete
 * @return Result indicating success or failure
 */
AsthraResult Asthra_pipeline_stop(AsthraConcurrencyPipeline* pipeline, 
                                 bool wait_for_completion);

/**
 * Get pipeline statistics
 * @param pipeline Pipeline to query
 * @param items_processed Total items processed
 * @param stage_stats Per-stage statistics (array of size num_stages)
 * @return Result indicating success or failure
 */
AsthraResult Asthra_pipeline_get_stats(AsthraConcurrencyPipeline* pipeline,
                                      uint64_t* items_processed,
                                      uint64_t* stage_stats);

/**
 * Destroy a pipeline
 * @param pipeline Pipeline to destroy
 */
void Asthra_pipeline_destroy(AsthraConcurrencyPipeline* pipeline);

// =============================================================================
// FAN-OUT OPERATIONS
// =============================================================================

/**
 * Create a fan-out pattern for distributing work
 * @param input_channel Channel with input data
 * @param output_channels Array of output channels
 * @param num_outputs Number of output channels
 * @param worker_func Function to process each item
 * @return Fan-out handle or NULL on failure
 */
AsthraConcurrencyFanOut* Asthra_fan_out_create(
    AsthraConcurrencyChannel* input_channel,
    AsthraConcurrencyChannel** output_channels,
    size_t num_outputs,
    AsthraConcurrencyTaskFunction worker_func);

/**
 * Start fan-out operation
 * @param fan_out Fan-out to start
 * @return Result indicating success or failure
 */
AsthraResult Asthra_fan_out_start(AsthraConcurrencyFanOut* fan_out);

/**
 * Stop fan-out operation
 * @param fan_out Fan-out to stop
 * @return Result indicating success or failure
 */
AsthraResult Asthra_fan_out_stop(AsthraConcurrencyFanOut* fan_out);

/**
 * Destroy fan-out
 * @param fan_out Fan-out to destroy
 */
void Asthra_fan_out_destroy(AsthraConcurrencyFanOut* fan_out);

// =============================================================================
// FAN-IN OPERATIONS
// =============================================================================

/**
 * Create a fan-in pattern for collecting results
 * @param input_channels Array of input channels
 * @param num_inputs Number of input channels
 * @param output_channel Channel for collected results
 * @param collector_func Function to process collected items
 * @return Fan-in handle or NULL on failure
 */
AsthraConcurrencyFanIn* Asthra_fan_in_create(
    AsthraConcurrencyChannel** input_channels,
    size_t num_inputs,
    AsthraConcurrencyChannel* output_channel,
    AsthraConcurrencyTaskFunction collector_func);

/**
 * Start fan-in operation
 * @param fan_in Fan-in to start
 * @return Result indicating success or failure
 */
AsthraResult Asthra_fan_in_start(AsthraConcurrencyFanIn* fan_in);

/**
 * Stop fan-in operation
 * @param fan_in Fan-in to stop
 * @return Result indicating success or failure
 */
AsthraResult Asthra_fan_in_stop(AsthraConcurrencyFanIn* fan_in);

/**
 * Destroy fan-in
 * @param fan_in Fan-in to destroy
 */
void Asthra_fan_in_destroy(AsthraConcurrencyFanIn* fan_in);

// =============================================================================
// CONVENIENCE PATTERN FUNCTIONS
// =============================================================================

/**
 * Simple fan-out pattern: distribute work to multiple workers
 * @param input_channel Channel with input data
 * @param output_channels Array of output channels
 * @param num_outputs Number of output channels
 * @param worker_func Function to process each item
 * @return Result indicating success or failure
 */
AsthraResult Asthra_pattern_fan_out(AsthraConcurrencyChannel* input_channel,
                                   AsthraConcurrencyChannel** output_channels,
                                   size_t num_outputs,
                                   AsthraConcurrencyTaskFunction worker_func);

/**
 * Simple fan-in pattern: collect results from multiple workers
 * @param input_channels Array of input channels
 * @param num_inputs Number of input channels
 * @param output_channel Channel for collected results
 * @param collector_func Function to process collected items
 * @return Result indicating success or failure
 */
AsthraResult Asthra_pattern_fan_in(AsthraConcurrencyChannel** input_channels,
                                  size_t num_inputs,
                                  AsthraConcurrencyChannel* output_channel,
                                  AsthraConcurrencyTaskFunction collector_func);

/**
 * Simple pipeline pattern: chain processing stages
 * @param stages Array of processing functions
 * @param num_stages Number of stages
 * @param input_channel Initial input channel
 * @param output_channel Final output channel
 * @return Result indicating success or failure
 */
AsthraResult Asthra_pattern_pipeline(AsthraConcurrencyTaskFunction* stages,
                                    size_t num_stages,
                                    AsthraConcurrencyChannel* input_channel,
                                    AsthraConcurrencyChannel* output_channel);

/**
 * Map-reduce pattern: apply function to all items and reduce results
 * @param input_channel Channel with input items
 * @param output_channel Channel for final result
 * @param map_func Function to apply to each item
 * @param reduce_func Function to combine results
 * @param num_workers Number of parallel workers
 * @return Result indicating success or failure
 */
AsthraResult Asthra_pattern_map_reduce(AsthraConcurrencyChannel* input_channel,
                                      AsthraConcurrencyChannel* output_channel,
                                      AsthraConcurrencyTaskFunction map_func,
                                      AsthraConcurrencyTaskFunction reduce_func,
                                      size_t num_workers);

/**
 * Producer-consumer pattern with bounded buffer
 * @param producer_func Producer function
 * @param consumer_func Consumer function
 * @param buffer_size Buffer size between producer and consumer
 * @param num_producers Number of producer threads
 * @param num_consumers Number of consumer threads
 * @return Result indicating success or failure
 */
AsthraResult Asthra_pattern_producer_consumer(AsthraConcurrencyTaskFunction producer_func,
                                             AsthraConcurrencyTaskFunction consumer_func,
                                             size_t buffer_size,
                                             size_t num_producers,
                                             size_t num_consumers);

// =============================================================================
// PATTERN STATISTICS
// =============================================================================

/**
 * Pattern system statistics
 */
typedef struct {
    asthra_concurrency_atomic_counter_t worker_pools_created;  // Total worker pools created
    asthra_concurrency_atomic_counter_t worker_pools_active;   // Currently active worker pools
    asthra_concurrency_atomic_counter_t pipelines_created;     // Total pipelines created
    asthra_concurrency_atomic_counter_t pipelines_active;      // Currently active pipelines
    asthra_concurrency_atomic_counter_t fan_outs_created;      // Total fan-outs created
    asthra_concurrency_atomic_counter_t fan_ins_created;       // Total fan-ins created
    asthra_concurrency_atomic_counter_t total_pattern_tasks;   // Total pattern tasks executed
    asthra_concurrency_atomic_counter_t pattern_task_failures; // Pattern task failures
} AsthraConcurrencyPatternStats;

/**
 * Get pattern system statistics
 * @return Pattern statistics structure
 */
AsthraConcurrencyPatternStats Asthra_get_pattern_stats(void);

/**
 * Reset pattern statistics
 */
void Asthra_reset_pattern_stats(void);

// =============================================================================
// PATTERN SYSTEM MANAGEMENT
// =============================================================================

/**
 * Initialize the pattern system
 * @return Result indicating success or failure
 */
AsthraResult Asthra_pattern_system_init(void);

/**
 * Cleanup the pattern system
 */
void Asthra_pattern_system_cleanup(void);

// =============================================================================
// ERROR CODES
// =============================================================================

#define ASTHRA_PATTERN_ERROR_BASE 0x4000

typedef enum {
    ASTHRA_PATTERN_ERROR_NONE = 0,
    ASTHRA_PATTERN_ERROR_INIT_FAILED = ASTHRA_PATTERN_ERROR_BASE + 1,
    ASTHRA_PATTERN_ERROR_POOL_FULL = ASTHRA_PATTERN_ERROR_BASE + 2,
    ASTHRA_PATTERN_ERROR_INVALID_CONFIG = ASTHRA_PATTERN_ERROR_BASE + 3,
    ASTHRA_PATTERN_ERROR_ALREADY_RUNNING = ASTHRA_PATTERN_ERROR_BASE + 4,
    ASTHRA_PATTERN_ERROR_NOT_RUNNING = ASTHRA_PATTERN_ERROR_BASE + 5,
    ASTHRA_PATTERN_ERROR_PIPELINE_FAILED = ASTHRA_PATTERN_ERROR_BASE + 6,
    ASTHRA_PATTERN_ERROR_WORKER_FAILED = ASTHRA_PATTERN_ERROR_BASE + 7,
    ASTHRA_PATTERN_ERROR_SYSTEM_ERROR = ASTHRA_PATTERN_ERROR_BASE + 8
} AsthraConcurrencyPatternErrorCode;

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_CONCURRENCY_PATTERNS_H
