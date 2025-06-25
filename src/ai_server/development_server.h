#ifndef DEVELOPMENT_SERVER_H
#define DEVELOPMENT_SERVER_H

#include "../fast_check/fast_check_engine.h"
#include <pthread.h>
#include <stdbool.h>
#include <stddef.h>
#include <time.h>

// Forward declarations for stub implementation
typedef struct AsthraDevelopmentServer AsthraDevelopmentServer;
typedef struct DevServerRequest DevServerRequest;
typedef struct DevServerResponse DevServerResponse;
typedef struct DevServerConnection DevServerConnection;
typedef struct ClientRateLimit ClientRateLimit;

// Connection structure
struct DevServerConnection {
    char *client_id;
    int socket_fd;
    bool is_active;
    time_t last_activity;
    // For WebSocket connection
    char *websocket_key;
    bool is_subscribed_for_analysis;   // Flag for real-time analysis subscription
    time_t last_analysis_request_time; // For debouncing analysis requests
    char *subscribed_file_path;        // The file path this connection is subscribed to
    // FFI memory tracking
    size_t ffi_allocated_bytes;    // Bytes allocated via FFI by this connection
    size_t ffi_freed_bytes;        // Bytes freed via FFI by this connection
    bool ffi_memory_leak_detected; // Flag to indicate a leak
};

// Rate limiting structure per client
struct ClientRateLimit {
    char *client_id;         // Unique identifier for the client
    double tokens;           // Current number of tokens in the bucket
    time_t last_refill_time; // Last time tokens were refilled
    double bucket_capacity;  // Maximum tokens the bucket can hold
    double refill_rate;      // Rate at which tokens are refilled (tokens per second)
    pthread_mutex_t mutex;   // Mutex to protect this client's rate limit state
};

// Stats structure (moved before struct definitions)
typedef struct {
    size_t total_requests;
    size_t successful_requests;
    size_t failed_requests; // Add failed requests
    double average_response_time_ms;
    size_t active_connections;
    size_t cache_hit_rate_percent; // Add cache hit rate
} DevServerStats;

// Struct definitions
struct AsthraDevelopmentServer {
    int port;
    bool is_running;
    bool should_stop;
    DevServerStats stats;

    // Fast check engine (forward declaration stub)
    FastCheckEngine *fast_check;

    // Connection management
    DevServerConnection *connections;
    size_t connection_count;
    size_t connection_capacity;
    pthread_mutex_t connection_mutex;
    pthread_t server_thread; // Server thread

    // Rate limiting management
    ClientRateLimit **client_rate_limits; // Array of pointers to client rate limits
    size_t client_rate_limit_count;
    size_t client_rate_limit_capacity;
    pthread_mutex_t client_rate_limit_mutex; // Mutex for the client rate limits array

    // Performance metrics
    size_t total_requests;
    double total_response_time;
    double average_response_time;
    size_t error_count;

    // Rate limiting
    size_t max_requests_per_minute;
    time_t rate_limit_window_start;
    size_t current_window_requests;
};

struct DevServerRequest {
    char *command;
    char *request_id;
    char *file_path;
    char *code_content;
    int line;               // Line number for context
    int column;             // Column number for context
    char *client_id;        // Client ID for tracking
    char **parameters;      // For array parameters in JSON
    size_t parameter_count; // Count of parameters
    time_t timestamp;       // Timestamp of request
};

struct DevServerResponse {
    bool success;
    char *request_id;
    double processing_time_ms;
    char *result_json;
    char *error_message;
    time_t timestamp;     // Timestamp of response
    size_t response_size; // Size of response in bytes
};

// Note: DevServerStats is now defined above the struct definitions

// Fast check engine function declarations are now provided by fast_check_engine.h

// Server functions
AsthraDevelopmentServer *asthra_dev_server_create(int port);
void asthra_dev_server_destroy(AsthraDevelopmentServer *server);
bool asthra_dev_server_start(AsthraDevelopmentServer *server);
void asthra_dev_server_stop(AsthraDevelopmentServer *server);
bool asthra_dev_server_is_running(const AsthraDevelopmentServer *server);          // Added const
DevServerStats asthra_dev_server_get_stats(const AsthraDevelopmentServer *server); // Added const
void asthra_dev_server_reset_stats(AsthraDevelopmentServer *server);               // New function
char *asthra_dev_server_stats_to_json(const DevServerStats *stats);                // New function

// Request/Response handling
DevServerRequest *dev_server_request_create(const char *json);
void dev_server_request_destroy(DevServerRequest *request);
DevServerResponse *asthra_dev_server_handle_request(AsthraDevelopmentServer *server,
                                                    DevServerRequest *request);
DevServerResponse *dev_server_response_create(const char *request_id, bool success);
void dev_server_response_destroy(DevServerResponse *response);
char *dev_server_response_to_json(const DevServerResponse *response);

// Connection management functions
void asthra_dev_server_add_connection(AsthraDevelopmentServer *server, int client_fd,
                                      const char *client_id,
                                      const char *websocket_key); // New function
void asthra_dev_server_remove_connection(AsthraDevelopmentServer *server,
                                         int client_fd); // New function
void asthra_dev_server_broadcast_notification(
    AsthraDevelopmentServer *server,
    const char *notification_json); // Existing, but moved for organization

// Rate limiting functions
ClientRateLimit *asthra_dev_server_get_or_create_rate_limit(AsthraDevelopmentServer *server,
                                                            const char *client_id, double capacity,
                                                            double refill_rate);       // New
bool asthra_dev_server_check_rate_limit(ClientRateLimit *limit, double tokens_needed); // New
void asthra_dev_server_destroy_client_rate_limit(ClientRateLimit *limit);              // New

// Error Codes
typedef enum {
    DEV_SERVER_OK = 0,
    DEV_SERVER_ERROR_INVALID_REQUEST,
    DEV_SERVER_ERROR_FILE_NOT_FOUND,
    DEV_SERVER_ERROR_PARSE_ERROR,
    DEV_SERVER_ERROR_TIMEOUT,
    DEV_SERVER_ERROR_SERVER_ERROR,
    DEV_SERVER_ERROR_RATE_LIMITED,
    DEV_SERVER_ERROR_CONNECTION_CLOSED,
    DEV_SERVER_ERROR_INVALID_PARAMS
} DevServerErrorCode;

const char *dev_server_error_to_string(DevServerErrorCode error); // New function

#endif // DEVELOPMENT_SERVER_H
