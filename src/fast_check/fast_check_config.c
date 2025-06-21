#include "fast_check_command.h"
#include "fast_check_engine.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Configuration Management Implementation

FastCheckConfig *fast_check_config_create(void) {
    FastCheckConfig *config = calloc(1, sizeof(FastCheckConfig));
    if (!config) return NULL;
    
    // Set defaults
    config->watch_mode = false;
    config->syntax_only = false;
    config->timeout_ms = 1000;  // 1 second default
    config->clear_cache = false;
    config->output_format = strdup("human");
    config->include_patterns = NULL;
    config->exclude_patterns = NULL;
    config->include_count = 0;
    config->exclude_count = 0;
    
    return config;
}

void fast_check_config_set_watch_mode(FastCheckConfig *config, bool enabled) {
    if (config) config->watch_mode = enabled;
}

void fast_check_config_set_syntax_only(FastCheckConfig *config, bool enabled) {
    if (config) config->syntax_only = enabled;
}

void fast_check_config_set_timeout(FastCheckConfig *config, uint64_t timeout_ms) {
    if (config) config->timeout_ms = timeout_ms;
}

void fast_check_config_set_output_format(FastCheckConfig *config, const char *format) {
    if (!config || !format) return;
    
    free(config->output_format);
    config->output_format = strdup(format);
}

void fast_check_config_add_include_pattern(FastCheckConfig *config, const char *pattern) {
    if (!config || !pattern) return;
    
    config->include_patterns = realloc(config->include_patterns, 
                                     (config->include_count + 1) * sizeof(char*));
    config->include_patterns[config->include_count] = strdup(pattern);
    config->include_count++;
}

void fast_check_config_add_exclude_pattern(FastCheckConfig *config, const char *pattern) {
    if (!config || !pattern) return;
    
    config->exclude_patterns = realloc(config->exclude_patterns, 
                                     (config->exclude_count + 1) * sizeof(char*));
    config->exclude_patterns[config->exclude_count] = strdup(pattern);
    config->exclude_count++;
}