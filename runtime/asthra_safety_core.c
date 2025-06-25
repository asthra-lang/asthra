/**
 * Asthra Safety System - Core Module
 * System initialization, global state management, and performance metrics
 */

#include "asthra_safety_core.h"
#include "asthra_runtime.h"
#include "asthra_safety_common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// =============================================================================
// GLOBAL SAFETY STATE
// =============================================================================

static AsthraSafetyConfig g_safety_config = {0};
static bool g_safety_initialized = false;
static pthread_mutex_t g_safety_mutex = PTHREAD_MUTEX_INITIALIZER;
static AsthraSafetyPerformanceMetrics g_performance_metrics = {0};

// =============================================================================
// DEFAULT SAFETY CONFIGURATIONS
// =============================================================================

const AsthraSafetyConfig ASTHRA_SAFETY_CONFIG_DEBUG = {.level = ASTHRA_SAFETY_LEVEL_ENHANCED,
                                                       .enable_parser_validation = true,
                                                       .enable_pattern_matching_checks = true,
                                                       .enable_type_safety_checks = true,
                                                       .enable_ffi_annotation_verification = true,
                                                       .enable_boundary_checks = true,
                                                       .enable_ownership_tracking = true,
                                                       .enable_variadic_validation = true,
                                                       .enable_string_operation_validation = true,
                                                       .enable_slice_bounds_checking = true,
                                                       .enable_memory_layout_validation = true,
                                                       .enable_concurrency_debugging = true,
                                                       .enable_error_handling_aids = true,
                                                       .enable_security_enforcement = true,
                                                       .enable_stack_canaries = true,
                                                       .enable_ffi_call_logging = true,
                                                       .enable_constant_time_verification = false,
                                                       .enable_secure_memory_validation = true,
                                                       .enable_fault_injection_testing = false,
                                                       .enable_performance_monitoring = true};

const AsthraSafetyConfig ASTHRA_SAFETY_CONFIG_RELEASE = {
    .level = ASTHRA_SAFETY_LEVEL_BASIC,
    .enable_parser_validation = false,
    .enable_pattern_matching_checks = false,
    .enable_type_safety_checks = false,
    .enable_ffi_annotation_verification = false,
    .enable_boundary_checks = true,
    .enable_ownership_tracking = false,
    .enable_variadic_validation = false,
    .enable_string_operation_validation = false,
    .enable_slice_bounds_checking = true,
    .enable_memory_layout_validation = false,
    .enable_concurrency_debugging = false,
    .enable_error_handling_aids = false,
    .enable_security_enforcement = false,
    .enable_stack_canaries = false,
    .enable_ffi_call_logging = false,
    .enable_constant_time_verification = false,
    .enable_secure_memory_validation = false,
    .enable_fault_injection_testing = false,
    .enable_performance_monitoring = false};

// =============================================================================
// GLOBAL STATE ACCESSORS
// =============================================================================

AsthraSafetyConfig *asthra_safety_get_config_ptr(void) {
    return &g_safety_config;
}

AsthraSafetyConfig asthra_safety_get_config(void) {
    return g_safety_config;
}

bool asthra_safety_is_initialized(void) {
    return g_safety_initialized;
}

pthread_mutex_t *asthra_safety_get_mutex(void) {
    return &g_safety_mutex;
}

AsthraSafetyPerformanceMetrics *asthra_safety_get_metrics_ptr(void) {
    return &g_performance_metrics;
}

// =============================================================================
// VIOLATION REPORTING
// =============================================================================

void asthra_safety_report_violation(AsthraViolationType type, AsthraSafetyLevel severity,
                                    const char *message, const char *location, int line,
                                    const char *function, void *context, size_t context_size) {
    if (!g_safety_initialized) {
        return;
    }

    // Update performance metrics
    __sync_fetch_and_add(&g_performance_metrics.violations_detected, 1);

    // Log the violation
    AsthraLogLevel log_level;
    switch (severity) {
    case ASTHRA_SAFETY_LEVEL_BASIC:
        log_level = ASTHRA_LOG_WARN;
        break;
    case ASTHRA_SAFETY_LEVEL_STANDARD:
        log_level = ASTHRA_LOG_ERROR;
        break;
    case ASTHRA_SAFETY_LEVEL_ENHANCED:
    case ASTHRA_SAFETY_LEVEL_PARANOID:
        log_level = ASTHRA_LOG_FATAL;
        break;
    default:
        log_level = ASTHRA_LOG_ERROR;
        break;
    }

    asthra_log(log_level, ASTHRA_LOG_CATEGORY_GENERAL, "SAFETY VIOLATION [%d]: %s at %s:%d in %s",
               type, message, location, line, function);

    // In debug builds, we might want to abort on certain violations
    if (severity >= ASTHRA_SAFETY_LEVEL_ENHANCED &&
        g_safety_config.level >= ASTHRA_SAFETY_LEVEL_ENHANCED) {
        fprintf(stderr, "FATAL SAFETY VIOLATION: %s\n", message);
        abort();
    }
}

// =============================================================================
// SYSTEM INITIALIZATION AND MANAGEMENT
// =============================================================================

int asthra_safety_init(const AsthraSafetyConfig *config) {
    pthread_mutex_lock(&g_safety_mutex);

    if (g_safety_initialized) {
        pthread_mutex_unlock(&g_safety_mutex);
        return 0;
    }

    if (config) {
        g_safety_config = *config;
    } else {
        g_safety_config = ASTHRA_SAFETY_CONFIG_DEBUG;
    }

    // Initialize performance metrics
    memset(&g_performance_metrics, 0, sizeof(g_performance_metrics));

    g_safety_initialized = true;
    pthread_mutex_unlock(&g_safety_mutex);

    asthra_log(ASTHRA_LOG_INFO, ASTHRA_LOG_CATEGORY_GENERAL,
               "Asthra safety system initialized with level %d", g_safety_config.level);

    return 0;
}

void asthra_safety_cleanup(void) {
    pthread_mutex_lock(&g_safety_mutex);

    if (!g_safety_initialized) {
        pthread_mutex_unlock(&g_safety_mutex);
        return;
    }

    // Call module-specific cleanup functions
    extern void asthra_safety_memory_ffi_cleanup(void);
    extern void asthra_safety_concurrency_errors_cleanup(void);
    extern void asthra_safety_security_cleanup(void);

    asthra_safety_memory_ffi_cleanup();
    asthra_safety_concurrency_errors_cleanup();
    asthra_safety_security_cleanup();

    // Reset configuration and metrics
    memset(&g_safety_config, 0, sizeof(g_safety_config));
    memset(&g_performance_metrics, 0, sizeof(g_performance_metrics));

    g_safety_initialized = false;
    pthread_mutex_unlock(&g_safety_mutex);

    asthra_log(ASTHRA_LOG_INFO, ASTHRA_LOG_CATEGORY_GENERAL, "Asthra safety system cleaned up");
}

// =============================================================================
// PERFORMANCE METRICS
// =============================================================================

AsthraSafetyPerformanceMetrics asthra_safety_get_performance_metrics(void) {
    pthread_mutex_lock(&g_safety_mutex);
    AsthraSafetyPerformanceMetrics metrics = g_performance_metrics;
    pthread_mutex_unlock(&g_safety_mutex);
    return metrics;
}

void asthra_safety_reset_performance_metrics(void) {
    pthread_mutex_lock(&g_safety_mutex);
    memset(&g_performance_metrics, 0, sizeof(g_performance_metrics));
    pthread_mutex_unlock(&g_safety_mutex);
}
