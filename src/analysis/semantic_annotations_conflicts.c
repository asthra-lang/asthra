/**
 * Asthra Programming Language Compiler
 * Semantic Analysis - Annotations Conflicts Module Implementation
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Implementation of conflict detection and duplicate checking for semantic annotations
 * Split from semantic_annotations.c for better modularity
 */

#include "semantic_annotations_conflicts.h"
#include "semantic_annotations_errors.h"
#include "../parser/ast_node_list.h"
#include <string.h>

// =============================================================================
// CONFLICT DETECTION FUNCTIONS
// =============================================================================

bool check_annotation_conflicts(SemanticAnalyzer *analyzer, ASTNode *node, ASTNodeList *annotations) {
    if (!analyzer || !node || !annotations) {
        return true; // No conflicts if no annotations
    }
    
    size_t annotation_count = ast_node_list_size(annotations);
    if (annotation_count < 2) {
        return true; // Need at least 2 annotations for conflicts
    }
    
    // Get the conflict registry
    const AnnotationConflict *conflicts = get_all_annotation_conflicts();
    
    // Check all pairs of annotations for conflicts
    for (size_t i = 0; i < annotation_count; i++) {
        ASTNode *annotation1 = ast_node_list_get(annotations, i);
        if (!annotation1 || annotation1->type != AST_SEMANTIC_TAG) continue;
        
        const char *name1 = annotation1->data.semantic_tag.name;
        if (!name1) continue;
        
        for (size_t j = i + 1; j < annotation_count; j++) {
            ASTNode *annotation2 = ast_node_list_get(annotations, j);
            if (!annotation2 || annotation2->type != AST_SEMANTIC_TAG) continue;
            
            const char *name2 = annotation2->data.semantic_tag.name;
            if (!name2) continue;
            
            // Check for conflicts in both directions
            for (size_t k = 0; conflicts[k].annotation1 != NULL; k++) {
                const AnnotationConflict *conflict = &conflicts[k];
                
                if ((strcmp(conflict->annotation1, name1) == 0 && strcmp(conflict->annotation2, name2) == 0) ||
                    (strcmp(conflict->annotation1, name2) == 0 && strcmp(conflict->annotation2, name1) == 0)) {
                    
                    // Handle different conflict types
                    switch (conflict->type) {
                        case CONFLICT_MUTUALLY_EXCLUSIVE:
                            report_annotation_conflict_error(analyzer, node, conflict);
                            return false;
                            
                        case CONFLICT_REDUNDANT:
                            issue_annotation_warning(analyzer, node, WARNING_MEDIUM, conflict->resolution_hint);
                            break;
                            
                        case CONFLICT_DEPRECATED_COMBINATION:
                            issue_annotation_warning(analyzer, node, WARNING_HIGH, conflict->resolution_hint);
                            break;
                    }
                }
            }
        }
    }
    
    return true;
}

bool check_duplicate_annotations(SemanticAnalyzer *analyzer, ASTNode *node, ASTNodeList *annotations) {
    if (!analyzer || !node || !annotations) {
        return true; // No duplicates if no annotations
    }
    
    size_t annotation_count = ast_node_list_size(annotations);
    if (annotation_count < 2) {
        return true; // Need at least 2 annotations for duplicates
    }
    
    // Check for duplicate annotation names
    for (size_t i = 0; i < annotation_count; i++) {
        ASTNode *annotation1 = ast_node_list_get(annotations, i);
        if (!annotation1 || annotation1->type != AST_SEMANTIC_TAG) continue;
        
        const char *name1 = annotation1->data.semantic_tag.name;
        if (!name1) continue;
        
        for (size_t j = i + 1; j < annotation_count; j++) {
            ASTNode *annotation2 = ast_node_list_get(annotations, j);
            if (!annotation2 || annotation2->type != AST_SEMANTIC_TAG) continue;
            
            const char *name2 = annotation2->data.semantic_tag.name;
            if (!name2) continue;
            
            if (strcmp(name1, name2) == 0) {
                report_duplicate_annotation_error(analyzer, node, name1);
                return false;
            }
        }
    }
    
    return true;
} 
