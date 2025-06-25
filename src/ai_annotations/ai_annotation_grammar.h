#ifndef ASTHRA_AI_ANNOTATION_GRAMMAR_H
#define ASTHRA_AI_ANNOTATION_GRAMMAR_H

#include "../analysis/semantic_annotations.h"
#include "../parser/ast.h"
#include "../parser/grammar_annotations.h"
#include "../parser/parser.h"

#ifdef __cplusplus
extern "C" {
#endif

// AI-specific annotation types (extending existing annotation system)
typedef enum {
    AI_ANNOTATION_CONFIDENCE,    // #[ai_confidence(0.8)]
    AI_ANNOTATION_HYPOTHESIS,    // #[ai_hypothesis("reasoning")]
    AI_ANNOTATION_REVIEW_NEEDED, // #[ai_review_needed("area")]
    AI_ANNOTATION_TODO,          // #[ai_todo("task")]
    AI_ANNOTATION_OPTIMIZE,      // #[ai_optimize("suggestion")]
    AI_ANNOTATION_TEST_COVERAGE, // #[ai_test_coverage("level")]
    AI_ANNOTATION_SECURITY,      // #[ai_security_review("priority")]
    AI_ANNOTATION_PATTERN,       // #[ai_pattern("pattern_name")]
    AI_ANNOTATION_COMPLEXITY,    // #[ai_complexity("level")]
    AI_ANNOTATION_REFINEMENT     // #[ai_refinement_step(3)]
} AIAnnotationType;

// AI annotation parameter structure (extends existing system)
typedef struct {
    AnnotationParamType type;
    char *name; // Parameter name
    union {
        char *string_value; // For string parameters
        char *ident_value;  // For identifier parameters
        int int_value;      // For integer parameters
        bool bool_value;    // For boolean parameters
        float float_value;  // For float parameters (confidence scores)
    } value;
} AIAnnotationParam;

// AI annotation metadata for complex annotations
typedef struct {
    char *category; // "security", "performance", "maintainability"
    char *priority; // "low", "medium", "high", "critical"
    char **tags;    // Additional classification tags
    size_t tag_count;
    char *reasoning;    // AI reasoning for this annotation
    float impact_score; // Estimated impact (0.0-1.0)
} AIAnnotationMetadata;

// AI annotation structure (extends existing annotation system)
typedef struct {
    AIAnnotationType type;
    union {
        float confidence_score;         // For confidence annotations
        char *text_value;               // For text-based annotations
        int numeric_value;              // For numeric annotations
        AIAnnotationMetadata *metadata; // For complex annotations
    } value;
    SourceLocation location;
    ASTNode *target_node;
    bool is_processed; // Compile-time processing flag
} AIAnnotation;

// AI annotation parsing result
typedef struct {
    char *name;                // Annotation name (e.g., "ai_confidence")
    AIAnnotationType type;     // Parsed annotation type
    AIAnnotationParam *params; // Parameters using our extended structure
    size_t param_count;
    bool is_valid;       // Validation result
    char *error_message; // Error if invalid
} AIAnnotationParseResult;

// Parser extensions for AI annotations (building on existing system)
AIAnnotationParseResult *parse_ai_annotation(Parser *parser, const char *annotation_name);
bool validate_ai_annotation_syntax(const char *annotation_name, AIAnnotationParam *params,
                                   size_t param_count);
AIAnnotationType resolve_ai_annotation_type(const char *annotation_name);
bool is_ai_annotation(const char *annotation_name);

// Grammar validation functions
bool validate_ai_confidence_annotation(AIAnnotationParam *params, size_t param_count);
bool validate_ai_hypothesis_annotation(AIAnnotationParam *params, size_t param_count);
bool validate_ai_review_annotation(AIAnnotationParam *params, size_t param_count);
bool validate_ai_todo_annotation(AIAnnotationParam *params, size_t param_count);
bool validate_ai_optimize_annotation(AIAnnotationParam *params, size_t param_count);
bool validate_ai_test_coverage_annotation(AIAnnotationParam *params, size_t param_count);
bool validate_ai_security_annotation(AIAnnotationParam *params, size_t param_count);
bool validate_ai_pattern_annotation(AIAnnotationParam *params, size_t param_count);
bool validate_ai_complexity_annotation(AIAnnotationParam *params, size_t param_count);
bool validate_ai_refinement_annotation(AIAnnotationParam *params, size_t param_count);

// Integration with existing annotation system
ASTNode *parse_ai_semantic_annotation(Parser *parser);
bool is_ai_semantic_annotation(const char *annotation_name);

// Context validation functions (from ai_annotation_basic.c)
bool ai_annotation_valid_for_function(AIAnnotationType type);
bool ai_annotation_valid_for_statement(AIAnnotationType type);
bool ai_annotation_valid_for_expression(AIAnnotationType type);
bool ai_annotation_valid_for_declaration(AIAnnotationType type);

// Parameter parsing helpers
AIAnnotationParam *parse_ai_annotation_parameters(Parser *parser, size_t *param_count);
void free_ai_annotation_params(AIAnnotationParam *params, size_t count);
void free_ai_annotation_parse_result(AIAnnotationParseResult *result);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_AI_ANNOTATION_GRAMMAR_H
