#ifndef ASTHRA_JSON_UTILS_H
#define ASTHRA_JSON_UTILS_H

// Integration with json-c library
#include <json.h>
#include <stdbool.h>
#include <stddef.h>

// Asthra JSON utilities
typedef struct {
    json_object *root;
    char *error_message;
    bool success;
} AsthrraJSONResult;

// JSON parsing and generation functions
AsthrraJSONResult asthra_json_parse(const char *json_string);
char* asthra_json_generate(json_object *obj);
char* asthra_json_generate_pretty(json_object *obj);
void asthra_json_result_free(AsthrraJSONResult *result);

// JSON object creation
json_object* asthra_json_create_object(void);
json_object* asthra_json_create_array(void);
json_object* asthra_json_create_string(const char *value);
json_object* asthra_json_create_int(int value);
json_object* asthra_json_create_double(double value);
json_object* asthra_json_create_boolean(bool value);

// JSON object manipulation
json_object* asthra_json_get_object(json_object *obj, const char *key);
const char* asthra_json_get_string(json_object *obj, const char *key);
int asthra_json_get_int(json_object *obj, const char *key);
double asthra_json_get_double(json_object *obj, const char *key);
bool asthra_json_get_boolean(json_object *obj, const char *key);

// JSON object modification
bool asthra_json_set_object(json_object *obj, const char *key, json_object *value);
bool asthra_json_set_string(json_object *obj, const char *key, const char *value);
bool asthra_json_set_int(json_object *obj, const char *key, int value);
bool asthra_json_set_double(json_object *obj, const char *key, double value);
bool asthra_json_set_boolean(json_object *obj, const char *key, bool value);

// JSON array operations
int asthra_json_array_length(json_object *array);
json_object* asthra_json_array_get(json_object *array, int index);
bool asthra_json_array_add(json_object *array, json_object *value);

// JSON validation and utilities
bool asthra_json_is_valid(const char *json_string);
bool asthra_json_has_key(json_object *obj, const char *key);
int asthra_json_object_length(json_object *obj);

// Error handling
const char* asthra_json_get_last_error(void);
void asthra_json_clear_error(void);

// ============================================================================
// Enhanced API for Asthra stdlib compatibility
// ============================================================================

// Type alias for Asthra FFI bridge
typedef json_object JSONValueHandle;

// Core FFI bridge functions for Asthra stdlib
JSONValueHandle* asthra_stdlib_json_parse(const char *input);
char* asthra_stdlib_json_stringify(JSONValueHandle *handle);
char* asthra_stdlib_json_stringify_pretty(JSONValueHandle *handle);
void asthra_stdlib_json_free_handle(JSONValueHandle *handle);
void asthra_stdlib_json_free_string(char *str);

// Type creation functions for Asthra FFI
JSONValueHandle* asthra_stdlib_json_null(void);
JSONValueHandle* asthra_stdlib_json_bool(bool value);
JSONValueHandle* asthra_stdlib_json_number(double value);
JSONValueHandle* asthra_stdlib_json_string(const char *str);
JSONValueHandle* asthra_stdlib_json_array(void);
JSONValueHandle* asthra_stdlib_json_object(void);

// Object operations for Asthra FFI
bool asthra_stdlib_json_object_set(JSONValueHandle *obj, const char *key, JSONValueHandle *value);
JSONValueHandle* asthra_stdlib_json_object_get(JSONValueHandle *obj, const char *key);
bool asthra_stdlib_json_object_has(JSONValueHandle *obj, const char *key);
size_t asthra_stdlib_json_object_size(JSONValueHandle *obj);

// Array operations for Asthra FFI
bool asthra_stdlib_json_array_push(JSONValueHandle *array, JSONValueHandle *value);
JSONValueHandle* asthra_stdlib_json_array_get(JSONValueHandle *array, size_t index);
size_t asthra_stdlib_json_array_length(JSONValueHandle *array);

// Type checking for Asthra FFI
bool asthra_stdlib_json_is_null(JSONValueHandle *handle);
bool asthra_stdlib_json_is_bool(JSONValueHandle *handle);
bool asthra_stdlib_json_is_number(JSONValueHandle *handle);
bool asthra_stdlib_json_is_string(JSONValueHandle *handle);
bool asthra_stdlib_json_is_array(JSONValueHandle *handle);
bool asthra_stdlib_json_is_object(JSONValueHandle *handle);

// Value extraction for Asthra FFI
bool asthra_stdlib_json_get_bool(JSONValueHandle *handle);
double asthra_stdlib_json_get_number(JSONValueHandle *handle);
const char* asthra_stdlib_json_get_string(JSONValueHandle *handle);

// Validation for Asthra FFI
bool asthra_stdlib_json_is_valid(const char *input);

#endif // ASTHRA_JSON_UTILS_H 
