/**
 * Asthra Programming Language
 * Common Error Framework for Tools - Implementation
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "error_framework.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define INITIAL_ERROR_CAPACITY 16

static uint64_t get_timestamp_ms(void) {
    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) == 0) {
        return (uint64_t)ts.tv_sec * 1000 + (uint64_t)ts.tv_nsec / 1000000;
    }
    return 0;
}

ErrorFramework* error_create_framework(const char *tool_name) {
    ErrorFramework *framework = malloc(sizeof(ErrorFramework));
    if (!framework) {
        return NULL;
    }
    
    framework->tool_name = tool_name;
    framework->errors = malloc(sizeof(ErrorMessage) * INITIAL_ERROR_CAPACITY);
    if (!framework->errors) {
        free(framework);
        return NULL;
    }
    
    framework->error_count = 0;
    framework->error_capacity = INITIAL_ERROR_CAPACITY;
    framework->warnings_as_errors = false;
    framework->min_severity = ERROR_WARNING;
    
    return framework;
}

void error_destroy_framework(ErrorFramework *framework) {
    if (framework) {
        free(framework->errors);
        free(framework);
    }
}

static bool ensure_error_capacity(ErrorFramework *framework) {
    if (framework->error_count >= framework->error_capacity) {
        size_t new_capacity = framework->error_capacity * 2;
        ErrorMessage *new_errors = realloc(framework->errors, 
                                          sizeof(ErrorMessage) * new_capacity);
        if (!new_errors) {
            return false;
        }
        
        framework->errors = new_errors;
        framework->error_capacity = new_capacity;
    }
    
    return true;
}

void error_report(ErrorFramework *framework, ErrorSeverity severity, 
                 const char *message, const char *suggestion,
                 const char *file_name, size_t line_number, size_t column_number) {
    if (!framework || !message) {
        return;
    }
    
    // Apply warnings_as_errors setting
    if (framework->warnings_as_errors && severity == ERROR_WARNING) {
        severity = ERROR_ERROR;
    }
    
    // Check minimum severity
    if (severity < framework->min_severity) {
        return;
    }
    
    // Ensure capacity
    if (!ensure_error_capacity(framework)) {
        return;
    }
    
    // Add error
    ErrorMessage *error = &framework->errors[framework->error_count];
    error->severity = severity;
    error->message = message;
    error->suggestion = suggestion;
    error->context.file_name = file_name;
    error->context.line_number = line_number;
    error->context.column_number = column_number;
    error->context.function_name = NULL;
    error->timestamp_ms = get_timestamp_ms();
    
    framework->error_count++;
}

void error_report_simple(ErrorFramework *framework, ErrorSeverity severity, const char *message) {
    error_report(framework, severity, message, NULL, NULL, 0, 0);
}

size_t error_get_count(const ErrorFramework *framework, ErrorSeverity severity) {
    if (!framework) {
        return 0;
    }
    
    size_t count = 0;
    for (size_t i = 0; i < framework->error_count; i++) {
        if (framework->errors[i].severity == severity) {
            count++;
        }
    }
    
    return count;
}

size_t error_get_total_count(const ErrorFramework *framework) {
    return framework ? framework->error_count : 0;
}

bool error_has_errors(const ErrorFramework *framework) {
    return error_get_count(framework, ERROR_ERROR) > 0 || 
           error_get_count(framework, ERROR_CRITICAL) > 0;
}

bool error_has_critical_errors(const ErrorFramework *framework) {
    return error_get_count(framework, ERROR_CRITICAL) > 0;
}

void error_print_summary(const ErrorFramework *framework, bool verbose) {
    if (!framework) {
        return;
    }
    
    size_t info_count = error_get_count(framework, ERROR_INFO);
    size_t warning_count = error_get_count(framework, ERROR_WARNING);
    size_t error_count = error_get_count(framework, ERROR_ERROR);
    size_t critical_count = error_get_count(framework, ERROR_CRITICAL);
    
    printf("\n%s Error Summary:\n", framework->tool_name);
    printf("=====================================\n");
    printf("  Info:     %zu\n", info_count);
    printf("  Warnings: %zu\n", warning_count);
    printf("  Errors:   %zu\n", error_count);
    printf("  Critical: %zu\n", critical_count);
    printf("  Total:    %zu\n", framework->error_count);
    printf("=====================================\n");
    
    if (verbose && framework->error_count > 0) {
        printf("\nDetailed Error List:\n");
        error_print_all(framework);
    }
}

void error_print_json(const ErrorFramework *framework) {
    if (!framework) {
        return;
    }
    
    printf("{\n");
    printf("  \"tool\": \"%s\",\n", framework->tool_name);
    printf("  \"error_summary\": {\n");
    printf("    \"info\": %zu,\n", error_get_count(framework, ERROR_INFO));
    printf("    \"warnings\": %zu,\n", error_get_count(framework, ERROR_WARNING));
    printf("    \"errors\": %zu,\n", error_get_count(framework, ERROR_ERROR));
    printf("    \"critical\": %zu,\n", error_get_count(framework, ERROR_CRITICAL));
    printf("    \"total\": %zu\n", framework->error_count);
    printf("  },\n");
    printf("  \"errors\": [\n");
    
    for (size_t i = 0; i < framework->error_count; i++) {
        const ErrorMessage *error = &framework->errors[i];
        printf("    {\n");
        printf("      \"severity\": \"%s\",\n", error_severity_to_string(error->severity));
        printf("      \"message\": \"%s\"", error->message);
        
        if (error->suggestion) {
            printf(",\n      \"suggestion\": \"%s\"", error->suggestion);
        }
        
        if (error->context.file_name) {
            printf(",\n      \"file\": \"%s\"", error->context.file_name);
            if (error->context.line_number > 0) {
                printf(",\n      \"line\": %zu", error->context.line_number);
            }
            if (error->context.column_number > 0) {
                printf(",\n      \"column\": %zu", error->context.column_number);
            }
        }
        
        printf("\n    }");
        if (i < framework->error_count - 1) {
            printf(",");
        }
        printf("\n");
    }
    
    printf("  ]\n");
    printf("}\n");
}

void error_print_all(const ErrorFramework *framework) {
    if (!framework) {
        return;
    }
    
    for (size_t i = 0; i < framework->error_count; i++) {
        const ErrorMessage *error = &framework->errors[i];
        
        printf("[%s] %s", error_severity_to_string(error->severity), error->message);
        
        if (error->context.file_name) {
            printf(" (%s", error->context.file_name);
            if (error->context.line_number > 0) {
                printf(":%zu", error->context.line_number);
                if (error->context.column_number > 0) {
                    printf(":%zu", error->context.column_number);
                }
            }
            printf(")");
        }
        
        printf("\n");
        
        if (error->suggestion) {
            printf("  Suggestion: %s\n", error->suggestion);
        }
    }
}

void error_set_warnings_as_errors(ErrorFramework *framework, bool enabled) {
    if (framework) {
        framework->warnings_as_errors = enabled;
    }
}

void error_set_min_severity(ErrorFramework *framework, ErrorSeverity min_severity) {
    if (framework) {
        framework->min_severity = min_severity;
    }
}

const char* error_severity_to_string(ErrorSeverity severity) {
    switch (severity) {
        case ERROR_INFO:     return "info";
        case ERROR_WARNING:  return "warning";
        case ERROR_ERROR:    return "error";
        case ERROR_CRITICAL: return "critical";
        default:             return "unknown";
    }
}

ErrorSeverity error_string_to_severity(const char *severity_str) {
    if (!severity_str) {
        return ERROR_WARNING;
    }
    
    if (strcmp(severity_str, "info") == 0) {
        return ERROR_INFO;
    } else if (strcmp(severity_str, "warning") == 0) {
        return ERROR_WARNING;
    } else if (strcmp(severity_str, "error") == 0) {
        return ERROR_ERROR;
    } else if (strcmp(severity_str, "critical") == 0) {
        return ERROR_CRITICAL;
    }
    
    return ERROR_WARNING;
} 
