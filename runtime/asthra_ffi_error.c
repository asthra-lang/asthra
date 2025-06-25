/**
 * Asthra FFI Error Handling Implementation v1.0
 * Mapping C error codes (primarily errno values) to Asthra's Result types
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "asthra_ffi_error.h"
#include "asthra_runtime.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// String creation helper function
static AsthraCFFIString *asthra_cstring_create(const char *cstr) {
    if (!cstr) {
        cstr = "";
    }

    size_t len = strlen(cstr);
    AsthraCFFIString *str = asthra_alloc(sizeof(AsthraCFFIString), ASTHRA_ZONE_GC);
    if (!str) {
        return NULL;
    }

    str->data = asthra_alloc(len + 1, ASTHRA_ZONE_GC);
    if (!str->data) {
        asthra_free(str, ASTHRA_ZONE_GC);
        return NULL;
    }

    memcpy(str->data, cstr, len);
    str->data[len] = '\0';
    str->len = len;
    str->cap = len + 1;

    return str;
}

// String free helper function
static void asthra_cstring_free(AsthraCFFIString *str) {
    if (str) {
        if (str->data) {
            asthra_free(str->data, ASTHRA_ZONE_GC);
        }
        asthra_free(str, ASTHRA_ZONE_GC);
    }
}

AsthraCFFIError *asthra_ffi_error_create(int32_t code, const char *subsystem) {
    AsthraCFFIError *error = asthra_alloc(sizeof(AsthraCFFIError), ASTHRA_ZONE_GC);
    if (!error) {
        return NULL;
    }

    error->code = code;
    error->subsystem = asthra_cstring_create(subsystem ? subsystem : "unknown");

    // Get error message from strerror
    const char *c_message = strerror(code);
    error->message = asthra_cstring_create(c_message ? c_message : "Unknown error");

    // Default values for source location
    error->source_file = asthra_cstring_create("unknown");
    error->line = 0;

    return error;
}

void asthra_ffi_error_free(AsthraCFFIError *error) {
    if (error) {
        asthra_cstring_free(error->subsystem);
        asthra_cstring_free(error->message);
        asthra_cstring_free(error->source_file);
        asthra_free(error, ASTHRA_ZONE_GC);
    }
}

AsthraCFFIError *asthra_ffi_error_from_errno(const char *subsystem) {
    int err = errno;
    return asthra_ffi_error_create(err, subsystem ? subsystem : "libc");
}

AsthraCFFIError *asthra_ffi_error_create_with_location(int32_t code, const char *subsystem,
                                                       const char *file, int32_t line) {
    AsthraCFFIError *error = asthra_ffi_error_create(code, subsystem);
    if (error) {
        asthra_ffi_error_set_location(error, file, line);
    }
    return error;
}

void asthra_ffi_error_set_location(AsthraCFFIError *error, const char *file, int32_t line) {
    if (error) {
        if (error->source_file) {
            asthra_cstring_free(error->source_file);
        }
        error->source_file = asthra_cstring_create(file ? file : "unknown");
        error->line = line;
    }
}

const char *asthra_ffi_error_get_message(AsthraCFFIError *error) {
    if (!error || !error->message || !error->message->data) {
        return "Unknown error";
    }
    return error->message->data;
}

int32_t asthra_ffi_error_get_code(AsthraCFFIError *error) {
    return error ? error->code : 0;
}

uint32_t asthra_ffi_error_register_type(void) {
    // Register the FFIError type with the runtime
    return asthra_register_type("FFIError", sizeof(AsthraCFFIError),
                                (void (*)(void *))asthra_ffi_error_free);
}

void *asthra_ffi_error_create_err_value(AsthraCFFIError *error) {
    // Create a copy of the error for the Result.Err value
    // This allows the original error to be freed independently
    if (!error) {
        return NULL;
    }

    AsthraCFFIError *err_value =
        asthra_ffi_error_create(error->code, error->subsystem ? error->subsystem->data : "unknown");

    if (err_value) {
        // Copy the source location
        asthra_ffi_error_set_location(
            err_value, error->source_file ? error->source_file->data : "unknown", error->line);
    }

    return err_value;
}

bool asthra_ffi_error_is_temporary(int32_t code) {
    // Check for temporary/transient errors
    return (code == EAGAIN) || (code == EWOULDBLOCK) || (code == EINTR);
}

bool asthra_ffi_error_is_permission_denied(int32_t code) {
    return (code == EACCES) || (code == EPERM);
}

bool asthra_ffi_error_is_not_found(int32_t code) {
    return code == ENOENT;
}

size_t asthra_ffi_error_to_string(AsthraCFFIError *error, char *buffer, size_t buffer_size) {
    if (!error || !buffer || buffer_size == 0) {
        return 0;
    }

    const char *subsystem_str =
        error->subsystem && error->subsystem->data ? error->subsystem->data : "unknown";
    const char *message_str =
        error->message && error->message->data ? error->message->data : "Unknown error";
    const char *file_str =
        error->source_file && error->source_file->data ? error->source_file->data : "unknown";

    size_t result =
        (size_t)snprintf(buffer, buffer_size, "FFI error (%s): [%d] %s at %s:%d", subsystem_str,
                         error->code, message_str, file_str, error->line);

    return result;
}

// Builtin function handlers for Asthra language integration

AsthraCFFIError *asthra_builtin_from_errno(const char *subsystem) {
    AsthraCFFIError *error = asthra_ffi_error_from_errno(subsystem);

    // Get caller location from frame (this would need to be provided by the runtime)
    // For now, we'll just use default values
    error->source_file = asthra_cstring_create("unknown");
    error->line = 0;

    return error;
}

AsthraCFFIError *asthra_builtin_from_error_code(int32_t code, const char *subsystem) {
    AsthraCFFIError *error = asthra_ffi_error_create(code, subsystem);

    // Get caller location from frame
    // For now, we'll just use default values
    error->source_file = asthra_cstring_create("unknown");
    error->line = 0;

    return error;
}
