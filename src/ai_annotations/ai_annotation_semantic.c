#include "ai_annotation_semantic.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Forward declarations for helper functions
static void collect_ai_annotations_recursive(ASTNode *node, AIAnnotationAnalysisResult *result);
static void add_ai_annotation_to_result(AIAnnotationAnalysisResult *result, ASTNode *annotation_node, ASTNode *target);
static void add_validation_error(AIAnnotationAnalysisResult *result, AIAnnotation *annotation, const char *error_msg);

AIAnnotationAnalysisResult *ai_annotation_analyze_program(SemanticAnalyzer *analyzer, ASTNode *program) {
    if (!analyzer || !program) return NULL;
    
    AIAnnotationAnalysisResult *result = malloc(sizeof(AIAnnotationAnalysisResult));
    if (!result) return NULL;
    
    // Initialize result structure
    result->annotations = malloc(sizeof(AIAnnotation) * 64); // Initial capacity
    if (!result->annotations) {
        free(result);
        return NULL;
    }
    
    result->annotation_count = 0;
    result->capacity = 64;
    result->confidence_annotations = 0;
    result->review_needed_count = 0;
    result->todo_count = 0;
    result->security_annotations = 0;
    result->valid_annotations = 0;
    result->invalid_annotations = 0;
    result->validation_errors = NULL;
    result->error_count = 0;
    
    // Traverse AST and collect AI annotations
    collect_ai_annotations_recursive(program, result);
    
    // Validate all collected annotations
    for (size_t i = 0; i < result->annotation_count; i++) {
        AIAnnotation *annotation = &result->annotations[i];
        
        if (ai_annotation_validate_placement(annotation, annotation->target_node)) {
            result->valid_annotations++;
            
            // Update statistics
            switch (annotation->type) {
                case AI_ANNOTATION_CONFIDENCE:
                    result->confidence_annotations++;
                    break;
                case AI_ANNOTATION_REVIEW_NEEDED:
                    result->review_needed_count++;
                    break;
                case AI_ANNOTATION_TODO:
                    result->todo_count++;
                    break;
                case AI_ANNOTATION_SECURITY:
                    result->security_annotations++;
                    break;
                default:
                    break;
            }
        } else {
            result->invalid_annotations++;
            add_validation_error(result, annotation, "Invalid annotation placement");
        }
    }
    
    return result;
}

bool ai_annotation_validate_placement(AIAnnotation *annotation, ASTNode *target) {
    if (!annotation || !target) return false;
    
    switch (target->type) {
        case AST_FUNCTION_DECL:
            return ai_annotation_valid_for_function(annotation->type);
        case AST_LET_STMT:
        case AST_STRUCT_DECL:
        case AST_ENUM_DECL:
            return ai_annotation_valid_for_declaration(annotation->type);
        case AST_IF_STMT:
        case AST_FOR_STMT:
        case AST_RETURN_STMT:
            return ai_annotation_valid_for_statement(annotation->type);
        case AST_CALL_EXPR:
        case AST_BINARY_EXPR:
        case AST_UNARY_EXPR:
            return ai_annotation_valid_for_expression(annotation->type);
        default:
            return false;
    }
}

// Context validation functions are implemented in ai_annotation_basic.c

bool ai_annotation_check_conflicts(AIAnnotation *annotation, AIAnnotation *existing[], size_t existing_count) {
    if (!annotation || !existing) return false;
    
    for (size_t i = 0; i < existing_count; i++) {
        if (!existing[i]) continue;
        
        // Check for conflicting annotation types
        if (annotation->type == existing[i]->type) {
            // Duplicate annotation of same type
            return true;
        }
        
        // Check for specific conflicts
        if ((annotation->type == AI_ANNOTATION_CONFIDENCE && existing[i]->type == AI_ANNOTATION_CONFIDENCE) ||
            (annotation->type == AI_ANNOTATION_HYPOTHESIS && existing[i]->type == AI_ANNOTATION_HYPOTHESIS)) {
            // Only one confidence or hypothesis annotation per target
            return true;
        }
    }
    
    return false;
}

void ai_annotation_extract_metadata(AIAnnotation *annotation, AIAnnotationMetadata *metadata) {
    if (!annotation || !metadata) return;
    
    // Initialize metadata
    metadata->category = NULL;
    metadata->priority = NULL;
    metadata->tags = NULL;
    metadata->tag_count = 0;
    metadata->reasoning = NULL;
    metadata->impact_score = 0.0f;
    
    // Extract metadata based on annotation type
    switch (annotation->type) {
        case AI_ANNOTATION_SECURITY:
            metadata->category = strdup("security");
            metadata->priority = strdup("high");
            metadata->impact_score = 0.9f;
            break;
        case AI_ANNOTATION_OPTIMIZE:
            metadata->category = strdup("performance");
            metadata->priority = strdup("medium");
            metadata->impact_score = 0.6f;
            break;
        case AI_ANNOTATION_TODO:
            metadata->category = strdup("maintainability");
            metadata->priority = strdup("low");
            metadata->impact_score = 0.3f;
            break;
        case AI_ANNOTATION_REVIEW_NEEDED:
            metadata->category = strdup("quality");
            metadata->priority = strdup("high");
            metadata->impact_score = 0.8f;
            break;
        case AI_ANNOTATION_CONFIDENCE:
            metadata->category = strdup("ai_metadata");
            metadata->priority = strdup("info");
            metadata->impact_score = annotation->value.confidence_score;
            break;
        default:
            metadata->category = strdup("general");
            metadata->priority = strdup("medium");
            metadata->impact_score = 0.5f;
            break;
    }
}

// Helper function implementations
static void collect_ai_annotations_recursive(ASTNode *node, AIAnnotationAnalysisResult *result) {
    if (!node || !result) return;
    
    // Check if this node has semantic tags (annotations)
    if (node->type == AST_SEMANTIC_TAG) {
        const char *annotation_name = node->data.semantic_tag.name;
        if (annotation_name && is_ai_annotation(annotation_name)) {
            add_ai_annotation_to_result(result, node, node);
        }
    }
    
    // Check for annotations attached to this node (for nodes that support annotations)
    ASTNodeList *annotations = NULL;
    switch (node->type) {
        case AST_FUNCTION_DECL:
            annotations = node->data.function_decl.annotations;
            break;
        case AST_STRUCT_DECL:
            annotations = node->data.struct_decl.annotations;
            break;
        case AST_ENUM_DECL:
            annotations = node->data.enum_decl.annotations;
            break;
        default:
            break;
    }
    
    if (annotations) {
        for (size_t i = 0; i < annotations->count; i++) {
            ASTNode *annotation_node = annotations->nodes[i];
            if (annotation_node && annotation_node->type == AST_SEMANTIC_TAG) {
                const char *annotation_name = annotation_node->data.semantic_tag.name;
                if (annotation_name && is_ai_annotation(annotation_name)) {
                    add_ai_annotation_to_result(result, annotation_node, node);
                }
            }
        }
    }
    
    // Recursively process child nodes
    switch (node->type) {
        case AST_PROGRAM:
            if (node->data.program.declarations) {
                for (size_t i = 0; i < node->data.program.declarations->count; i++) {
                    collect_ai_annotations_recursive(node->data.program.declarations->nodes[i], result);
                }
            }
            break;
        case AST_FUNCTION_DECL:
            if (node->data.function_decl.body) {
                collect_ai_annotations_recursive(node->data.function_decl.body, result);
            }
            break;
        case AST_BLOCK:
            if (node->data.block.statements) {
                for (size_t i = 0; i < node->data.block.statements->count; i++) {
                    collect_ai_annotations_recursive(node->data.block.statements->nodes[i], result);
                }
            }
            break;
        default:
            // For other node types, traverse children if they exist
            // This is a simplified traversal - in practice, you'd handle each node type
            break;
    }
}

static void add_ai_annotation_to_result(AIAnnotationAnalysisResult *result, ASTNode *annotation_node, ASTNode *target) {
    if (!result || !annotation_node || !target) return;
    
    // Expand capacity if needed
    if (result->annotation_count >= result->capacity) {
        size_t new_capacity = result->capacity * 2;
        AIAnnotation *new_annotations = realloc(result->annotations, sizeof(AIAnnotation) * new_capacity);
        if (!new_annotations) return; // Out of memory
        result->annotations = new_annotations;
        result->capacity = new_capacity;
    }
    
    AIAnnotation *annotation = &result->annotations[result->annotation_count];
    memset(annotation, 0, sizeof(AIAnnotation));
    
    // Extract annotation information
    const char *annotation_name = annotation_node->data.semantic_tag.name;
    annotation->type = resolve_ai_annotation_type(annotation_name);
    annotation->target_node = target;
    annotation->location = annotation_node->location;
    annotation->is_processed = false;
    
    // Extract value based on annotation type and parameters
    if (annotation_node->data.semantic_tag.params && 
        annotation_node->data.semantic_tag.params->count > 0) {
        
        ASTNode *first_param = annotation_node->data.semantic_tag.params->nodes[0];
        
        switch (annotation->type) {
            case AI_ANNOTATION_CONFIDENCE:
                if (first_param->type == AST_STRING_LITERAL) {
                    annotation->value.confidence_score = (float)atof(first_param->data.string_literal.value);
                } else if (first_param->type == AST_FLOAT_LITERAL) {
                    annotation->value.confidence_score = (float)first_param->data.float_literal.value;
                }
                break;
            case AI_ANNOTATION_REFINEMENT:
                if (first_param->type == AST_INTEGER_LITERAL) {
                    annotation->value.numeric_value = (int)first_param->data.integer_literal.value;
                }
                break;
            default:
                // Text-based annotations
                if (first_param->type == AST_STRING_LITERAL) {
                    annotation->value.text_value = strdup(first_param->data.string_literal.value);
                }
                break;
        }
    }
    
    result->annotation_count++;
}

static void add_validation_error(AIAnnotationAnalysisResult *result, AIAnnotation *annotation, const char *error_msg) {
    if (!result || !error_msg) return;
    
    // Expand error array if needed
    char **new_errors = realloc(result->validation_errors, sizeof(char*) * (result->error_count + 1));
    if (!new_errors) return;
    
    result->validation_errors = new_errors;
    result->validation_errors[result->error_count] = strdup(error_msg);
    result->error_count++;
}

void free_ai_annotation_analysis_result(AIAnnotationAnalysisResult *result) {
    if (!result) return;
    
    // Free annotations
    if (result->annotations) {
        for (size_t i = 0; i < result->annotation_count; i++) {
            AIAnnotation *annotation = &result->annotations[i];
            if (annotation->value.text_value) {
                free(annotation->value.text_value);
            }
            if (annotation->value.metadata) {
                // Free metadata if allocated
                AIAnnotationMetadata *metadata = annotation->value.metadata;
                free(metadata->category);
                free(metadata->priority);
                free(metadata->reasoning);
                if (metadata->tags) {
                    for (size_t j = 0; j < metadata->tag_count; j++) {
                        free(metadata->tags[j]);
                    }
                    free(metadata->tags);
                }
                free(metadata);
            }
        }
        free(result->annotations);
    }
    
    // Free validation errors
    if (result->validation_errors) {
        for (size_t i = 0; i < result->error_count; i++) {
            free(result->validation_errors[i]);
        }
        free(result->validation_errors);
    }
    
    free(result);
} 
