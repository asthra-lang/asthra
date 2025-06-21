/**
 * Asthra Programming Language Runtime - TOML Support Implementation
 * Minimal TOML parsing and generation support for stdlib::toml
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "stdlib_toml_support.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

// =============================================================================
// INTERNAL UTILITIES
// =============================================================================

static char* asthra_strdup(const char* str) {
    if (!str) return NULL;
    size_t len = strlen(str);
    char* copy = malloc(len + 1);
    if (copy) {
        memcpy(copy, str, len + 1);  // More efficient than snprintf for simple copy
    }
    return copy;
}

static void skip_whitespace(const char** input) {
    while (**input && (isspace(**input) || **input == '#')) {
        if (**input == '#') {
            // Skip comment line
            while (**input && **input != '\n') {
                (*input)++;
            }
        }
        if (**input) (*input)++;
    }
}

static bool parse_key(const char** input, char** key) {
    skip_whitespace(input);
    const char* start = *input;
    
    if (!isalpha(**input) && **input != '_') {
        return false;
    }
    
    while (isalnum(**input) || **input == '_' || **input == '-') {
        (*input)++;
    }
    
    size_t len = *input - start;
    *key = malloc(len + 1);
    if (!*key) return false;
    
    strncpy(*key, start, len);
    (*key)[len] = '\0';
    return true;
}

static bool parse_string_value(const char** input, char** value) {
    skip_whitespace(input);
    
    if (**input != '"') return false;
    (*input)++; // Skip opening quote
    
    const char* start = *input;
    while (**input && **input != '"') {
        if (**input == '\\') {
            (*input)++; // Skip escape character
            if (**input) (*input)++; // Skip escaped character
        } else {
            (*input)++;
        }
    }
    
    if (**input != '"') return false;
    
    size_t len = *input - start;
    *value = malloc(len + 1);
    if (!*value) return false;
    
    strncpy(*value, start, len);
    (*value)[len] = '\0';
    
    (*input)++; // Skip closing quote
    return true;
}

static bool parse_integer_value(const char** input, int64_t* value) {
    skip_whitespace(input);
    char* endptr;
    *value = strtoll(*input, &endptr, 10);
    
    if (endptr == *input) return false;
    *input = endptr;
    return true;
}

static bool parse_float_value(const char** input, double* value) {
    skip_whitespace(input);
    char* endptr;
    *value = strtod(*input, &endptr);
    
    if (endptr == *input) return false;
    *input = endptr;
    return true;
}

static bool parse_boolean_value(const char** input, bool* value) {
    skip_whitespace(input);
    
    if (strncmp(*input, "true", 4) == 0) {
        *value = true;
        *input += 4;
        return true;
    } else if (strncmp(*input, "false", 5) == 0) {
        *value = false;
        *input += 5;
        return true;
    }
    
    return false;
}

// =============================================================================
// TOML VALUE CREATION FUNCTIONS
// =============================================================================

AsthraTOMLValue* asthra_toml_string_value(const char* str) {
    AsthraTOMLValue* value = calloc(1, sizeof(AsthraTOMLValue));
    if (!value) return NULL;
    
    value->type = ASTHRA_TOML_STRING;
    value->data.string_val = asthra_strdup(str);
    value->ownership = ASTHRA_OWNERSHIP_C;
    
    if (!value->data.string_val) {
        free(value);
        return NULL;
    }
    
    return value;
}

AsthraTOMLValue* asthra_toml_integer_value(int64_t val) {
    AsthraTOMLValue* value = calloc(1, sizeof(AsthraTOMLValue));
    if (!value) return NULL;
    
    value->type = ASTHRA_TOML_INTEGER;
    value->data.integer_val = val;
    value->ownership = ASTHRA_OWNERSHIP_C;
    
    return value;
}

AsthraTOMLValue* asthra_toml_float_value(double val) {
    AsthraTOMLValue* value = calloc(1, sizeof(AsthraTOMLValue));
    if (!value) return NULL;
    
    value->type = ASTHRA_TOML_FLOAT;
    value->data.float_val = val;
    value->ownership = ASTHRA_OWNERSHIP_C;
    
    return value;
}

AsthraTOMLValue* asthra_toml_boolean_value(bool val) {
    AsthraTOMLValue* value = calloc(1, sizeof(AsthraTOMLValue));
    if (!value) return NULL;
    
    value->type = ASTHRA_TOML_BOOLEAN;
    value->data.boolean_val = val;
    value->ownership = ASTHRA_OWNERSHIP_C;
    
    return value;
}

AsthraTOMLValue* asthra_toml_array_value(AsthraTOMLValue** values, size_t count) {
    AsthraTOMLValue* value = calloc(1, sizeof(AsthraTOMLValue));
    if (!value) return NULL;
    
    value->type = ASTHRA_TOML_ARRAY;
    value->data.array_val.items = calloc(count, sizeof(AsthraTOMLValue));
    value->data.array_val.count = count;
    value->data.array_val.capacity = count;
    value->ownership = ASTHRA_OWNERSHIP_C;
    
    if (count > 0 && !value->data.array_val.items) {
        free(value);
        return NULL;
    }
    
    for (size_t i = 0; i < count; i++) {
        if (values[i]) {
            value->data.array_val.items[i] = *values[i];
        }
    }
    
    return value;
}

AsthraTOMLValue* asthra_toml_table_value(AsthraTOMLTable* table) {
    AsthraTOMLValue* value = calloc(1, sizeof(AsthraTOMLValue));
    if (!value) return NULL;
    
    value->type = ASTHRA_TOML_TABLE;
    value->data.table_val = table;
    value->ownership = ASTHRA_OWNERSHIP_C;
    
    return value;
}

// =============================================================================
// TOML TABLE OPERATIONS
// =============================================================================

AsthraTOMLTable* asthra_toml_table_create(void) {
    AsthraTOMLTable* table = calloc(1, sizeof(AsthraTOMLTable));
    if (!table) return NULL;
    
    table->capacity = 8;
    table->keys = calloc(table->capacity, sizeof(char*));
    table->values = calloc(table->capacity, sizeof(AsthraTOMLValue));
    table->ownership = ASTHRA_OWNERSHIP_C;
    
    if (!table->keys || !table->values) {
        free(table->keys);
        free(table->values);
        free(table);
        return NULL;
    }
    
    return table;
}

AsthraTOMLValue* asthra_toml_table_get(const AsthraTOMLTable* table, const char* key) {
    if (!table || !key) return NULL;
    
    for (size_t i = 0; i < table->count; i++) {
        if (table->keys[i] && strcmp(table->keys[i], key) == 0) {
            return &table->values[i];
        }
    }
    
    return NULL;
}

AsthraResult asthra_toml_table_set(AsthraTOMLTable* table, const char* key, AsthraTOMLValue* value) {
    if (!table || !key || !value) {
        return asthra_result_err_cstr("Invalid arguments to table_set");
    }
    
    // Check if key already exists
    for (size_t i = 0; i < table->count; i++) {
        if (table->keys[i] && strcmp(table->keys[i], key) == 0) {
            // Update existing value
            asthra_toml_value_free(&table->values[i]);
            table->values[i] = *value;
            return asthra_result_ok_cstr("Value updated");
        }
    }
    
    // Add new key-value pair
    if (table->count >= table->capacity) {
        // Expand table
        size_t new_capacity = table->capacity * 2;
        char** new_keys = realloc(table->keys, new_capacity * sizeof(char*));
        AsthraTOMLValue* new_values = realloc(table->values, new_capacity * sizeof(AsthraTOMLValue));
        
        if (!new_keys || !new_values) {
            return asthra_result_err_cstr("Memory allocation failed");
        }
        
        table->keys = new_keys;
        table->values = new_values;
        table->capacity = new_capacity;
        
        // Initialize new slots
        for (size_t i = table->count; i < new_capacity; i++) {
            table->keys[i] = NULL;
            memset(&table->values[i], 0, sizeof(AsthraTOMLValue));
        }
    }
    
    table->keys[table->count] = asthra_strdup(key);
    table->values[table->count] = *value;
    table->count++;
    
    return asthra_result_ok_cstr("Value added");
}

bool asthra_toml_table_contains_key(const AsthraTOMLTable* table, const char* key) {
    return asthra_toml_table_get(table, key) != NULL;
}

size_t asthra_toml_table_size(const AsthraTOMLTable* table) {
    return table ? table->count : 0;
}

// =============================================================================
// TOML DOCUMENT OPERATIONS
// =============================================================================

AsthraTOMLDocument* asthra_toml_document_create(void) {
    AsthraTOMLDocument* doc = calloc(1, sizeof(AsthraTOMLDocument));
    if (!doc) return NULL;
    
    doc->root_table = asthra_toml_table_create();
    if (!doc->root_table) {
        free(doc);
        return NULL;
    }
    
    doc->ownership = ASTHRA_OWNERSHIP_C;
    return doc;
}

AsthraTOMLTable* asthra_toml_document_get_root(const AsthraTOMLDocument* document) {
    return document ? document->root_table : NULL;
}

bool asthra_toml_document_has_errors(const AsthraTOMLDocument* document) {
    return document ? document->has_errors : false;
}

const char** asthra_toml_document_get_errors(const AsthraTOMLDocument* document, size_t* error_count) {
    if (!document || !error_count) return NULL;
    
    *error_count = document->error_count;
    return (const char**)document->parse_errors;
}

// =============================================================================
// CORE TOML PARSING FUNCTIONS
// =============================================================================

AsthraResult asthra_toml_parse_string(const char* input) {
    AsthraTOMLConfig config = asthra_toml_default_config();
    return asthra_toml_parse_string_with_config(input, &config);
}

AsthraResult asthra_toml_parse_string_with_config(const char* input, const AsthraTOMLConfig* config) {
    if (!input) {
        return asthra_result_err_cstr("NULL input string");
    }
    
    AsthraTOMLDocument* doc = asthra_toml_document_create();
    if (!doc) {
        return asthra_result_err_cstr("Failed to create document");
    }
    
    const char* current = input;
    
    while (*current) {
        skip_whitespace(&current);
        if (!*current) break;
        
        // Parse key-value pair
        char* key = NULL;
        if (!parse_key(&current, &key)) {
            asthra_toml_document_free(doc);
            return asthra_result_err_cstr("Failed to parse key");
        }
        
        skip_whitespace(&current);
        if (*current != '=') {
            free(key);
            asthra_toml_document_free(doc);
            return asthra_result_err_cstr("Expected '=' after key");
        }
        current++; // Skip '='
        
        skip_whitespace(&current);
        
        // Try to parse value
        AsthraTOMLValue* value = NULL;
        
        if (*current == '"') {
            char* str_val;
            if (parse_string_value(&current, &str_val)) {
                value = asthra_toml_string_value(str_val);
                free(str_val);
            }
        } else if (strncmp(current, "true", 4) == 0 || strncmp(current, "false", 5) == 0) {
            bool bool_val;
            if (parse_boolean_value(&current, &bool_val)) {
                value = asthra_toml_boolean_value(bool_val);
            }
        } else if (strchr(current, '.')) {
            double float_val;
            if (parse_float_value(&current, &float_val)) {
                value = asthra_toml_float_value(float_val);
            }
        } else {
            int64_t int_val;
            if (parse_integer_value(&current, &int_val)) {
                value = asthra_toml_integer_value(int_val);
            }
        }
        
        if (!value) {
            free(key);
            asthra_toml_document_free(doc);
            return asthra_result_err_cstr("Failed to parse value");
        }
        
        AsthraResult set_result = asthra_toml_table_set(doc->root_table, key, value);
        free(key);
        
        if (set_result.tag != ASTHRA_RESULT_OK) {
            asthra_toml_value_free(value);
            free(value);
            asthra_toml_document_free(doc);
            return asthra_result_err_cstr("Failed to set table value");
        }
        
        // Skip to next line
        while (*current && *current != '\n') {
            current++;
        }
        if (*current) current++;
    }
    
    return asthra_result_ok(doc, sizeof(AsthraTOMLDocument*), 0, ASTHRA_OWNERSHIP_C);
}

AsthraResult asthra_toml_parse_file(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        return asthra_result_err_cstr("Failed to open file");
    }
    
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    char* content = malloc(size + 1);
    if (!content) {
        fclose(file);
        return asthra_result_err_cstr("Memory allocation failed");
    }
    
    size_t bytes_read = fread(content, 1, size, file);
    if (bytes_read != size) {
        free(content);
        fclose(file);
        return asthra_result_err_cstr("Failed to read complete file");
    }
    content[size] = '\0';
    fclose(file);
    
    AsthraResult result = asthra_toml_parse_string(content);
    free(content);
    
    return result;
}

AsthraResult asthra_toml_generate_string(const AsthraTOMLDocument* document) {
    AsthraTOMLConfig config = asthra_toml_default_config();
    return asthra_toml_generate_string_with_config(document, &config);
}

AsthraResult asthra_toml_generate_string_with_config(const AsthraTOMLDocument* document, const AsthraTOMLConfig* config) {
    if (!document || !document->root_table) {
        return asthra_result_err_cstr("Invalid document");
    }
    
    // Simple generation - just key = value pairs
    size_t buffer_size = 1024;
    char* buffer = malloc(buffer_size);
    if (!buffer) {
        return asthra_result_err_cstr("Memory allocation failed");
    }
    
    buffer[0] = '\0';
    size_t pos = 0;
    
    AsthraTOMLTable* table = document->root_table;
    for (size_t i = 0; i < table->count; i++) {
        if (!table->keys[i]) continue;
        
        // Ensure buffer has enough space
        size_t needed = strlen(table->keys[i]) + 64; // Conservative estimate
        if (pos + needed >= buffer_size) {
            buffer_size *= 2;
            char* new_buffer = realloc(buffer, buffer_size);
            if (!new_buffer) {
                free(buffer);
                return asthra_result_err_cstr("Memory reallocation failed");
            }
            buffer = new_buffer;
        }
        
        // Add key
        int key_len = sprintf(buffer + pos, "%s = ", table->keys[i]);
        if (key_len < 0) {
            free(buffer);
            return asthra_result_err_cstr("sprintf failed");
        }
        pos += (size_t)key_len;
        
        // Add value based on type
        AsthraTOMLValue* value = &table->values[i];
        int val_len;
        switch (value->type) {
            case ASTHRA_TOML_STRING:
                val_len = sprintf(buffer + pos, "\"%s\"", value->data.string_val);
                break;
            case ASTHRA_TOML_INTEGER:
                val_len = sprintf(buffer + pos, "%lld", (long long)value->data.integer_val);
                break;
            case ASTHRA_TOML_FLOAT:
                val_len = sprintf(buffer + pos, "%f", value->data.float_val);
                break;
            case ASTHRA_TOML_BOOLEAN:
                val_len = sprintf(buffer + pos, "%s", value->data.boolean_val ? "true" : "false");
                break;
            default:
                val_len = sprintf(buffer + pos, "\"unsupported\"");
                break;
        }
        
        if (val_len < 0) {
            free(buffer);
            return asthra_result_err_cstr("sprintf failed");
        }
        pos += (size_t)val_len;
        
        int newline_len = sprintf(buffer + pos, "\n");
        if (newline_len < 0) {
            free(buffer);
            return asthra_result_err_cstr("sprintf failed");
        }
        pos += (size_t)newline_len;
    }
    
    return asthra_result_ok(buffer, strlen(buffer) + 1, 0, ASTHRA_OWNERSHIP_C);
}

// =============================================================================
// TOML CONFIGURATION
// =============================================================================

AsthraTOMLConfig asthra_toml_default_config(void) {
    AsthraTOMLConfig config = {
        .preserve_order = true,
        .allow_mixed_arrays = false,
        .strict_mode = true,
        .max_depth = 64
    };
    return config;
}

AsthraTOMLConfig asthra_toml_lenient_config(void) {
    AsthraTOMLConfig config = {
        .preserve_order = false,
        .allow_mixed_arrays = true,
        .strict_mode = false,
        .max_depth = 128
    };
    return config;
}

// =============================================================================
// MEMORY MANAGEMENT
// =============================================================================

void asthra_toml_value_free(AsthraTOMLValue* value) {
    if (!value) return;
    
    switch (value->type) {
        case ASTHRA_TOML_STRING:
        case ASTHRA_TOML_DATETIME:
            free(value->data.string_val);
            break;
        case ASTHRA_TOML_ARRAY:
            if (value->data.array_val.items) {
                for (size_t i = 0; i < value->data.array_val.count; i++) {
                    asthra_toml_value_free(&value->data.array_val.items[i]);
                }
                free(value->data.array_val.items);
            }
            break;
        case ASTHRA_TOML_TABLE:
        case ASTHRA_TOML_INLINE_TABLE:
            asthra_toml_table_free(value->data.table_val);
            break;
        default:
            // Primitive types don't need special cleanup
            break;
    }
    
    memset(value, 0, sizeof(AsthraTOMLValue));
}

void asthra_toml_table_free(AsthraTOMLTable* table) {
    if (!table) return;
    
    for (size_t i = 0; i < table->count; i++) {
        free(table->keys[i]);
        asthra_toml_value_free(&table->values[i]);
    }
    
    free(table->keys);
    free(table->values);
    free(table);
}

void asthra_toml_document_free(AsthraTOMLDocument* document) {
    if (!document) return;
    
    asthra_toml_table_free(document->root_table);
    
    if (document->parse_errors) {
        for (size_t i = 0; i < document->error_count; i++) {
            free(document->parse_errors[i]);
        }
        free(document->parse_errors);
    }
    
    free(document);
}

// =============================================================================
// UTILITY FUNCTIONS
// =============================================================================

const char* asthra_toml_value_type_string(AsthraTOMLValueType type) {
    switch (type) {
        case ASTHRA_TOML_STRING: return "string";
        case ASTHRA_TOML_INTEGER: return "integer";
        case ASTHRA_TOML_FLOAT: return "float";
        case ASTHRA_TOML_BOOLEAN: return "boolean";
        case ASTHRA_TOML_DATETIME: return "datetime";
        case ASTHRA_TOML_ARRAY: return "array";
        case ASTHRA_TOML_TABLE: return "table";
        case ASTHRA_TOML_INLINE_TABLE: return "inline_table";
        default: return "unknown";
    }
}

bool asthra_toml_value_is_primitive(const AsthraTOMLValue* value) {
    if (!value) return false;
    
    switch (value->type) {
        case ASTHRA_TOML_STRING:
        case ASTHRA_TOML_INTEGER:
        case ASTHRA_TOML_FLOAT:
        case ASTHRA_TOML_BOOLEAN:
        case ASTHRA_TOML_DATETIME:
            return true;
        default:
            return false;
    }
}

AsthraTOMLValue* asthra_toml_value_clone(const AsthraTOMLValue* value) {
    if (!value) return NULL;
    
    switch (value->type) {
        case ASTHRA_TOML_STRING:
            return asthra_toml_string_value(value->data.string_val);
        case ASTHRA_TOML_INTEGER:
            return asthra_toml_integer_value(value->data.integer_val);
        case ASTHRA_TOML_FLOAT:
            return asthra_toml_float_value(value->data.float_val);
        case ASTHRA_TOML_BOOLEAN:
            return asthra_toml_boolean_value(value->data.boolean_val);
        case ASTHRA_TOML_DATETIME:
            return asthra_toml_string_value(value->data.datetime_val);
        default:
            // Complex types not supported in minimal implementation
            return NULL;
    }
} 
