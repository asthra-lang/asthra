/**
 * Asthra Programming Language Compiler
 * Parser error handling and reporting
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "parser.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// =============================================================================
// ERROR HANDLING AND REPORTING
// =============================================================================

void report_error(Parser *parser, const char *message) {
    if (!parser || !message) return;
    
    report_error_at(parser, parser->current_token.location, message);
}

void report_error_at(Parser *parser, SourceLocation location, const char *message) {
    if (!parser || !message) return;
    
    // Check if we've hit the error limit
    if (parser->error_count >= parser->config.max_errors) {
        return;
    }
    
    ParseError *error = malloc(sizeof(ParseError));
    if (!error) return;
    
    error->location = location;
    error->message = strdup(message);
    error->is_warning = false;
    error->next = NULL;
    
    if (parser->last_error) {
        parser->last_error->next = error;
    } else {
        parser->errors = error;
    }
    parser->last_error = error;
    parser->error_count++;
    parser->had_error = true;
    
    // Print error immediately for debugging
    fprintf(stderr, "Error at %s:%d:%d: %s\n", 
            location.filename ? location.filename : "<unknown>",
            location.line, location.column, message);
}

void report_warning(Parser *parser, const char *message) {
    if (!parser || !message) return;
    
    ParseError *warning = malloc(sizeof(ParseError));
    if (!warning) return;
    
    warning->location = parser->current_token.location;
    warning->message = strdup(message);
    warning->is_warning = true;
    warning->next = NULL;
    
    if (parser->last_error) {
        parser->last_error->next = warning;
    } else {
        parser->errors = warning;
    }
    parser->last_error = warning;
    
    // Print warning immediately for debugging
    fprintf(stderr, "Warning at %s:%d:%d: %s\n", 
            warning->location.filename ? warning->location.filename : "<unknown>",
            warning->location.line, warning->location.column, message);
}

void synchronize(Parser *parser) {
    if (!parser) return;
    
    parser->panic_mode = false;
    
    while (!at_end(parser)) {
        if (parser->current_token.type == TOKEN_SEMICOLON) {
            advance_token(parser);
            return;
        }
        
        switch (parser->current_token.type) {
            case TOKEN_FN:
            case TOKEN_STRUCT:
            case TOKEN_EXTERN:
            case TOKEN_LET:
            case TOKEN_IF:
            case TOKEN_FOR:
            case TOKEN_RETURN:
            case TOKEN_MATCH:
            case TOKEN_SPAWN:
            case TOKEN_UNSAFE:
                return;
            default:
                break;
        }
        
        advance_token(parser);
    }
}

void enter_panic_mode(Parser *parser) {
    if (parser) {
        parser->panic_mode = true;
    }
}

void exit_panic_mode(Parser *parser) {
    if (parser) {
        parser->panic_mode = false;
    }
}

bool parser_had_warning(Parser *parser) {
    if (!parser) return false;
    
    // Check if any errors are actually warnings
    ParseError *current = parser->errors;
    while (current) {
        if (current->is_warning) {
            return true;
        }
        current = current->next;
    }
    return false;
}

// =============================================================================
// IMPORT VALIDATION FUNCTIONS - Phase 1: Import System Enhancement
// =============================================================================

bool validate_import_path_format(const char* path) {
    if (!path) return false;
    
    // Basic format validation (not complex grammar rules)
    size_t len = strlen(path);
    if (len == 0) return false;
    
    // No double spaces
    if (strstr(path, "  ")) return false;
    
    // No leading/trailing spaces
    if (path[0] == ' ' || path[len-1] == ' ') return false;
    
    // Validate known patterns
    if (strncmp(path, "stdlib/", 7) == 0) return validate_stdlib_path(path);
    if (strncmp(path, "internal/", 9) == 0) return false;  // Internal paths are restricted for user code
    if (strstr(path, "github.com/") || strstr(path, "gitlab.com/") || strstr(path, "bitbucket.org/") ||
        strstr(path, "codeberg.org/") || strstr(path, "sr.ht/")) {
        return validate_github_path(path);
    }
    if (path[0] == '.' && (path[1] == '/' || (path[1] == '.' && path[2] == '/'))) {
        return validate_local_path(path);
    }
    
    // Check if it looks like a local path without proper prefix
    if (strchr(path, '/') && path[0] != '.' && strncmp(path, "stdlib/", 7) != 0 && 
        strncmp(path, "internal/", 9) != 0 && !strstr(path, ".com/") && !strstr(path, ".org/")) {
        return false;  // Reject paths that look local but don't have proper prefix
    }
    
    // Also reject simple identifiers that should be local paths
    if (!strchr(path, '/') && !strstr(path, ".com") && !strstr(path, ".org")) {
        return false;  // Reject simple names like "utils" that should be "./utils"
    }
    
    return true;  // Allow unknown patterns for future extensibility
}

bool validate_stdlib_path(const char* path) {
    if (!path || strncmp(path, "stdlib/", 7) != 0) return false;
    
    const char* module_name = path + 7;  // Skip "stdlib/"
    if (strlen(module_name) == 0) return false;
    
    // Check for valid module names (basic validation)
    // Valid stdlib modules: string, io, collections, math, etc.
    const char* valid_modules[] = {
        "string", "io", "collections", "math", "time", "fs", "net", 
        "crypto", "json", "xml", "http", "concurrent", "testing", NULL
    };
    
    for (int i = 0; valid_modules[i]; i++) {
        size_t mod_len = strlen(valid_modules[i]);
        if (strncmp(module_name, valid_modules[i], mod_len) == 0) {
            // Allow submodules like "collections/hashmap"
            if (module_name[mod_len] == '\0' || module_name[mod_len] == '/') {
                return true;
            }
        }
    }
    
    return true;  // Allow unknown stdlib modules for extensibility
}

bool validate_internal_path(const char* path) {
    if (!path || strncmp(path, "internal/", 9) != 0) return false;
    
    const char* module_name = path + 9;  // Skip "internal/"
    if (strlen(module_name) == 0) return false;
    
    // Internal modules are restricted - only compiler internals
    const char* valid_internal[] = {
        "runtime", "memory", "gc", "ffi", "codegen", "analysis", NULL
    };
    
    for (int i = 0; valid_internal[i]; i++) {
        size_t mod_len = strlen(valid_internal[i]);
        if (strncmp(module_name, valid_internal[i], mod_len) == 0) {
            if (module_name[mod_len] == '\0' || module_name[mod_len] == '/') {
                return true;
            }
        }
    }
    
    return false;  // Internal modules are strictly controlled
}

bool validate_github_path(const char* path) {
    if (!path) return false;
    
    // Look for domain patterns
    const char* domains[] = {
        "github.com/", "gitlab.com/", "bitbucket.org/", 
        "codeberg.org/", "sr.ht/", NULL
    };
    
    const char* domain_start = NULL;
    size_t domain_len = 0;
    
    for (int i = 0; domains[i]; i++) {
        domain_start = strstr(path, domains[i]);
        if (domain_start == path) {  // Must be at start
            domain_len = strlen(domains[i]);
            break;
        }
    }
    
    if (!domain_start) return false;
    
    const char* repo_path = path + domain_len;
    if (strlen(repo_path) == 0) return false;
    
    // Must have at least "user/repo" format
    const char* slash = strchr(repo_path, '/');
    if (!slash || slash == repo_path) return false;  // No username
    if (strlen(slash + 1) == 0) return false;        // No repo name
    
    return true;
}

bool validate_local_path(const char* path) {
    if (!path) return false;
    
    // Must start with "./" or "../"
    if (!(path[0] == '.' && path[1] == '/') && 
        !(path[0] == '.' && path[1] == '.' && path[2] == '/')) {
        return false;
    }
    
    // Check for basic path validity (no null bytes, reasonable length)
    size_t len = strlen(path);
    if (len > 512) return false;  // Reasonable path length limit
    
    // No ".." after initial "../" for security
    const char* dotdot = strstr(path + 3, "..");
    if (dotdot) return false;
    
    return true;
}

void report_import_path_error(Parser* parser, ImportPathErrorType error_type, 
                             const char* path, int line_number) {
    if (!parser || !path) return;
    
    switch (error_type) {
        case IMPORT_ERROR_EMPTY_PATH:
            parser_error_with_suggestion(parser, line_number,
                "Empty import path \"\"",
                "Specify a valid package path like \"stdlib/io\" or \"github.com/user/repo\"");
            break;
        case IMPORT_ERROR_INVALID_GITHUB: {
            char error_msg[512];
            snprintf(error_msg, sizeof(error_msg), "Invalid GitHub path \"%s\"", path);
            parser_error_with_suggestion(parser, line_number, error_msg,
                "GitHub imports require format \"github.com/user/repository\"");
            break;
        }
        case IMPORT_ERROR_INVALID_STDLIB: {
            char error_msg[512];
            snprintf(error_msg, sizeof(error_msg), "Invalid stdlib path \"%s\"", path);
            parser_error_with_suggestion(parser, line_number, error_msg,
                "Use format \"stdlib/module\" (e.g., \"stdlib/string\", \"stdlib/io\")");
            break;
        }
        case IMPORT_ERROR_INTERNAL_ACCESS_DENIED: {
            char error_msg[512];
            snprintf(error_msg, sizeof(error_msg), "Cannot import internal package \"%s\"", path);
            parser_error_with_suggestion(parser, line_number, error_msg,
                "Internal packages are restricted. Use stdlib alternatives or third-party packages");
            break;
        }
        case IMPORT_ERROR_WHITESPACE: {
            char error_msg[512];
            snprintf(error_msg, sizeof(error_msg), "Invalid whitespace in import path \"%s\"", path);
            parser_error_with_suggestion(parser, line_number, error_msg,
                "Remove extra spaces and use valid path format");
            break;
        }
        case IMPORT_ERROR_INVALID_LOCAL: {
            char error_msg[512];
            snprintf(error_msg, sizeof(error_msg), "Invalid local import path \"%s\"", path);
            parser_error_with_suggestion(parser, line_number, error_msg,
                "Local imports must start with \"./\" or \"../\"");
            break;
        }
        case IMPORT_ERROR_MALFORMED_DOMAIN: {
            char error_msg[512];
            snprintf(error_msg, sizeof(error_msg), "Malformed domain in import path \"%s\"", path);
            parser_error_with_suggestion(parser, line_number, error_msg,
                "Use valid domain like github.com, gitlab.com, or bitbucket.org");
            break;
        }
        case IMPORT_ERROR_INCOMPLETE_PATH: {
            char error_msg[512];
            snprintf(error_msg, sizeof(error_msg), "Incomplete import path \"%s\"", path);
            parser_error_with_suggestion(parser, line_number, error_msg,
                "Specify complete path including repository name");
            break;
        }
    }
}

void parser_error_with_suggestion(Parser* parser, int line_number,
                                const char* error_message, const char* suggestion) {
    if (!parser || !error_message || !suggestion) return;
    
    // Create formatted error message with suggestion
    size_t msg_len = strlen(error_message) + strlen(suggestion) + 100;
    char* formatted_msg = malloc(msg_len);
    if (!formatted_msg) {
        report_error(parser, error_message);  // Fallback to basic error
        return;
    }
    
    snprintf(formatted_msg, msg_len, "%s\n  Suggestion: %s", error_message, suggestion);
    
    SourceLocation loc = parser->current_token.location;
    loc.line = line_number;
    
    report_error_at(parser, loc, formatted_msg);
    free(formatted_msg);
} 
