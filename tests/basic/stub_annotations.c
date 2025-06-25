/**
 * Annotation-related stub functions for comprehensive tests
 */

#include <stdarg.h>
#include <stdbool.h>

// Annotation validation functions
bool analyze_annotation_inheritance(void *analyzer, void *node) {
    (void)analyzer;
    (void)node;
    return true;
}

bool check_annotation_conflicts(void *analyzer, void *node) {
    (void)analyzer;
    (void)node;
    return true;
}

bool check_duplicate_annotations(void *analyzer, void *node) {
    (void)analyzer;
    (void)node;
    return true;
}

bool is_annotation_valid(void *annotation) {
    (void)annotation;
    return true;
}

bool validate_annotation_dependencies(void *analyzer, void *annotations) {
    (void)analyzer;
    (void)annotations;
    return true;
}

// Annotation error reporting
void issue_annotation_warning(void *analyzer, const char *msg, ...) {
    (void)analyzer;
    (void)msg;
}

void report_annotation_conflict_error(void *analyzer, void *ann1, void *ann2) {
    (void)analyzer;
    (void)ann1;
    (void)ann2;
}

void report_context_validation_error(void *analyzer, void *context, const char *msg) {
    (void)analyzer;
    (void)context;
    (void)msg;
}

void report_parameter_validation_error(void *analyzer, void *param, const char *msg) {
    (void)analyzer;
    (void)param;
    (void)msg;
}

void report_unknown_annotation_error(void *analyzer, void *annotation) {
    (void)analyzer;
    (void)annotation;
}