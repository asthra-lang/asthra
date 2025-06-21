/**
 * Expression-Oriented Test Utilities
 * 
 * Utilities for handling Asthra's expression-oriented design in tests
 */

#ifndef EXPRESSION_ORIENTED_TEST_UTILS_H
#define EXPRESSION_ORIENTED_TEST_UTILS_H

#include <stdbool.h>
#include <string.h>
#include <stdio.h>

// Pattern transformation utilities for expression-oriented testing

/**
 * Transform an incomplete if statement to a complete if-else expression
 * Example: "if (c) { action(); }" -> "if (c) { action(); () } else { () }"
 */
static inline bool transform_incomplete_if(const char* input, char* output, size_t output_size) {
    // Look for if without else
    const char* if_pos = strstr(input, "if");
    if (!if_pos) {
        return false;
    }
    
    const char* else_pos = strstr(input, "else");
    if (else_pos) {
        // Already has else, no transformation needed
        strncpy(output, input, output_size - 1);
        output[output_size - 1] = '\0';
        return false;
    }
    
    // Find the closing brace of the if body
    int brace_count = 0;
    const char* p = if_pos;
    const char* closing_brace = NULL;
    
    while (*p) {
        if (*p == '{') {
            brace_count++;
        } else if (*p == '}') {
            brace_count--;
            if (brace_count == 0) {
                closing_brace = p;
                break;
            }
        }
        p++;
    }
    
    if (!closing_brace) {
        return false;
    }
    
    // Build the transformed expression
    size_t prefix_len = closing_brace - input;
    if (prefix_len >= output_size - 20) {  // Need space for " else { () }"
        return false;
    }
    
    // Copy up to the closing brace
    strncpy(output, input, prefix_len);
    output[prefix_len] = '\0';
    
    // Check if the if body already returns unit
    const char* body_start = strchr(if_pos, '{');
    if (body_start && body_start < closing_brace) {
        const char* return_unit = strstr(body_start, "()");
        if (!return_unit || return_unit > closing_brace) {
            // Add unit return before closing brace
            strcat(output, "; ()");
        }
    }
    
    // Add the else branch
    strcat(output, " } else { () }");
    
    // Add any remaining content after the original if
    if (*(closing_brace + 1)) {
        strcat(output, closing_brace + 1);
    }
    
    return true;
}

/**
 * Check if an expression returns a value (not void/unit)
 */
static inline bool is_expression_returning_value(const char* expr) {
    // Simple heuristics - can be expanded
    if (strstr(expr, "func1(") || strstr(expr, "func2(") || 
        strstr(expr, "+ ") || strstr(expr, "- ") ||
        strstr(expr, "* ") || strstr(expr, "/ ") ||
        strstr(expr, "get_value(") || strstr(expr, "compute_")) {
        return true;
    }
    
    // Check for method calls that return values
    if (strstr(expr, ".scale(") || strstr(expr, "method1(") || 
        strstr(expr, "method2(")) {
        return true;
    }
    
    return false;
}

/**
 * Transform if-else with void calls to ensure proper void context
 */
static inline bool transform_if_else_for_void_context(const char* input, char* output, size_t output_size) {
    // This is specifically for complete if-else statements that call void functions
    if (!strstr(input, "if") || !strstr(input, "else")) {
        return false;
    }
    
    // Check if it contains void function calls (action1, action2, etc.)
    if (strstr(input, "action1()") || strstr(input, "action2()") || 
        strstr(input, "process()") || strstr(input, "update()")) {
        // For void if-else, we need to ensure each branch returns unit
        // Transform: if (c) { f(); } else { g(); }
        // To:        if (c) { f(); () } else { g(); () }
        char temp[4096];
        strncpy(temp, input, sizeof(temp) - 1);
        temp[sizeof(temp) - 1] = '\0';
        
        // Replace "; }" with "; () }"
        char result[4096] = {0};
        char* write_pos = result;
        const char* read_pos = temp;
        
        while (*read_pos) {
            if (strncmp(read_pos, "; }", 3) == 0) {
                strcpy(write_pos, "; () }");
                write_pos += 6;
                read_pos += 3;
            } else {
                *write_pos++ = *read_pos++;
            }
        }
        *write_pos = '\0';
        
        strncpy(output, result, output_size - 1);
        output[output_size - 1] = '\0';
        return true;
    }
    
    // For non-void if-else, copy as-is
    strncpy(output, input, output_size - 1);
    output[output_size - 1] = '\0';
    return false;
}

/**
 * Wrap a code fragment appropriately for expression-oriented context
 */
static inline void wrap_fragment_for_expression_context(const char* fragment, char* output, size_t output_size) {
    // Handle different types of fragments
    
    // Skip transformations for already-valid expression-oriented syntax
    // If it's a let statement with an if-else expression, it's already correct
    if (strstr(fragment, "let") && strstr(fragment, "=") && strstr(fragment, "if") && strstr(fragment, "else")) {
        strncpy(output, fragment, output_size - 1);
        output[output_size - 1] = '\0';
        return;
    }
    
    // 1. Incomplete if statements (must add else branch)
    if (strstr(fragment, "if") && !strstr(fragment, "else")) {
        if (transform_incomplete_if(fragment, output, output_size)) {
            return;
        }
    }
    
    // 2. Complete if-else statements with void function calls
    if (strstr(fragment, "if") && strstr(fragment, "else") && 
        (strstr(fragment, "action1()") || strstr(fragment, "action2()") || 
         strstr(fragment, "();") == NULL)) {
        if (transform_if_else_for_void_context(fragment, output, output_size)) {
            return;
        }
    }
    
    // 3. Expressions returning values in void context
    if (is_expression_returning_value(fragment)) {
        // Don't wrap if it's already an assignment or return statement
        if (!strstr(fragment, "=") && !strstr(fragment, "return")) {
            snprintf(output, output_size, "let _ = %s", fragment);
            return;
        }
    }
    
    // 4. For loops are statements - they work as-is
    
    // Default: copy as-is
    strncpy(output, fragment, output_size - 1);
    output[output_size - 1] = '\0';
}

/**
 * Test pattern mappings for common statement-to-expression transformations
 */
typedef struct {
    const char* statement_form;    // Statement-oriented pattern
    const char* expression_form;   // Expression-oriented equivalent
    const char* description;       // Explanation
} TestPatternMapping;

static const TestPatternMapping expression_patterns[] = {
    // If expressions
    {"if (c) { a(); }", 
     "if (c) { a(); () } else { () }", 
     "Incomplete if must have else branch returning unit"},
    
    {"if (c) { return 1; }", 
     "if (c) { 1 } else { 0 }", 
     "If expression should return values, not use return statements"},
    
    // Function calls
    {"compute_value();", 
     "let _ = compute_value();", 
     "Non-void function results must be used or explicitly discarded"},
    
    {"void_func();", 
     "void_func();", 
     "Void function calls can be used as statements"},
    
    // Assignments with expressions
    {"x = if (c) { 1 } else { 2 }", 
     "x = if (c) { 1 } else { 2 };", 
     "Assignment of if expression is valid"},
    
    // Loops
    {"for i in arr { process(i); }", 
     "for i in arr { process(i); }", 
     "For loops are statements and work as-is"},
    
    {NULL, NULL, NULL} // Sentinel
};

/**
 * Get the expression-oriented form of a statement pattern
 */
static inline const char* get_expression_form(const char* statement) {
    for (int i = 0; expression_patterns[i].statement_form != NULL; i++) {
        if (strcmp(statement, expression_patterns[i].statement_form) == 0) {
            return expression_patterns[i].expression_form;
        }
    }
    return NULL;
}

#endif // EXPRESSION_ORIENTED_TEST_UTILS_H