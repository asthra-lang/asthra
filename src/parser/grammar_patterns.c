/**
 * Asthra Programming Language Compiler - Pattern Grammar Productions Implementation
 * Pattern matching for match statements and destructuring
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "grammar_patterns.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// =============================================================================
// PATTERN PARSING
// =============================================================================

ASTNode *parse_pattern(Parser *parser) {
    if (!parser) return NULL;
    
    SourceLocation start_loc = parser->current_token.location;
    
    #ifdef DEBUG_PARSER
    fprintf(stderr, "parse_pattern: current token type=%d\n", parser->current_token.type);
    if (parser->current_token.type == TOKEN_IDENTIFIER) {
        fprintf(stderr, "parse_pattern: identifier='%s'\n", parser->current_token.data.identifier.name);
    }
    #endif
    
    // Handle 'mut' modifier for mutable bindings in patterns
    bool is_mutable = false;
    if (match_token(parser, TOKEN_MUT)) {
        is_mutable = true;
        advance_token(parser);
        // Update start location after consuming 'mut'
        start_loc = parser->current_token.location;
    }
    
    // Handle enum patterns like Result.Ok(value) and Option.Some(value) and struct patterns like Point { x, y }
    // Also handle Result and Option keywords for enum patterns
    if (match_token(parser, TOKEN_IDENTIFIER) || match_token(parser, TOKEN_RESULT) || match_token(parser, TOKEN_OPTION)) {
        char *name;
        if (parser->current_token.type == TOKEN_RESULT) {
            name = strdup("Result");
        } else if (parser->current_token.type == TOKEN_OPTION) {
            name = strdup("Option");
        } else {
            name = strdup(parser->current_token.data.identifier.name);
        }
        
        advance_token(parser);
        
        if (match_token(parser, TOKEN_DOT)) {
            // This is an enum pattern: EnumName.Variant(binding)
            advance_token(parser);
            
            char *variant_name = NULL;
            if (match_token(parser, TOKEN_IDENTIFIER)) {
                variant_name = strdup(parser->current_token.data.identifier.name);
                advance_token(parser);
            } else if (match_token(parser, TOKEN_NONE)) {
                // Allow "none" as a variant name even though it's a reserved keyword
                variant_name = strdup("none");
                advance_token(parser);
            } else {
                report_error(parser, "Expected variant name after '.'");
                free(name);
                return NULL;
            }
            
            char *binding = NULL;
            if (match_token(parser, TOKEN_LEFT_PAREN)) {
                advance_token(parser);
                
                // Parse according to current grammar: PatternArgs <- Pattern (',' Pattern)* / 'none'
                if (match_token(parser, TOKEN_NONE)) {
                                          // Explicit none for empty pattern arguments (current grammar semantic clarity)
                    advance_token(parser);
                    binding = NULL; // No binding
                } else if (match_token(parser, TOKEN_VOID)) {
                    // Legacy compatibility error with helpful message
                    report_error(parser, "Unexpected 'void' in pattern arguments. Use 'none' for empty pattern arguments per current grammar");
                    free(name);
                    free(variant_name);
                    return NULL;
                } else if (match_token(parser, TOKEN_IDENTIFIER)) {
                    // Parse actual pattern binding
                    binding = strdup(parser->current_token.data.identifier.name);
                    advance_token(parser);
                }
                
                if (!expect_token(parser, TOKEN_RIGHT_PAREN)) {
                    free(name);
                    free(variant_name);
                    if (binding) free(binding);
                    return NULL;
                }
            }
            
            ASTNode *node = ast_create_node(AST_ENUM_PATTERN, start_loc);
            if (!node) {
                free(name);
                free(variant_name);
                if (binding) free(binding);
                return NULL;
            }
            
            node->data.enum_pattern.enum_name = name;
            node->data.enum_pattern.variant_name = variant_name;
            node->data.enum_pattern.binding = binding;
            
            return node;
        } else if (match_token(parser, TOKEN_LESS_THAN) || match_token(parser, TOKEN_LEFT_BRACE)) {
            // This is a struct pattern: StructName<T> { field1, field2 } or StructName { field1, field2 }
            
            // Parse optional type arguments: TypeArgs? <- '<' Type (',' Type)* '>'
            ASTNodeList *type_args = NULL;
            if (match_token(parser, TOKEN_LESS_THAN)) {
                advance_token(parser);
                
                type_args = ast_node_list_create(2);
                if (!type_args) {
                    free(name);
                    return NULL;
                }
                
                do {
                    ASTNode *type_arg = parse_type(parser);
                    if (!type_arg) {
                        ast_node_list_destroy(type_args);
                        free(name);
                        return NULL;
                    }
                    
                    ast_node_list_add(&type_args, type_arg);
                    
                    if (match_token(parser, TOKEN_COMMA)) {
                        advance_token(parser);
                    } else {
                        break;
                    }
                } while (!at_end(parser));
                
                if (!expect_token(parser, TOKEN_GREATER_THAN)) {
                    ast_node_list_destroy(type_args);
                    free(name);
                    return NULL;
                }
            }
            
            // Now expect the left brace for the struct pattern
            if (!match_token(parser, TOKEN_LEFT_BRACE)) {
                // If we have type args but no brace, this is a type usage, not a pattern
                if (type_args) ast_node_list_destroy(type_args);
                // This is just an identifier pattern
                ASTNode *node = ast_create_node(AST_IDENTIFIER, start_loc);
                if (!node) {
                    free(name);
                    return NULL;
                }
                
                node->data.identifier.name = name;
                return node;
            }
            
            ASTNode *struct_pattern = parse_struct_pattern(parser);
            if (!struct_pattern) {
                if (type_args) ast_node_list_destroy(type_args);
                free(name);
                return NULL;
            }
            
            // Set the struct name and type arguments
            struct_pattern->data.struct_pattern.struct_name = name;
            struct_pattern->data.struct_pattern.type_args = type_args;
            return struct_pattern;
        } else if (match_token(parser, TOKEN_LEFT_PAREN)) {
            // This is an enum variant pattern without enum name: Variant(binding)
            // In match patterns, we allow Some(x) instead of Option.Some(x)
            advance_token(parser);
            
            char *binding = NULL;
            if (match_token(parser, TOKEN_NONE)) {
                // Explicit none for empty pattern arguments
                advance_token(parser);
                binding = NULL;
            } else if (match_token(parser, TOKEN_IDENTIFIER)) {
                // Parse pattern binding
                binding = strdup(parser->current_token.data.identifier.name);
                advance_token(parser);
            }
            
            if (!expect_token(parser, TOKEN_RIGHT_PAREN)) {
                free(name);
                if (binding) free(binding);
                return NULL;
            }
            
            // Create enum pattern node without enum name
            ASTNode *node = ast_create_node(AST_ENUM_PATTERN, start_loc);
            if (!node) {
                free(name);
                if (binding) free(binding);
                return NULL;
            }
            
            node->data.enum_pattern.enum_name = NULL;  // No enum name in pattern
            node->data.enum_pattern.variant_name = name;
            node->data.enum_pattern.binding = binding;
            
            return node;
        } else {
            // Simple identifier pattern
            ASTNode *node = ast_create_node(AST_IDENTIFIER, start_loc);
            if (!node) {
                free(name);
                return NULL;
            }
            
            node->data.identifier.name = name;
            // Note: For patterns, mutability is stored differently than regular identifiers
            // This might need to be handled at a higher level in the AST
            // For now, we'll just parse it correctly
            return node;
        }
    }
    
    // Note: According to grammar line 105, StructPattern requires SimpleIdent
    // StructPattern <- SimpleIdent TypeArgs? '{' FieldPattern (',' FieldPattern)* '}'
    // So we don't allow anonymous struct patterns like { x, y } here
    
    // Handle tuple patterns: (pattern1, pattern2, ...)
    if (match_token(parser, TOKEN_LEFT_PAREN)) {
        advance_token(parser);
        
        // Check for unit pattern ()
        if (match_token(parser, TOKEN_RIGHT_PAREN)) {
            advance_token(parser);
            // Unit pattern - treat as wildcard or special case
            ASTNode *node = ast_create_node(AST_WILDCARD_PATTERN, start_loc);
            if (!node) return NULL;
            return node;
        }
        
        // Parse tuple pattern
        ASTNodeList *patterns = ast_node_list_create(2);
        if (!patterns) return NULL;
        
        // Parse first pattern
        ASTNode *first_pattern = parse_pattern(parser);
        if (!first_pattern) {
            ast_node_list_destroy(patterns);
            return NULL;
        }
        
        // Check for comma - if no comma, it's just a parenthesized pattern
        if (!match_token(parser, TOKEN_COMMA)) {
            // Just a parenthesized pattern
            if (expect_token(parser, TOKEN_RIGHT_PAREN)) {
                ast_node_list_destroy(patterns);
                return first_pattern;
            } else {
                ast_free_node(first_pattern);
                ast_node_list_destroy(patterns);
                return NULL;
            }
        }
        
        // We have a comma, so this is a tuple pattern
        ast_node_list_add(&patterns, first_pattern);
        advance_token(parser); // consume comma
        
        // Parse remaining patterns
        do {
            ASTNode *pattern = parse_pattern(parser);
            if (!pattern) {
                // Clean up
                for (size_t i = 0; i < patterns->count; i++) {
                    ast_free_node(patterns->nodes[i]);
                }
                ast_node_list_destroy(patterns);
                return NULL;
            }
            
            ast_node_list_add(&patterns, pattern);
            
            if (match_token(parser, TOKEN_COMMA)) {
                advance_token(parser);
            } else {
                break;
            }
        } while (!at_end(parser) && !match_token(parser, TOKEN_RIGHT_PAREN));
        
        // Expect closing paren
        if (!expect_token(parser, TOKEN_RIGHT_PAREN)) {
            // Clean up
            for (size_t i = 0; i < patterns->count; i++) {
                ast_free_node(patterns->nodes[i]);
            }
            ast_node_list_destroy(patterns);
            return NULL;
        }
        
        // Create tuple pattern node
        ASTNode *node = ast_create_node(AST_TUPLE_PATTERN, start_loc);
        if (!node) {
            // Clean up
            for (size_t i = 0; i < patterns->count; i++) {
                ast_free_node(patterns->nodes[i]);
            }
            ast_node_list_destroy(patterns);
            return NULL;
        }
        
        node->data.tuple_pattern.patterns = patterns;
        return node;
    }
    
    // Handle underscore wildcard pattern
    if (match_token(parser, TOKEN_IDENTIFIER) && 
        strcmp(parser->current_token.data.identifier.name, "_") == 0) {
        advance_token(parser);
        
        ASTNode *node = ast_create_node(AST_WILDCARD_PATTERN, start_loc);
        if (!node) return NULL;
        
        return node;
    }
    
    // Handle literals and other patterns
    if (match_token(parser, TOKEN_INTEGER)) {
        int64_t value = parser->current_token.data.integer.value;
        advance_token(parser);
        
        ASTNode *node = ast_create_node(AST_INTEGER_LITERAL, start_loc);
        if (!node) return NULL;
        
        node->data.integer_literal.value = value;
        return node;
    }
    
    if (match_token(parser, TOKEN_STRING)) {
        char *value = strdup(parser->current_token.data.string.value);
        advance_token(parser);
        
        ASTNode *node = ast_create_node(AST_STRING_LITERAL, start_loc);
        if (!node) {
            free(value);
            return NULL;
        }
        
        node->data.string_literal.value = value;
        return node;
    }
    
    if (match_token(parser, TOKEN_BOOL_TRUE)) {
        advance_token(parser);
        
        ASTNode *node = ast_create_node(AST_BOOL_LITERAL, start_loc);
        if (!node) return NULL;
        
        node->data.bool_literal.value = true;
        return node;
    }
    
    if (match_token(parser, TOKEN_BOOL_FALSE)) {
        advance_token(parser);
        
        ASTNode *node = ast_create_node(AST_BOOL_LITERAL, start_loc);
        if (!node) return NULL;
        
        node->data.bool_literal.value = false;
        return node;
    }
    
    report_error(parser, "Expected pattern");
    return NULL;
}

ASTNode *parse_enum_pattern(Parser *parser) {
    // This is handled within parse_pattern
    return parse_pattern(parser);
}

ASTNode *parse_struct_pattern(Parser *parser) {
    if (!parser) return NULL;
    
    SourceLocation start_loc = parser->current_token.location;
    
    if (!expect_token(parser, TOKEN_LEFT_BRACE)) {
        return NULL;
    }
    
    ASTNode **field_patterns = NULL;
    size_t field_count = 0;
    size_t field_capacity = 4;
    
    field_patterns = malloc(field_capacity * sizeof(ASTNode*));
    if (!field_patterns) return NULL;
    
    bool has_rest_pattern = false;
    
    while (!match_token(parser, TOKEN_RIGHT_BRACE) && !at_end(parser)) {
        // Check for ".." pattern to ignore remaining fields
        if (match_token(parser, TOKEN_ELLIPSIS)) {
            advance_token(parser); // consume "..."
            has_rest_pattern = true;
            
            // Note: TOKEN_ELLIPSIS is "..." but we use it for ".." in patterns
            // This is a lexer limitation we're working around
            
            // ".." must be the last element
            if (!match_token(parser, TOKEN_RIGHT_BRACE)) {
                report_error(parser, "'..' must be the last element in struct pattern");
                for (size_t i = 0; i < field_count; i++) {
                    ast_free_node(field_patterns[i]);
                }
                free(field_patterns);
                return NULL;
            }
            break;
        }
        
        ASTNode *field_pattern = parse_field_pattern(parser);
        if (!field_pattern) {
            for (size_t i = 0; i < field_count; i++) {
                ast_free_node(field_patterns[i]);
            }
            free(field_patterns);
            return NULL;
        }
        
        if (field_count >= field_capacity) {
            field_capacity *= 2;
            field_patterns = realloc(field_patterns, field_capacity * sizeof(ASTNode*));
            if (!field_patterns) {
                ast_free_node(field_pattern);
                return NULL;
            }
        }
        
        field_patterns[field_count++] = field_pattern;
        
        // Parse comma between field patterns (v1.12: no trailing comma allowed)
        // After parsing a field pattern (which might contain nested patterns),
        // we need to check what the next token is to decide how to continue
        if (match_token(parser, TOKEN_COMMA)) {
            advance_token(parser);
            // After comma, we must have another field pattern or ".." (no trailing comma)
            if (match_token(parser, TOKEN_RIGHT_BRACE)) {
                report_error(parser, "Trailing comma not allowed in struct patterns");
                for (size_t i = 0; i < field_count; i++) {
                    ast_free_node(field_patterns[i]);
                }
                free(field_patterns);
                return NULL;
            }
            // Continue the loop to parse the next field pattern
        } else if (match_token(parser, TOKEN_RIGHT_BRACE)) {
            // Found closing brace - end of struct pattern
            break;
        } else {
            // Neither comma nor closing brace - this is an error
            report_error(parser, "Expected ',' between field patterns or '}' after last field pattern");
            for (size_t i = 0; i < field_count; i++) {
                ast_free_node(field_patterns[i]);
            }
            free(field_patterns);
            return NULL;
        }
    }
    
    if (!expect_token(parser, TOKEN_RIGHT_BRACE)) {
        for (size_t i = 0; i < field_count; i++) {
            ast_free_node(field_patterns[i]);
        }
        free(field_patterns);
        return NULL;
    }
    
    ASTNode *node = ast_create_node(AST_STRUCT_PATTERN, start_loc);
    if (!node) {
        for (size_t i = 0; i < field_count; i++) {
            ast_free_node(field_patterns[i]);
        }
        free(field_patterns);
        return NULL;
    }
    
    node->data.struct_pattern.struct_name = NULL; // Will be filled in by parent
    node->data.struct_pattern.type_args = NULL;   // Will be filled in by parent for generic structs
    node->data.struct_pattern.is_partial = has_rest_pattern; // Store whether we have ".."
    
    // Convert field_patterns array to ASTNodeList
    if (field_patterns && field_count > 0) {
        node->data.struct_pattern.field_patterns = ast_node_list_create(field_count);
        if (node->data.struct_pattern.field_patterns) {
            for (size_t i = 0; i < field_count; i++) {
                ast_node_list_add(&node->data.struct_pattern.field_patterns, field_patterns[i]);
            }
        }
        free(field_patterns);
    } else {
        node->data.struct_pattern.field_patterns = NULL;
    }
    
    return node;
}

ASTNode *parse_field_pattern(Parser *parser) {
    if (!parser) return NULL;
    
    SourceLocation start_loc = parser->current_token.location;
    
    if (!match_token(parser, TOKEN_IDENTIFIER)) {
        report_error(parser, "Expected field name in pattern");
        return NULL;
    }
    
    char *field_name = strdup(parser->current_token.data.identifier.name);
    advance_token(parser);
    
    // Phase 2: Enforce explicit field binding syntax - colon is now mandatory
    if (!expect_token(parser, TOKEN_COLON)) {
        report_error(parser, "Expected ':' after field name in pattern. Use explicit syntax like 'field: variable' or 'field: _'");
        free(field_name);
        return NULL;
    }
    
    char *binding_name = NULL;
    bool is_ignored = false;
    ASTNode *nested_pattern = NULL;
    
    // After ':', we can have:
    // 1. Simple binding: "field: var"
    // 2. Ignored binding: "field: _"
    // 3. Nested pattern: "field: Point { x, y }" or "field: Some(value)"
    
    // Handle underscore explicitly for ignored bindings
    if (match_token(parser, TOKEN_IDENTIFIER) && 
        strcmp(parser->current_token.data.identifier.name, "_") == 0) {
        // This is an ignored binding: "field: _"
        advance_token(parser);
        is_ignored = true;
    } else if (match_token(parser, TOKEN_IDENTIFIER)) {
        // Save the identifier in case we need to backtrack
        char *potential_binding = strdup(parser->current_token.data.identifier.name);
        SourceLocation id_loc = parser->current_token.location;
        advance_token(parser);
        
        // Check if this might be the start of a nested pattern
        if (match_token(parser, TOKEN_DOT) || match_token(parser, TOKEN_LEFT_BRACE) || 
            match_token(parser, TOKEN_LESS_THAN)) {
            // This is a nested pattern, not a simple binding
            // We need to parse the full pattern starting from the identifier
            
            // For enum patterns like Some(x), we already consumed the identifier
            // so we need to handle the rest manually
            if (match_token(parser, TOKEN_DOT)) {
                // Enum pattern: EnumName.Variant
                advance_token(parser); // consume '.'
                
                if (!match_token(parser, TOKEN_IDENTIFIER)) {
                    report_error(parser, "Expected variant name after '.'");
                    free(field_name);
                    free(potential_binding);
                    return NULL;
                }
                
                char *variant_name = strdup(parser->current_token.data.identifier.name);
                advance_token(parser);
                
                // Create enum pattern node
                nested_pattern = ast_create_node(AST_ENUM_PATTERN, id_loc);
                if (!nested_pattern) {
                    free(field_name);
                    free(potential_binding);
                    free(variant_name);
                    return NULL;
                }
                
                nested_pattern->data.enum_pattern.enum_name = potential_binding;
                nested_pattern->data.enum_pattern.variant_name = variant_name;
                nested_pattern->data.enum_pattern.binding = NULL;
                
                // Check for enum constructor arguments
                if (match_token(parser, TOKEN_LEFT_PAREN)) {
                    advance_token(parser);
                    
                    if (match_token(parser, TOKEN_IDENTIFIER)) {
                        nested_pattern->data.enum_pattern.binding = strdup(parser->current_token.data.identifier.name);
                        advance_token(parser);
                    }
                    
                    if (!expect_token(parser, TOKEN_RIGHT_PAREN)) {
                        ast_free_node(nested_pattern);
                        free(field_name);
                        return NULL;
                    }
                }
            } else if (match_token(parser, TOKEN_LEFT_BRACE)) {
                // Struct pattern: StructName { ... }
                // We need to parse the full struct pattern
                ASTNode *struct_pattern = parse_struct_pattern(parser);
                if (!struct_pattern) {
                    free(field_name);
                    free(potential_binding);
                    return NULL;
                }
                
                // Set the struct name that we already consumed
                struct_pattern->data.struct_pattern.struct_name = potential_binding;
                nested_pattern = struct_pattern;
            } else {
                // For other complex patterns or simple binding
                binding_name = potential_binding;
            }
        } else {
            // It's a simple binding
            if (strcmp(potential_binding, "_") == 0) {
                is_ignored = true;
                free(potential_binding);
            } else {
                binding_name = potential_binding;
            }
        }
    } else if (match_token(parser, TOKEN_LEFT_BRACE)) {
        // According to grammar line 105, StructPattern requires SimpleIdent before '{'
        // StructPattern <- SimpleIdent TypeArgs? '{' FieldPattern (',' FieldPattern)* '}'
        // So patterns like "field: { x, y }" without struct name should be rejected
        report_error(parser, "Expected struct name before '{' in nested pattern. Use syntax like 'field: StructName { ... }'");
        free(field_name);
        return NULL;
    } else {
        // Try parsing any other pattern type
        nested_pattern = parse_pattern(parser);
        if (!nested_pattern) {
            report_error(parser, "Expected binding name, '_', or nested pattern after ':' in field pattern");
            free(field_name);
            return NULL;
        }
    }
    
    ASTNode *node = ast_create_node(AST_FIELD_PATTERN, start_loc);
    if (!node) {
        free(field_name);
        if (binding_name) free(binding_name);
        if (nested_pattern) ast_free_node(nested_pattern);
        return NULL;
    }
    
    node->data.field_pattern.field_name = field_name;
    node->data.field_pattern.binding_name = binding_name;
    node->data.field_pattern.is_ignored = is_ignored;
    node->data.field_pattern.pattern = nested_pattern;
    
    return node;
} 
