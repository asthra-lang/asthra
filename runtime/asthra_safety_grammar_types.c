/**
 * Asthra Safety System - Grammar and Type Safety Module
 * Parser validation, pattern matching checks, and type safety validation
 */

#include "asthra_runtime.h"
#include "asthra_safety_common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// =============================================================================
// GRAMMAR VALIDATION IMPLEMENTATION
// =============================================================================

AsthraGrammarValidation asthra_safety_validate_grammar(const char *source_code,
                                                       size_t code_length) {
    AsthraGrammarValidation validation = {0};
    AsthraSafetyConfig *config = asthra_safety_get_config_ptr();

    if (!config->enable_parser_validation) {
        validation.result = ASTHRA_GRAMMAR_VALID;
        return validation;
    }

    if (!source_code || code_length == 0) {
        validation.result = ASTHRA_GRAMMAR_INVALID_SYNTAX;
        snprintf(validation.error_message, sizeof(validation.error_message),
                 "Invalid source code: null pointer or zero length");
        return validation;
    }

    // Basic syntax validation - check for balanced braces, parentheses, brackets
    int brace_count = 0, paren_count = 0, bracket_count = 0;
    int line = 1, column = 1;

    for (size_t i = 0; i < code_length; i++) {
        char c = source_code[i];

        switch (c) {
        case '{':
            brace_count++;
            break;
        case '}':
            brace_count--;
            break;
        case '(':
            paren_count++;
            break;
        case ')':
            paren_count--;
            break;
        case '[':
            bracket_count++;
            break;
        case ']':
            bracket_count--;
            break;
        case '\n':
            line++;
            column = 1;
            continue;
        }

        column++;

        // Check for negative counts (unmatched closing)
        if (brace_count < 0 || paren_count < 0 || bracket_count < 0) {
            validation.result = ASTHRA_GRAMMAR_INVALID_SYNTAX;
            validation.line_number = line;
            validation.column_number = column;
            snprintf(validation.error_message, sizeof(validation.error_message),
                     "Unmatched closing delimiter at line %d, column %d", line, column);
            return validation;
        }
    }

    // Check for unmatched opening delimiters
    if (brace_count != 0 || paren_count != 0 || bracket_count != 0) {
        validation.result = ASTHRA_GRAMMAR_INVALID_SYNTAX;
        snprintf(validation.error_message, sizeof(validation.error_message),
                 "Unmatched delimiters: braces=%d, parentheses=%d, brackets=%d", brace_count,
                 paren_count, bracket_count);
        return validation;
    }

    validation.result = ASTHRA_GRAMMAR_VALID;
    return validation;
}

// =============================================================================
// PATTERN MATCHING COMPLETENESS IMPLEMENTATION
// =============================================================================

AsthraPatternCompletenessCheck asthra_safety_check_pattern_completeness(AsthraMatchArm *arms,
                                                                        size_t arm_count,
                                                                        uint32_t result_type_id) {
    AsthraPatternCompletenessCheck check = {0};
    AsthraSafetyConfig *config = asthra_safety_get_config_ptr();

    if (!config->enable_pattern_matching_checks) {
        check.result = ASTHRA_PATTERN_COMPLETE;
        return check;
    }

    if (!arms || arm_count == 0) {
        check.result = ASTHRA_PATTERN_INCOMPLETE;
        snprintf(check.missing_patterns, sizeof(check.missing_patterns), "No patterns provided");
        return check;
    }

    bool has_ok_pattern = false;
    bool has_err_pattern = false;
    bool has_wildcard = false;

    check.pattern_count = arm_count;

    for (size_t i = 0; i < arm_count; i++) {
        switch (arms[i].pattern) {
        case ASTHRA_MATCH_OK:
            if (has_ok_pattern) {
                check.result = ASTHRA_PATTERN_REDUNDANT;
                snprintf(check.unreachable_patterns, sizeof(check.unreachable_patterns),
                         "Duplicate Ok pattern at index %zu", i);
                return check;
            }
            has_ok_pattern = true;
            check.covered_patterns++;
            break;

        case ASTHRA_MATCH_ERR:
            if (has_err_pattern) {
                check.result = ASTHRA_PATTERN_REDUNDANT;
                snprintf(check.unreachable_patterns, sizeof(check.unreachable_patterns),
                         "Duplicate Err pattern at index %zu", i);
                return check;
            }
            has_err_pattern = true;
            check.covered_patterns++;
            break;

        case ASTHRA_MATCH_WILDCARD:
            if (has_wildcard) {
                check.result = ASTHRA_PATTERN_REDUNDANT;
                snprintf(check.unreachable_patterns, sizeof(check.unreachable_patterns),
                         "Duplicate wildcard pattern at index %zu", i);
                return check;
            }
            has_wildcard = true;
            check.covered_patterns++;
            break;

        default:
            // Unknown pattern type
            check.result = ASTHRA_PATTERN_INCOMPLETE;
            snprintf(check.missing_patterns, sizeof(check.missing_patterns),
                     "Unknown pattern type %d at index %zu", arms[i].pattern, i);
            return check;
        }
    }

    // Check completeness for Result types
    if (result_type_id == ASTHRA_TYPE_RESULT) {
        if (!has_wildcard && (!has_ok_pattern || !has_err_pattern)) {
            check.result = ASTHRA_PATTERN_INCOMPLETE;
            if (!has_ok_pattern && !has_err_pattern) {
                snprintf(check.missing_patterns, sizeof(check.missing_patterns),
                         "Missing both Ok and Err patterns for Result type");
            } else if (!has_ok_pattern) {
                snprintf(check.missing_patterns, sizeof(check.missing_patterns),
                         "Missing Ok pattern for Result type");
            } else {
                snprintf(check.missing_patterns, sizeof(check.missing_patterns),
                         "Missing Err pattern for Result type");
            }
            return check;
        }
    }

    check.result = ASTHRA_PATTERN_COMPLETE;
    return check;
}

// =============================================================================
// TYPE SAFETY VALIDATION IMPLEMENTATION
// =============================================================================

AsthraTypeSafetyCheck asthra_safety_validate_result_type_usage(AsthraResult result,
                                                               uint32_t expected_type_id) {
    AsthraTypeSafetyCheck check = {0};
    AsthraSafetyConfig *config = asthra_safety_get_config_ptr();

    if (!config->enable_type_safety_checks) {
        check.is_valid = true;
        return check;
    }

    check.expected_type_id = expected_type_id;
    check.actual_type_id = (result.tag == ASTHRA_RESULT_OK) ? result.data.ok.value_type_id
                                                            : result.data.err.error_type_id;
    check.context = "Result type validation";

    uint32_t actual_type_id = (result.tag == ASTHRA_RESULT_OK) ? result.data.ok.value_type_id
                                                               : result.data.err.error_type_id;
    if (actual_type_id != expected_type_id) {
        check.is_valid = false;
        snprintf(check.type_error_message, sizeof(check.type_error_message),
                 "Type mismatch: expected type %u, got type %u", expected_type_id, actual_type_id);

        asthra_safety_report_violation(ASTHRA_VIOLATION_TYPE_SAFETY, ASTHRA_SAFETY_LEVEL_STANDARD,
                                       check.type_error_message, __FILE__, __LINE__, __func__,
                                       &check, sizeof(check));
        return check;
    }

    // Additional validation for Result types
    if (expected_type_id == ASTHRA_TYPE_RESULT) {
        if (result.tag != ASTHRA_RESULT_OK && result.tag != ASTHRA_RESULT_ERR) {
            check.is_valid = false;
            snprintf(check.type_error_message, sizeof(check.type_error_message),
                     "Invalid Result tag: %d (expected OK or ERR)", result.tag);

            asthra_safety_report_violation(ASTHRA_VIOLATION_TYPE_SAFETY,
                                           ASTHRA_SAFETY_LEVEL_STANDARD, check.type_error_message,
                                           __FILE__, __LINE__, __func__, &check, sizeof(check));
            return check;
        }

        // Validate that Ok results have valid data
        if (result.tag == ASTHRA_RESULT_OK && !result.data.ok.value) {
            check.is_valid = false;
            snprintf(check.type_error_message, sizeof(check.type_error_message),
                     "Result marked as Ok but has null data");

            asthra_safety_report_violation(ASTHRA_VIOLATION_TYPE_SAFETY,
                                           ASTHRA_SAFETY_LEVEL_STANDARD, check.type_error_message,
                                           __FILE__, __LINE__, __func__, &check, sizeof(check));
            return check;
        }

        // Validate that Err results have valid error data
        if (result.tag == ASTHRA_RESULT_ERR && !result.data.err.error) {
            check.is_valid = false;
            snprintf(check.type_error_message, sizeof(check.type_error_message),
                     "Result marked as Err but has null error data");

            asthra_safety_report_violation(ASTHRA_VIOLATION_TYPE_SAFETY,
                                           ASTHRA_SAFETY_LEVEL_STANDARD, check.type_error_message,
                                           __FILE__, __LINE__, __func__, &check, sizeof(check));
            return check;
        }
    }

    check.is_valid = true;
    return check;
}
