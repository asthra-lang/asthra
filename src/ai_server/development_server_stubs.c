#include "../fast_check/fast_check_engine.h"
#include "development_server.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Fast check engine functions are now provided by the real implementation
// (removed duplicate stub implementations)

// Struct definitions are now in the header file

// Server functions (stub implementations)
AsthraDevelopmentServer *asthra_dev_server_create(int port) {
    AsthraDevelopmentServer *server = calloc(1, sizeof(AsthraDevelopmentServer));
    if (!server)
        return NULL;

    server->port = port;
    server->is_running = false;
    server->should_stop = false;

    // Initialize fast check engine
    server->fast_check = fast_check_engine_create();

    // Initialize connection management (stub - no actual connections)
    server->connections = NULL;
    server->connection_count = 0;
    server->connection_capacity = 0;

    // Initialize mutex (stub)
    if (pthread_mutex_init(&server->connection_mutex, NULL) != 0) {
        fast_check_engine_destroy(server->fast_check);
        free(server);
        return NULL;
    }

    // Initialize performance metrics
    server->total_requests = 0;
    server->total_response_time = 0.0;
    server->average_response_time = 0.0;
    server->error_count = 0;
    server->stats.successful_requests = 0; // Initialize new fields
    server->stats.failed_requests = 0;
    server->stats.active_connections = 0;
    server->stats.cache_hit_rate_percent = 0;

    // Initialize rate limiting
    server->max_requests_per_minute = 1000;
    server->rate_limit_window_start = time(NULL);
    server->current_window_requests = 0;

    return server;
}

void asthra_dev_server_destroy(AsthraDevelopmentServer *server) {
    if (!server)
        return;

    // Stop server if running
    if (server->is_running) {
        asthra_dev_server_stop(server);
    }

    // Clean up connections (stub - no actual connections to clean)
    if (server->connections) {
        free(server->connections);
    }

    // Destroy mutex
    pthread_mutex_destroy(&server->connection_mutex);

    // Destroy fast check engine
    if (server->fast_check) {
        fast_check_engine_destroy(server->fast_check);
    }

    free(server);
}

bool asthra_dev_server_start(AsthraDevelopmentServer *server) {
    if (!server)
        return false;
    server->is_running = true;
    return true;
}

void asthra_dev_server_stop(AsthraDevelopmentServer *server) {
    if (!server)
        return;
    server->is_running = false;
    server->should_stop = true;
}

bool asthra_dev_server_is_running(const AsthraDevelopmentServer *server) {
    return server ? server->is_running : false;
}

DevServerStats asthra_dev_server_get_stats(const AsthraDevelopmentServer *server) {
    if (!server) {
        DevServerStats empty = {0};
        return empty;
    }
    return server->stats;
}

void asthra_dev_server_reset_stats(AsthraDevelopmentServer *server) {
    if (!server)
        return;

    server->total_requests = 0;
    server->total_response_time = 0.0;
    server->average_response_time = 0.0;
    server->error_count = 0;
    server->stats.successful_requests = 0;
    server->stats.failed_requests = 0;
    server->stats.cache_hit_rate_percent = 0;
}

char *asthra_dev_server_stats_to_json(const DevServerStats *stats) {
    if (!stats)
        return NULL;

    // Simple JSON stub (not using cJSON here as it's a stub file)
    char *result = malloc(256);
    if (!result)
        return NULL;
    snprintf(result, 256, "{\"total_requests\": %zu, \"active_connections\": %zu}",
             stats->total_requests, stats->active_connections);
    return result;
}

// Request/Response handling (stub implementations)
DevServerRequest *dev_server_request_create(const char *json) {
    // This function is now fully implemented in development_server.c using cJSON
    // This stub version returns a minimal request for compilation compatibility.
    DevServerRequest *request = calloc(1, sizeof(DevServerRequest));
    if (!request)
        return NULL;
    request->command = strdup("stub_command");
    request->request_id = strdup("stub_id");
    request->file_path = NULL;
    request->code_content = NULL;
    request->line = 0;
    request->column = 0;
    request->client_id = NULL;
    request->parameters = NULL;
    request->parameter_count = 0;
    request->timestamp = time(NULL);
    return request;
}

void dev_server_request_destroy(DevServerRequest *request) {
    if (!request)
        return;
    free(request->command);
    free(request->request_id);
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

DevServerResponse *asthra_dev_server_handle_request(AsthraDevelopmentServer *server,
                                                    DevServerRequest *request) {
    DevServerResponse *response = calloc(1, sizeof(DevServerResponse));
    if (!response)
        return NULL;
    response->request_id = strdup(request ? request->request_id : "stub_id");
    response->success = true;
    response->processing_time_ms = 0.0;
    response->result_json = strdup("{\"status\":\"ok\"}");
    response->error_message = NULL;
    response->timestamp = time(NULL);
    response->response_size = 0;
    return response;
}

void dev_server_response_destroy(DevServerResponse *response) {
    if (!response)
        return;
    free(response->request_id);
    free(response->result_json);
    free(response->error_message);
    free(response);
}

char *dev_server_response_to_json(const DevServerResponse *response) {
    // This function is now fully implemented in development_server.c using cJSON
    // This stub version returns a minimal JSON string for compilation compatibility.
    if (!response)
        return NULL;
    char *result = malloc(256);
    if (!result)
        return NULL;
    snprintf(result, 256, "{\"request_id\":\"%s\",\"success\":%s}",
             response->request_id ? response->request_id : "",
             response->success ? "true" : "false");
    return result;
}

// Connection management (stub implementations)
void asthra_dev_server_add_connection(AsthraDevelopmentServer *server, int client_fd,
                                      const char *client_id, const char *websocket_key) {
    (void)server;
    (void)client_fd;
    (void)client_id;
    (void)websocket_key;
    fprintf(stdout, "STUB: Added connection %s\n", client_id);
}

void asthra_dev_server_remove_connection(AsthraDevelopmentServer *server, int client_fd) {
    (void)server;
    (void)client_fd;
    fprintf(stdout, "STUB: Removed connection %d\n", client_fd);
}

void asthra_dev_server_broadcast_notification(AsthraDevelopmentServer *server,
                                              const char *notification_json) {
    (void)server;
    (void)notification_json;
    fprintf(stdout, "STUB: Broadcast notification: %s\n", notification_json);
}

// Error Codes (stub implementation)
const char *dev_server_error_to_string(DevServerErrorCode error) {
    switch (error) {
    case DEV_SERVER_OK:
        return "STUB_Success";
    default:
        return "STUB_Unknown_Error";
    }
}
