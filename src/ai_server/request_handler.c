#include "request_handler.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <json-c/json_object.h>
#include <json-c/json_tokener.h>

// ============================================================================
// Request/Response Utilities
// ============================================================================

// Simple JSON value extraction (basic implementation)
static char *extract_json_string(const char *json, const char *key) {
    char search_pattern[256];
    snprintf(search_pattern, (size_t)sizeof(search_pattern), "\"%s\":", key);
    
    char *start = strstr(json, search_pattern);
    if (!start) return NULL;
    
    start = strchr(start, ':');
    if (!start) return NULL;
    start++;
    
    // Skip whitespace
    while (*start == ' ' || *start == '\t') start++;
    
    if (*start != '"') return NULL;
    start++; // Skip opening quote
    
    char *end = strchr(start, '"');
    if (!end) return NULL;
    
    size_t len = end - start;
    char *result = malloc(len + 1);
    if (!result) return NULL;
    
    strncpy(result, start, len);
    result[len] = '\0';
    return result;
}

static int extract_json_int(const char *json, const char *key) {
    char search_pattern[256];
    snprintf(search_pattern, (size_t)sizeof(search_pattern), "\"%s\":", key);
    
    char *start = strstr(json, search_pattern);
    if (!start) return 0;
    
    start = strchr(start, ':');
    if (!start) return 0;
    start++;
    
    return atoi(start);
}

DevServerRequest *dev_server_request_create(const char *request_json) {
    if (!request_json) return NULL;

    DevServerRequest *request = calloc(1, sizeof(DevServerRequest));
    if (!request) return NULL;

    json_object *parsed_json = json_tokener_parse(request_json);
    if (!parsed_json) {
        fprintf(stderr, "Error: Failed to parse JSON request: %s\n", request_json);
        free(request);
        return NULL;
    }

    json_object *command_obj;
    if (json_object_object_get_ex(parsed_json, "command", &command_obj)) {
        request->command = strdup(json_object_get_string(command_obj));
    } else {
        request->command = strdup("unknown");
    }

    json_object *request_id_obj;
    if (json_object_object_get_ex(parsed_json, "request_id", &request_id_obj)) {
        request->request_id = strdup(json_object_get_string(request_id_obj));
    } else {
        request->request_id = strdup("default_id");
    }

    json_object *file_path_obj;
    if (json_object_object_get_ex(parsed_json, "file_path", &file_path_obj)) {
        request->file_path = strdup(json_object_get_string(file_path_obj));
    }

    json_object *code_content_obj;
    if (json_object_object_get_ex(parsed_json, "code_content", &code_content_obj)) {
        request->code_content = strdup(json_object_get_string(code_content_obj));
    }

    json_object *line_obj;
    if (json_object_object_get_ex(parsed_json, "line", &line_obj)) {
        request->line = json_object_get_int(line_obj);
    }

    json_object *column_obj;
    if (json_object_object_get_ex(parsed_json, "column", &column_obj)) {
        request->column = json_object_get_int(column_obj);
    }

    json_object *client_id_obj;
    if (json_object_object_get_ex(parsed_json, "client_id", &client_id_obj)) {
        request->client_id = strdup(json_object_get_string(client_id_obj));
    }

    json_object_put(parsed_json); // Free the json_object

    request->timestamp = time(NULL);

    return request;
}

void dev_server_request_destroy(DevServerRequest *request) {
    if (!request) return;
    
    free(request->request_id);
    free(request->command);
    free(request->file_path);
    free(request->code_content);
    free(request->client_id);
    
    if (request->parameters) {
        for (size_t i = 0; i < request->parameter_count; i++) {
            free(request->parameters[i]);
        }
        free(request->parameters);
    }
    
    free(request);
}

// Note: dev_server_response_create is implemented in request_processing.c

void dev_server_response_destroy(DevServerResponse *response) {
    if (!response) return;
    
    free(response->request_id);
    free(response->result_json);
    free(response->error_message);
    free(response);
}

char *dev_server_response_to_json(const DevServerResponse *response) {
    if (!response) return NULL;

    json_object *json_response = json_object_new_object();
    if (!json_response) return NULL;

    if (response->request_id) {
        json_object_object_add(json_response, "request_id", json_object_new_string(response->request_id));
    } else {
        json_object_object_add(json_response, "request_id", json_object_new_string("unknown"));
    }
    json_object_object_add(json_response, "success", json_object_new_boolean(response->success));
    if (response->error_message) {
        json_object_object_add(json_response, "error_message", json_object_new_string(response->error_message));
    }
    json_object_object_add(json_response, "processing_time_ms", json_object_new_double(response->processing_time_ms));

    // Handle result_json which might be a JSON string itself or a plain string
    if (response->result_json) {
        json_object *nested_result = json_tokener_parse(response->result_json);
        if (nested_result) {
            // It's valid JSON, add as a nested object
            json_object_object_add(json_response, "result", nested_result);
        } else {
            // Not valid JSON, treat as a plain string
            json_object_object_add(json_response, "result", json_object_new_string(response->result_json));
        }
    }

    json_object_object_add(json_response, "timestamp", json_object_new_int64(response->timestamp));
    json_object_object_add(json_response, "response_size", json_object_new_int(response->response_size));

    const char *json_string = json_object_to_json_string_ext(json_response, JSON_C_TO_STRING_PRETTY);
    char *result = NULL;
    if (json_string) {
        result = strdup(json_string);
    }
    json_object_put(json_response); // Free the json_object

    return result;
}

// ============================================================================
// Performance Monitoring
// ============================================================================

DevServerStats asthra_dev_server_get_stats(const AsthraDevelopmentServer *server) {
    DevServerStats stats = {0};
    
    if (!server) return stats;
    
    stats.total_requests = server->total_requests;
    stats.successful_requests = (int)server->total_requests - server->error_count;
    stats.failed_requests = (int)server->error_count;
    stats.average_response_time_ms = server->average_response_time;
    stats.active_connections = (int)server->connection_count;
    
    // Get cache hit rate from fast check engine
    if (server->fast_check) {
        FastCheckEngineStats fc_stats = fast_check_engine_get_stats(server->fast_check);
        if (fc_stats.total_checks > 0) {
            stats.cache_hit_rate_percent = (fc_stats.cache_hits * 100) / fc_stats.total_checks;
        }
    }
    
    return stats;
}

void asthra_dev_server_reset_stats(AsthraDevelopmentServer *server) {
    if (!server) return;
    
    server->total_requests = 0;
    server->total_response_time = 0.0;
    server->average_response_time = 0.0;
    server->error_count = 0;
    
    if (server->fast_check) {
        // Reset stats is not available in the current API
        // fast_check_engine_reset_stats(server->fast_check);
    }
}

char *asthra_dev_server_stats_to_json(const DevServerStats *stats) {
    if (!stats) return NULL;
    
    // Use json-c library for proper JSON generation
    json_object *stats_obj = json_object_new_object();
    
    json_object *total_requests = json_object_new_int64(stats->total_requests);
    json_object *successful_requests = json_object_new_int64(stats->successful_requests);
    json_object *failed_requests = json_object_new_int64(stats->failed_requests);
    json_object *avg_response_time = json_object_new_double(stats->average_response_time_ms);
    json_object *active_connections = json_object_new_int64(stats->active_connections);
    json_object *cache_hit_rate = json_object_new_int64(stats->cache_hit_rate_percent);
    
    json_object_object_add(stats_obj, "total_requests", total_requests);
    json_object_object_add(stats_obj, "successful_requests", successful_requests);
    json_object_object_add(stats_obj, "failed_requests", failed_requests);
    json_object_object_add(stats_obj, "average_response_time_ms", avg_response_time);
    json_object_object_add(stats_obj, "active_connections", active_connections);
    json_object_object_add(stats_obj, "cache_hit_rate_percent", cache_hit_rate);
    
    const char *json_string = json_object_to_json_string_ext(stats_obj, JSON_C_TO_STRING_PLAIN);
    char *result = strdup(json_string);
    
    json_object_put(stats_obj);
    return result;
}

// ============================================================================
// Error Handling
// ============================================================================

const char *dev_server_error_to_string(DevServerErrorCode error) {
    switch (error) {
        case DEV_SERVER_OK: return "Success";
        case DEV_SERVER_ERROR_INVALID_REQUEST: return "Invalid request";
        case DEV_SERVER_ERROR_FILE_NOT_FOUND: return "File not found";
        case DEV_SERVER_ERROR_PARSE_ERROR: return "Parse error";
        case DEV_SERVER_ERROR_TIMEOUT: return "Request timeout";
        case DEV_SERVER_ERROR_SERVER_ERROR: return "Internal server error";
        case DEV_SERVER_ERROR_RATE_LIMITED: return "Rate limit exceeded";
        case DEV_SERVER_ERROR_CONNECTION_CLOSED: return "Connection closed";
        case DEV_SERVER_ERROR_INVALID_PARAMS: return "Invalid parameters";
        default: return "Unknown error";
    }
}