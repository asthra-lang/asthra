#include "json_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Global error state
static char *last_error = NULL;

// Helper function to set error message
static void set_error(const char *message) {
    if (last_error) {
        free(last_error);
    }
    last_error = strdup(message);
}

// JSON parsing and generation functions
AsthrraJSONResult asthra_json_parse(const char *json_string) {
    AsthrraJSONResult result = {NULL, NULL, false};

    if (!json_string) {
        result.error_message = strdup("JSON string is NULL");
        return result;
    }

    json_object *root = json_tokener_parse(json_string);
    if (!root) {
        result.error_message = strdup("Invalid JSON string");
        set_error("Failed to parse JSON string");
        return result;
    }

    result.root = root;
    result.success = true;
    return result;
}

char *asthra_json_generate(json_object *obj) {
    if (!obj) {
        set_error("JSON object is NULL");
        return NULL;
    }

    const char *json_str = json_object_to_json_string(obj);
    return json_str ? strdup(json_str) : NULL;
}

char *asthra_json_generate_pretty(json_object *obj) {
    if (!obj) {
        set_error("JSON object is NULL");
        return NULL;
    }

    const char *json_str = json_object_to_json_string_ext(obj, JSON_C_TO_STRING_PRETTY);
    return json_str ? strdup(json_str) : NULL;
}

void asthra_json_result_free(AsthrraJSONResult *result) {
    if (result) {
        if (result->root) {
            json_object_put(result->root);
            result->root = NULL;
        }
        if (result->error_message) {
            free(result->error_message);
            result->error_message = NULL;
        }
        result->success = false;
    }
}

// JSON object creation
json_object *asthra_json_create_object(void) {
    return json_object_new_object();
}

json_object *asthra_json_create_array(void) {
    return json_object_new_array();
}

json_object *asthra_json_create_string(const char *value) {
    return value ? json_object_new_string(value) : NULL;
}

json_object *asthra_json_create_int(int value) {
    return json_object_new_int(value);
}

json_object *asthra_json_create_double(double value) {
    return json_object_new_double(value);
}

json_object *asthra_json_create_boolean(bool value) {
    return json_object_new_boolean(value);
}

// JSON object manipulation
json_object *asthra_json_get_object(json_object *obj, const char *key) {
    if (!obj || !key) {
        set_error("Object or key is NULL");
        return NULL;
    }

    json_object *value = NULL;
    if (json_object_object_get_ex(obj, key, &value)) {
        return value;
    }
    return NULL;
}

const char *asthra_json_get_string(json_object *obj, const char *key) {
    json_object *value = asthra_json_get_object(obj, key);
    if (value && json_object_is_type(value, json_type_string)) {
        return json_object_get_string(value);
    }
    return NULL;
}

int asthra_json_get_int(json_object *obj, const char *key) {
    json_object *value = asthra_json_get_object(obj, key);
    if (value && json_object_is_type(value, json_type_int)) {
        return json_object_get_int(value);
    }
    return 0;
}

double asthra_json_get_double(json_object *obj, const char *key) {
    json_object *value = asthra_json_get_object(obj, key);
    if (value && json_object_is_type(value, json_type_double)) {
        return json_object_get_double(value);
    }
    return 0.0;
}

bool asthra_json_get_boolean(json_object *obj, const char *key) {
    json_object *value = asthra_json_get_object(obj, key);
    if (value && json_object_is_type(value, json_type_boolean)) {
        return json_object_get_boolean(value);
    }
    return false;
}

// JSON object modification
bool asthra_json_set_object(json_object *obj, const char *key, json_object *value) {
    if (!obj || !key) {
        set_error("Object or key is NULL");
        return false;
    }
    return json_object_object_add(obj, key, value) == 0;
}

bool asthra_json_set_string(json_object *obj, const char *key, const char *value) {
    json_object *json_value = asthra_json_create_string(value);
    return json_value ? asthra_json_set_object(obj, key, json_value) : false;
}

bool asthra_json_set_int(json_object *obj, const char *key, int value) {
    json_object *json_value = asthra_json_create_int(value);
    return json_value ? asthra_json_set_object(obj, key, json_value) : false;
}

bool asthra_json_set_double(json_object *obj, const char *key, double value) {
    json_object *json_value = asthra_json_create_double(value);
    return json_value ? asthra_json_set_object(obj, key, json_value) : false;
}

bool asthra_json_set_boolean(json_object *obj, const char *key, bool value) {
    json_object *json_value = asthra_json_create_boolean(value);
    return json_value ? asthra_json_set_object(obj, key, json_value) : false;
}

// JSON array operations
int asthra_json_array_length(json_object *array) {
    if (!array || !json_object_is_type(array, json_type_array)) {
        set_error("Object is not an array");
        return -1;
    }
    return (int)json_object_array_length(array);
}

json_object *asthra_json_array_get(json_object *array, int index) {
    if (!array || !json_object_is_type(array, json_type_array)) {
        set_error("Object is not an array");
        return NULL;
    }
    return json_object_array_get_idx(array, (size_t)index);
}

bool asthra_json_array_add(json_object *array, json_object *value) {
    if (!array || !json_object_is_type(array, json_type_array)) {
        set_error("Object is not an array");
        return false;
    }
    return json_object_array_add(array, value) == 0;
}

// JSON validation and utilities
bool asthra_json_is_valid(const char *json_string) {
    if (!json_string)
        return false;

    // Special case: json-c doesn't consider standalone "null" as valid top-level JSON
    // but according to RFC 7159, it should be valid
    if (strcmp(json_string, "null") == 0)
        return true;

    json_object *obj = json_tokener_parse(json_string);
    if (obj) {
        json_object_put(obj);
        return true;
    }
    return false;
}

bool asthra_json_has_key(json_object *obj, const char *key) {
    if (!obj || !key)
        return false;
    return json_object_object_get_ex(obj, key, NULL);
}

int asthra_json_object_length(json_object *obj) {
    if (!obj || !json_object_is_type(obj, json_type_object)) {
        set_error("Object is not a JSON object");
        return -1;
    }
    return (size_t)json_object_object_length(obj);
}

// Error handling
const char *asthra_json_get_last_error(void) {
    return last_error;
}

void asthra_json_clear_error(void) {
    if (last_error) {
        free(last_error);
        last_error = NULL;
    }
}

// ============================================================================
// Enhanced API for Asthra stdlib compatibility
// ============================================================================

// Core FFI bridge functions for Asthra stdlib
JSONValueHandle *asthra_stdlib_json_parse(const char *input) {
    if (!input)
        return NULL;

    JSONValueHandle *obj = json_tokener_parse(input);
    // json-c already handles reference counting
    return obj;
}

char *asthra_stdlib_json_stringify(JSONValueHandle *handle) {
    if (!handle)
        return NULL;

    const char *json_str = json_object_to_json_string(handle);
    return json_str ? strdup(json_str) : NULL;
}

char *asthra_stdlib_json_stringify_pretty(JSONValueHandle *handle) {
    if (!handle)
        return NULL;

    const char *json_str = json_object_to_json_string_ext(handle, JSON_C_TO_STRING_PRETTY);
    return json_str ? strdup(json_str) : NULL;
}

void asthra_stdlib_json_free_handle(JSONValueHandle *handle) {
    if (handle) {
        json_object_put(handle);
    }
}

void asthra_stdlib_json_free_string(char *str) {
    if (str) {
        free(str);
    }
}

// Type creation functions for Asthra FFI
JSONValueHandle *asthra_stdlib_json_null(void) {
    // json-c represents null as NULL pointer, but we need an actual object
    // for consistency. We'll return a special null object.
    return NULL; // This matches json-c's internal representation
}

JSONValueHandle *asthra_stdlib_json_bool(bool value) {
    return json_object_new_boolean(value);
}

JSONValueHandle *asthra_stdlib_json_number(double value) {
    return json_object_new_double(value);
}

JSONValueHandle *asthra_stdlib_json_string(const char *str) {
    return str ? json_object_new_string(str) : NULL;
}

JSONValueHandle *asthra_stdlib_json_array(void) {
    return json_object_new_array();
}

JSONValueHandle *asthra_stdlib_json_object(void) {
    return json_object_new_object();
}

// Object operations for Asthra FFI
bool asthra_stdlib_json_object_set(JSONValueHandle *obj, const char *key, JSONValueHandle *value) {
    if (!obj || !key)
        return false;
    return json_object_object_add(obj, key, value) == 0;
}

JSONValueHandle *asthra_stdlib_json_object_get(JSONValueHandle *obj, const char *key) {
    if (!obj || !key)
        return NULL;

    json_object *value = NULL;
    if (json_object_object_get_ex(obj, key, &value)) {
        // Increment reference count since we're returning it
        json_object_get(value);
        return value;
    }
    return NULL;
}

bool asthra_stdlib_json_object_has(JSONValueHandle *obj, const char *key) {
    if (!obj || !key)
        return false;
    return json_object_object_get_ex(obj, key, NULL);
}

size_t asthra_stdlib_json_object_size(JSONValueHandle *obj) {
    if (!obj || !json_object_is_type(obj, json_type_object))
        return 0;
    return (size_t)json_object_object_length(obj);
}

// Array operations for Asthra FFI
bool asthra_stdlib_json_array_push(JSONValueHandle *array, JSONValueHandle *value) {
    if (!array || !json_object_is_type(array, json_type_array))
        return false;
    return json_object_array_add(array, value) == 0;
}

JSONValueHandle *asthra_stdlib_json_array_get(JSONValueHandle *array, size_t index) {
    if (!array || !json_object_is_type(array, json_type_array))
        return NULL;

    json_object *value = json_object_array_get_idx(array, (int)index);
    if (value) {
        // Increment reference count since we're returning it
        json_object_get(value);
        return value;
    }
    return NULL;
}

size_t asthra_stdlib_json_array_length(JSONValueHandle *array) {
    if (!array || !json_object_is_type(array, json_type_array))
        return 0;
    return (int)json_object_array_length(array);
}

// Type checking for Asthra FFI
bool asthra_stdlib_json_is_null(JSONValueHandle *handle) {
    // In json-c, null values are represented differently in different contexts
    if (handle == NULL)
        return true;
    return json_object_is_type(handle, json_type_null);
}

bool asthra_stdlib_json_is_bool(JSONValueHandle *handle) {
    if (!handle)
        return false;
    return json_object_is_type(handle, json_type_boolean);
}

bool asthra_stdlib_json_is_number(JSONValueHandle *handle) {
    if (!handle)
        return false;
    return json_object_is_type(handle, json_type_int) ||
           json_object_is_type(handle, json_type_double);
}

bool asthra_stdlib_json_is_string(JSONValueHandle *handle) {
    if (!handle)
        return false;
    return json_object_is_type(handle, json_type_string);
}

bool asthra_stdlib_json_is_array(JSONValueHandle *handle) {
    if (!handle)
        return false;
    return json_object_is_type(handle, json_type_array);
}

bool asthra_stdlib_json_is_object(JSONValueHandle *handle) {
    if (!handle)
        return false;
    return json_object_is_type(handle, json_type_object);
}

// Value extraction for Asthra FFI
bool asthra_stdlib_json_get_bool(JSONValueHandle *handle) {
    if (!handle || !json_object_is_type(handle, json_type_boolean))
        return false;
    return json_object_get_boolean(handle);
}

double asthra_stdlib_json_get_number(JSONValueHandle *handle) {
    if (!handle)
        return 0.0;

    if (json_object_is_type(handle, json_type_int)) {
        return (double)json_object_get_int64(handle);
    } else if (json_object_is_type(handle, json_type_double)) {
        return json_object_get_double(handle);
    }
    return 0.0;
}

const char *asthra_stdlib_json_get_string(JSONValueHandle *handle) {
    if (!handle || !json_object_is_type(handle, json_type_string))
        return NULL;
    return json_object_get_string(handle);
}

// Validation for Asthra FFI
bool asthra_stdlib_json_is_valid(const char *input) {
    return asthra_json_is_valid(input);
}
