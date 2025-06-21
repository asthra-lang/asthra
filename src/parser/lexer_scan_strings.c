/**
 * Asthra Programming Language Compiler
 * String and multi-line string scanning implementation for Asthra grammar
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "lexer_scan_strings.h"
#include "lexer_scan_escape.h"
#include "lexer_internal.h"
#include <stdlib.h>
#include <string.h>

bool is_multiline_string_start(Lexer *lexer) {
    // Check for raw multi-line string: r"""
    if (lexer->position + 3 < lexer->source_length &&
        peek_char(lexer, 0) == 'r' &&
        peek_char(lexer, 1) == '"' &&
        peek_char(lexer, 2) == '"' &&
        peek_char(lexer, 3) == '"') {
        return true;
    }
    
    // Check for processed multi-line string: """
    if (lexer->position + 2 < lexer->source_length &&
        peek_char(lexer, 0) == '"' &&
        peek_char(lexer, 1) == '"' &&
        peek_char(lexer, 2) == '"') {
        return true;
    }
    
    return false;
}

Token scan_multiline_raw_string(Lexer *lexer) {
    SourceLocation start_loc = current_location(lexer);
    
    // Consume opening r"""
    advance_char(lexer); // r
    advance_char(lexer); // "
    advance_char(lexer); // "
    advance_char(lexer); // "
    
    // Start with reasonable buffer
    char *value = malloc(LEXER_SCAN_INITIAL_MULTILINE_BUFFER_SIZE);
    size_t value_capacity = LEXER_SCAN_INITIAL_MULTILINE_BUFFER_SIZE;
    size_t value_length = 0;
    
    bool found_closing = false;
    
    // Scan until closing """
    while (lexer->position < lexer->source_length) {
        // Check for closing delimiter
        if (lexer->position + 2 < lexer->source_length &&
            peek_char(lexer, 0) == '"' &&
            peek_char(lexer, 1) == '"' &&
            peek_char(lexer, 2) == '"') {
            // Found closing delimiter
            advance_char(lexer); // "
            advance_char(lexer); // "
            advance_char(lexer); // "
            found_closing = true;
            break;
        }
        
        // Resize buffer if needed
        if (value_length + 1 >= value_capacity) {
            value_capacity *= 2;
            value = realloc(value, value_capacity);
            if (!value) {
                set_error(lexer, "Memory allocation failed");
                return token_create(TOKEN_ERROR, start_loc);
            }
        }
        
        // Add character to buffer (no escape processing for raw strings)
        value[value_length++] = advance_char(lexer);
    }
    
    // Check if we found the closing delimiter
    if (!found_closing) {
        set_error(lexer, "Unterminated raw multi-line string literal");
        free(value);
        return token_create(TOKEN_ERROR, start_loc);
    }
    
    value[value_length] = '\0';
    
    Token token = token_create(TOKEN_STRING, start_loc);
    token.data.string.value = value;
    token.data.string.length = value_length;
    
    return token;
}

Token scan_multiline_processed_string(Lexer *lexer) {
    SourceLocation start_loc = current_location(lexer);
    
    // Consume opening """
    advance_char(lexer); // "
    advance_char(lexer); // "
    advance_char(lexer); // "
    
    // Start with reasonable buffer
    char *value = malloc(LEXER_SCAN_INITIAL_MULTILINE_BUFFER_SIZE);
    size_t value_capacity = LEXER_SCAN_INITIAL_MULTILINE_BUFFER_SIZE;
    size_t value_length = 0;
    
    bool found_closing = false;
    
    // Scan until closing """
    while (lexer->position < lexer->source_length) {
        // Check for closing delimiter first
        if (lexer->position + 2 < lexer->source_length &&
            peek_char(lexer, 0) == '"' &&
            peek_char(lexer, 1) == '"' &&
            peek_char(lexer, 2) == '"') {
            // Found closing delimiter
            advance_char(lexer); // "
            advance_char(lexer); // "
            advance_char(lexer); // "
            found_closing = true;
            break;
        }
        
        char c = peek_char(lexer, 0);
        
        if (c == '\\') {
            // Process escape sequences (like regular strings)
            advance_char(lexer); // consume backslash
            if (lexer->position >= lexer->source_length) {
                set_error(lexer, "Unterminated escape sequence in multi-line string literal");
                free(value);
                return token_create(TOKEN_ERROR, start_loc);
            }
            
            char escaped = peek_char(lexer, 0);
            advance_char(lexer); // consume escaped character
            
            bool error_occurred;
            char processed = process_escape_sequence(lexer, escaped, &error_occurred);
            if (error_occurred) {
                free(value);
                return token_create(TOKEN_ERROR, start_loc);
            }
            
            // Resize buffer if needed
            if (value_length + 1 >= value_capacity) {
                value_capacity *= 2;
                value = realloc(value, value_capacity);
                if (!value) {
                    set_error(lexer, "Memory allocation failed");
                    return token_create(TOKEN_ERROR, start_loc);
                }
            }
            
            value[value_length++] = processed;
        } else {
            // Regular character - resize buffer if needed
            if (value_length + 1 >= value_capacity) {
                value_capacity *= 2;
                value = realloc(value, value_capacity);
                if (!value) {
                    set_error(lexer, "Memory allocation failed");
                    return token_create(TOKEN_ERROR, start_loc);
                }
            }
            
            value[value_length++] = advance_char(lexer);
        }
    }
    
    // Check if we found the closing delimiter
    if (!found_closing) {
        set_error(lexer, "Unterminated processed multi-line string literal");
        free(value);
        return token_create(TOKEN_ERROR, start_loc);
    }
    
    value[value_length] = '\0';
    
    Token token = token_create(TOKEN_STRING, start_loc);
    token.data.string.value = value;
    token.data.string.length = value_length;
    
    return token;
} 
