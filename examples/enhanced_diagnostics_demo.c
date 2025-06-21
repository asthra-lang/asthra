/**
 * Enhanced Diagnostics Demonstration
 * Shows the AI-ready diagnostic system in action
 */

#include "../src/diagnostics/enhanced_diagnostics.h"
#include <stdio.h>
#include <stdlib.h>

void demo_undefined_variable_error(void) {
    printf("=== Demo: Undefined Variable Error ===\n");
    
    // Create enhanced diagnostic for undefined variable
    EnhancedDiagnostic *diagnostic = enhanced_diagnostic_create(
        "ASTHRA_E001", 
        DIAGNOSTIC_ERROR, 
        "undefined variable 'user_name'"
    );
    
    // Add source location span
    DiagnosticSpan span = {
        .start_line = 15, .start_column = 9,
        .end_line = 15, .end_column = 17,
        .file_path = "src/main.asthra",
        .label = "undefined variable",
        .snippet = "    let email = user_name.email;"
    };
    enhanced_diagnostic_add_span(diagnostic, &span);
    
    // Add AI-actionable suggestion
    DiagnosticSuggestion suggestion = {
        .type = SUGGESTION_REPLACE,
        .span = {
            .start_line = 15, .start_column = 9,
            .end_line = 15, .end_column = 17,
            .file_path = "src/main.asthra",
            .label = "replace with",
            .snippet = NULL
        },
        .text = "user_info",
        .confidence = CONFIDENCE_HIGH,
        .rationale = "Similar variable 'user_info' found in scope with 89% similarity"
    };
    enhanced_diagnostic_add_suggestion(diagnostic, &suggestion);
    
    // Add metadata for AI context
    char *similar_symbols[] = {"user_info", "user_data", "username"};
    DiagnosticMetadata metadata = {
        .inferred_types = NULL,
        .type_count = 0,
        .available_methods = NULL,
        .method_count = 0,
        .similar_symbols = similar_symbols,
        .symbol_count = 3,
        .error_category = "semantic"
    };
    enhanced_diagnostic_set_metadata(diagnostic, &metadata);
    
    // Generate AI-consumable JSON
    char *json_output = enhanced_diagnostic_to_json(diagnostic);
    printf("AI-Consumable JSON Output:\n%s\n\n", json_output);
    
    free(json_output);
    enhanced_diagnostic_destroy(diagnostic);
}

void demo_type_mismatch_error(void) {
    printf("=== Demo: Type Mismatch Error ===\n");
    
    // Create enhanced diagnostic for type mismatch
    EnhancedDiagnostic *diagnostic = enhanced_diagnostic_create(
        "ASTHRA_E002", 
        DIAGNOSTIC_ERROR, 
        "type mismatch: expected 'i32', found 'string'"
    );
    
    // Add source location span
    DiagnosticSpan span = {
        .start_line = 23, .start_column = 14,
        .end_line = 23, .end_column = 21,
        .file_path = "src/calculator.asthra",
        .label = "expected i32",
        .snippet = "    let result: i32 = \"hello\";"
    };
    enhanced_diagnostic_add_span(diagnostic, &span);
    
    // Add type conversion suggestion
    DiagnosticSuggestion suggestion = {
        .type = SUGGESTION_REPLACE,
        .span = {
            .start_line = 23, .start_column = 14,
            .end_line = 23, .end_column = 21,
            .file_path = "src/calculator.asthra",
            .label = "convert to i32",
            .snippet = NULL
        },
        .text = "42",
        .confidence = CONFIDENCE_MEDIUM,
        .rationale = "Replace string literal with integer literal"
    };
    enhanced_diagnostic_add_suggestion(diagnostic, &suggestion);
    
    // Add metadata with type information
    char *inferred_types[] = {"string", "i32"};
    DiagnosticMetadata metadata = {
        .inferred_types = inferred_types,
        .type_count = 2,
        .available_methods = NULL,
        .method_count = 0,
        .similar_symbols = NULL,
        .symbol_count = 0,
        .error_category = "type_system"
    };
    enhanced_diagnostic_set_metadata(diagnostic, &metadata);
    
    // Generate AI-consumable JSON
    char *json_output = enhanced_diagnostic_to_json(diagnostic);
    printf("AI-Consumable JSON Output:\n%s\n\n", json_output);
    
    free(json_output);
    enhanced_diagnostic_destroy(diagnostic);
}

void demo_multiple_suggestions(void) {
    printf("=== Demo: Multiple AI Suggestions ===\n");
    
    // Create enhanced diagnostic with multiple suggestions
    EnhancedDiagnostic *diagnostic = enhanced_diagnostic_create(
        "ASTHRA_W001", 
        DIAGNOSTIC_WARNING, 
        "unused variable 'temp_data'"
    );
    
    // Add source location span
    DiagnosticSpan span = {
        .start_line = 8, .start_column = 9,
        .end_line = 8, .end_column = 18,
        .file_path = "src/processor.asthra",
        .label = "unused variable",
        .snippet = "    let temp_data = fetch_data();"
    };
    enhanced_diagnostic_add_span(diagnostic, &span);
    
    // Suggestion 1: Remove unused variable
    DiagnosticSuggestion suggestion1 = {
        .type = SUGGESTION_DELETE,
        .span = {
            .start_line = 8, .start_column = 5,
            .end_line = 8, .end_column = 31,
            .file_path = "src/processor.asthra",
            .label = "remove line",
            .snippet = NULL
        },
        .text = "",
        .confidence = CONFIDENCE_HIGH,
        .rationale = "Variable is never used, safe to remove"
    };
    enhanced_diagnostic_add_suggestion(diagnostic, &suggestion1);
    
    // Suggestion 2: Use the variable
    DiagnosticSuggestion suggestion2 = {
        .type = SUGGESTION_INSERT,
        .span = {
            .start_line = 9, .start_column = 1,
            .end_line = 9, .end_column = 1,
            .file_path = "src/processor.asthra",
            .label = "use variable",
            .snippet = NULL
        },
        .text = "    process(temp_data);",
        .confidence = CONFIDENCE_MEDIUM,
        .rationale = "Add usage of the variable to avoid waste"
    };
    enhanced_diagnostic_add_suggestion(diagnostic, &suggestion2);
    
    // Generate AI-consumable JSON
    char *json_output = enhanced_diagnostic_to_json(diagnostic);
    printf("AI-Consumable JSON Output:\n%s\n\n", json_output);
    
    free(json_output);
    enhanced_diagnostic_destroy(diagnostic);
}

int main(void) {
    printf("🚀 Asthra Enhanced Diagnostics - AI Integration Demo\n");
    printf("===================================================\n\n");
    
    printf("This demonstration shows how Asthra's enhanced diagnostics\n");
    printf("provide AI tools with structured, actionable feedback.\n\n");
    
    demo_undefined_variable_error();
    demo_type_mismatch_error();
    demo_multiple_suggestions();
    
    printf("🎯 Key Benefits for AI Tools:\n");
    printf("• Structured error codes for pattern recognition\n");
    printf("• Confidence levels for suggestion quality assessment\n");
    printf("• Rich metadata for context-aware decision making\n");
    printf("• Machine-parsable JSON format for easy integration\n");
    printf("• Multiple suggestions ranked by confidence\n\n");
    
    printf("✅ Enhanced Diagnostics Phase 1 Week 1: COMPLETE\n");
    printf("Ready for AI integration and automatic error resolution!\n");
    
    return 0;
} 
