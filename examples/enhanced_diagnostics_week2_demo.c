#include "../src/compiler/enhanced_error_reporting.h"
#include "../src/diagnostics/suggestion_engine.h"
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    printf("=== Enhanced Diagnostics Week 2 Demo ===\n");
    printf("Demonstrating AI-ready error reporting and intelligent suggestions\n\n");
    
    // Create enhanced error reporter
    EnhancedErrorReporter *reporter = enhanced_error_reporter_create();
    if (!reporter) {
        printf("Failed to create error reporter\n");
        return 1;
    }
    
    printf("1. Testing undefined variable reporting...\n");
    enhanced_error_reporter_report_undefined_variable(reporter, "user_name", 15, 9, "demo.asthra");
    
    printf("2. Testing type mismatch reporting...\n");
    enhanced_error_reporter_report_type_mismatch(reporter, "string", "i32", 20, 15, "demo.asthra");
    
    printf("3. Testing missing visibility reporting (PEG v1.22 compliance)...\n");
    enhanced_error_reporter_report_missing_visibility(reporter, "struct", 10, 1, "demo.asthra");
    
    printf("4. Testing missing parameter list reporting...\n");
    enhanced_error_reporter_report_missing_parameter_list(reporter, "main", 5, 8, "demo.asthra");
    
    printf("5. Testing missing struct content reporting...\n");
    enhanced_error_reporter_report_missing_struct_content(reporter, "Empty", 12, 7, "demo.asthra");
    
    printf("\n=== Human-Readable Output ===\n");
    enhanced_error_reporter_print_human_readable(reporter);
    
    printf("\n=== AI-Consumable JSON Output ===\n");
    char *json = enhanced_error_reporter_to_json(reporter);
    if (json) {
        printf("%s\n", json);
        free(json);
    }
    
    printf("\n=== Statistics ===\n");
    printf("Total diagnostics: %zu\n", reporter->diagnostic_count);
    printf("Error count: %zu\n", enhanced_error_reporter_get_error_count(reporter));
    printf("Has errors: %s\n", enhanced_error_reporter_has_errors(reporter) ? "Yes" : "No");
    
    printf("\n=== Testing Suggestion Engine ===\n");
    
    // Test Levenshtein distance
    printf("Levenshtein distance 'user_name' vs 'user_info': %d\n", 
           levenshtein_distance("user_name", "user_info"));
    
    // Test similarity calculation
    printf("Similarity 'user_name' vs 'user_info': %.2f%%\n", 
           calculate_similarity("user_name", "user_info") * 100);
    
    // Test undefined variable suggestion
    const char *available_vars[] = {"user_info", "user_data", "username", "config"};
    DiagnosticSuggestion *suggestion = generate_undefined_variable_suggestion("user_name", 
                                                                             available_vars, 4);
    if (suggestion) {
        printf("Suggestion for 'user_name': '%s' (confidence: %s)\n", 
               suggestion->text,
               (suggestion->confidence == CONFIDENCE_HIGH) ? "high" :
               (suggestion->confidence == CONFIDENCE_MEDIUM) ? "medium" : "low");
        suggestion_destroy(suggestion);
    }
    
    // Test type mismatch suggestion
    suggestion = generate_type_mismatch_suggestion("string", "i32");
    if (suggestion) {
        printf("Type conversion suggestion i32->string: '%s'\n", suggestion->text);
        suggestion_destroy(suggestion);
    }
    
    // Test grammar compliance suggestions
    suggestion = generate_grammar_compliance_suggestion("missing_visibility", "struct");
    if (suggestion) {
        printf("Grammar compliance suggestion: '%s'\n", suggestion->text);
        suggestion_destroy(suggestion);
    }
    
    enhanced_error_reporter_destroy(reporter);
    
    printf("\n=== Key Features Demonstrated ===\n");
    printf("✅ Enhanced error reporting with structured diagnostics\n");
    printf("✅ Intelligent suggestion generation using Levenshtein distance\n");
    printf("✅ PEG grammar v1.22 compliance checking\n");
    printf("✅ AI-consumable JSON output for automated error resolution\n");
    printf("✅ Confidence-based suggestion ranking\n");
    printf("✅ Type conversion suggestions for common mismatches\n");
    printf("✅ Memory-safe implementation with comprehensive cleanup\n");
    
    printf("\n=== Demo completed successfully! ===\n");
    printf("Week 2 Enhanced Diagnostics implementation is ready for AI integration.\n");
    return 0;
} 
