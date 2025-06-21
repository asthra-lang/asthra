#ifndef ASTHRA_ENHANCED_ERROR_REPORTING_H
#define ASTHRA_ENHANCED_ERROR_REPORTING_H

#include "../diagnostics/enhanced_diagnostics.h"
#include <stddef.h>
#include <stdbool.h>

// Enhanced error reporter for compiler integration
typedef struct {
    EnhancedDiagnostic **diagnostics;
    size_t diagnostic_count;
    size_t capacity;
    
    // Configuration
    bool enable_suggestions;
    bool enable_metadata;
    ConfidenceLevel min_confidence;
} EnhancedErrorReporter;

// Core error reporting API
EnhancedErrorReporter *enhanced_error_reporter_create(void);
void enhanced_error_reporter_destroy(EnhancedErrorReporter *reporter);

// Error reporting functions conforming to current PEG grammar
void enhanced_error_reporter_report_undefined_variable(EnhancedErrorReporter *reporter, 
                                                      const char *var_name, 
                                                      size_t line, size_t column,
                                                      const char *file_path);
void enhanced_error_reporter_report_type_mismatch(EnhancedErrorReporter *reporter,
                                                  const char *expected_type,
                                                  const char *actual_type,
                                                  size_t line, size_t column,
                                                  const char *file_path);
void enhanced_error_reporter_report_missing_visibility(EnhancedErrorReporter *reporter,
                                                      const char *decl_type,
                                                      size_t line, size_t column,
                                                      const char *file_path);
void enhanced_error_reporter_report_missing_parameter_list(EnhancedErrorReporter *reporter,
                                                          const char *func_name,
                                                          size_t line, size_t column,
                                                          const char *file_path);
void enhanced_error_reporter_report_missing_struct_content(EnhancedErrorReporter *reporter,
                                                          const char *struct_name,
                                                          size_t line, size_t column,
                                                          const char *file_path);

// Output generation
char *enhanced_error_reporter_to_json(EnhancedErrorReporter *reporter);
void enhanced_error_reporter_print_human_readable(EnhancedErrorReporter *reporter);

// Utility functions
bool enhanced_error_reporter_has_errors(EnhancedErrorReporter *reporter);
size_t enhanced_error_reporter_get_error_count(EnhancedErrorReporter *reporter);
void enhanced_error_reporter_clear(EnhancedErrorReporter *reporter);

#endif 
