#include "ai_annotation_grammar.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// AI annotation mapping implemented in ai_annotation_grammar.c

// is_ai_annotation and resolve_ai_annotation_type implemented in ai_annotation_grammar.c

// Validation functions removed - implemented in ai_annotation_grammar.c

bool ai_annotation_valid_for_function(AIAnnotationType type) {
    switch (type) {
    case AI_ANNOTATION_CONFIDENCE:
    case AI_ANNOTATION_HYPOTHESIS:
    case AI_ANNOTATION_REVIEW_NEEDED:
    case AI_ANNOTATION_TODO:
    case AI_ANNOTATION_OPTIMIZE:
    case AI_ANNOTATION_TEST_COVERAGE:
    case AI_ANNOTATION_SECURITY:
    case AI_ANNOTATION_PATTERN:
    case AI_ANNOTATION_COMPLEXITY:
    case AI_ANNOTATION_REFINEMENT:
        return true;
    default:
        return false;
    }
}

bool ai_annotation_valid_for_statement(AIAnnotationType type) {
    switch (type) {
    case AI_ANNOTATION_CONFIDENCE:
    case AI_ANNOTATION_TODO:
    case AI_ANNOTATION_OPTIMIZE:
    case AI_ANNOTATION_SECURITY:
        return true;
    default:
        return false;
    }
}

bool ai_annotation_valid_for_expression(AIAnnotationType type) {
    switch (type) {
    case AI_ANNOTATION_CONFIDENCE:
    case AI_ANNOTATION_OPTIMIZE:
        return true;
    default:
        return false;
    }
}

bool ai_annotation_valid_for_declaration(AIAnnotationType type) {
    switch (type) {
    case AI_ANNOTATION_CONFIDENCE:
    case AI_ANNOTATION_HYPOTHESIS:
    case AI_ANNOTATION_REVIEW_NEEDED:
    case AI_ANNOTATION_TODO:
    case AI_ANNOTATION_OPTIMIZE:
    case AI_ANNOTATION_TEST_COVERAGE:
    case AI_ANNOTATION_SECURITY:
    case AI_ANNOTATION_PATTERN:
    case AI_ANNOTATION_COMPLEXITY:
        return true;
    default:
        return false;
    }
}
