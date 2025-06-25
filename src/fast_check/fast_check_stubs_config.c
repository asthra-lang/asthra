// This file provides stub implementations for configuration functions
// when the actual fast_check implementation is not available.
#ifdef FAST_CHECK_USE_STUBS

#include "fast_check_engine.h"
#include <stdbool.h>
#include <stdlib.h>

// =============================================================================
// CONFIGURATION
// =============================================================================

FastCheckConfig *fast_check_config_create_default(void) {
    FastCheckConfig *config = calloc(1, sizeof(FastCheckConfig));
    if (!config)
        return NULL;

    config->max_cache_memory_mb = 256;
    config->cache_ttl_seconds = 300;
    config->enable_dependency_tracking = true;
    config->enable_incremental_parsing = true;

    return config;
}

void fast_check_config_destroy(FastCheckConfig *config) {
    free(config);
}

FastCheckConfig *fast_check_config_create(void) {
    return fast_check_config_create_default();
}

void fast_check_config_set_watch_mode(FastCheckConfig *config, bool watch_mode) {
    // Stub implementation - config may not have this field yet
    (void)config;
    (void)watch_mode;
}

void fast_check_config_set_syntax_only(FastCheckConfig *config, bool syntax_only) {
    // Stub implementation
    (void)config;
    (void)syntax_only;
}

void fast_check_config_set_timeout(FastCheckConfig *config, int timeout_ms) {
    // Stub implementation
    (void)config;
    (void)timeout_ms;
}

void fast_check_config_set_output_format(FastCheckConfig *config, const char *format) {
    // Stub implementation
    (void)config;
    (void)format;
}

void fast_check_config_add_include_pattern(FastCheckConfig *config, const char *pattern) {
    // Stub implementation
    (void)config;
    (void)pattern;
}

void fast_check_config_add_exclude_pattern(FastCheckConfig *config, const char *pattern) {
    // Stub implementation
    (void)config;
    (void)pattern;
}

#else // FAST_CHECK_USE_STUBS

// Dummy declaration to avoid empty translation unit warning
static int fast_check_stubs_config_dummy = 0;

#endif // FAST_CHECK_USE_STUBS