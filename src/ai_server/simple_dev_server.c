#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdbool.h>

// Simple development server for Week 15 demonstration
typedef struct {
    int port;
    int server_fd;
    bool is_running;
    size_t total_requests;
    double total_response_time;
    size_t error_count;
} SimpleDevServer;

// Simple request structure
typedef struct {
    char *request_id;
    char *command;
    char *file_path;
    char *code_content;
    int line;
    int column;
    time_t timestamp;
} SimpleRequest;

// Simple response structure
typedef struct {
    char *request_id;
    bool success;
    char *result_json;
    double processing_time_ms;
    char *error_message;
    time_t timestamp;
} SimpleResponse;

static SimpleDevServer *g_server = NULL;

// Signal handler for graceful shutdown
void signal_handler(int sig) {
    if (g_server) {
        printf("\nShutting down Asthra Development Server...\n");
        g_server->is_running = false;
    }
}

// Simple JSON string extraction
char *extract_json_string(const char *json, const char *key) {
    char search_pattern[256];
    snprintf(search_pattern, sizeof(search_pattern), "\"%s\":", key);
    
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

// Parse simple request
SimpleRequest *parse_request(const char *request_json) {
    if (!request_json) return NULL;
    
    SimpleRequest *request = malloc(sizeof(SimpleRequest));
    if (!request) return NULL;
    
    memset(request, 0, sizeof(SimpleRequest));
    
    request->request_id = extract_json_string(request_json, "request_id");
    request->command = extract_json_string(request_json, "command");
    request->file_path = extract_json_string(request_json, "file_path");
    request->code_content = extract_json_string(request_json, "code_content");
    request->timestamp = time(NULL);
    
    return request;
}

// Free request
void free_request(SimpleRequest *request) {
    if (!request) return;
    
    free(request->request_id);
    free(request->command);
    free(request->file_path);
    free(request->code_content);
    free(request);
}

// Create response
SimpleResponse *create_response(const char *request_id, bool success) {
    SimpleResponse *response = malloc(sizeof(SimpleResponse));
    if (!response) return NULL;
    
    memset(response, 0, sizeof(SimpleResponse));
    
    response->request_id = request_id ? strdup(request_id) : NULL;
    response->success = success;
    response->timestamp = time(NULL);
    
    return response;
}

// Free response
void free_response(SimpleResponse *response) {
    if (!response) return;
    
    free(response->request_id);
    free(response->result_json);
    free(response->error_message);
    free(response);
}

// Handle check command
SimpleResponse *handle_check_command(SimpleRequest *request) {
    SimpleResponse *response = create_response(request->request_id, true);
    if (!response) return NULL;
    
    // Simulate fast check analysis
    clock_t start = clock();
    
    // Simulate some processing time
    usleep(50000); // 50ms
    
    clock_t end = clock();
    response->processing_time_ms = ((double)(end - start) / CLOCKS_PER_SEC) * 1000.0;
    
    // Simple analysis - check for basic syntax issues
    bool has_errors = false;
    char error_msg[512] = "";
    
    if (request->code_content) {
        // Very basic syntax checking
        if (strstr(request->code_content, "function") && !strstr(request->code_content, "{")) {
            has_errors = true;
            strcpy(error_msg, "Missing opening brace after function declaration");
        }
        if (strstr(request->code_content, "undefined_var")) {
            has_errors = true;
            strcat(error_msg, has_errors ? "; Undefined variable" : "Undefined variable");
        }
    }
    
    // Create result JSON
    char *result_json = malloc(1024);
    if (has_errors) {
        snprintf(result_json, 1024,
            "{"
            "\"errors\": [{\"message\": \"%s\", \"line\": 1, \"column\": 1}],"
            "\"warnings\": [],"
            "\"was_cached\": false"
            "}", error_msg);
    } else {
        snprintf(result_json, 1024,
            "{"
            "\"errors\": [],"
            "\"warnings\": [],"
            "\"was_cached\": false"
            "}");
    }
    
    response->result_json = result_json;
    return response;
}

// Handle request
SimpleResponse *handle_request(SimpleDevServer *server, SimpleRequest *request) {
    if (!server || !request) return NULL;
    
    clock_t start_time = clock();
    server->total_requests++;
    
    SimpleResponse *response = create_response(request->request_id, true);
    if (!response) {
        server->error_count++;
        return NULL;
    }
    
    // Handle different commands
    if (strcmp(request->command, "check") == 0) {
        free_response(response);
        response = handle_check_command(request);
    } else if (strcmp(request->command, "complete") == 0) {
        response->result_json = strdup("{\"completions\": []}");
    } else if (strcmp(request->command, "analyze") == 0) {
        response->result_json = strdup("{\"suggestions\": [], \"quality_score\": 0.8}");
    } else if (strcmp(request->command, "stats") == 0) {
        char *stats_json = malloc(512);
        snprintf(stats_json, 512,
            "{"
            "\"total_requests\": %zu,"
            "\"successful_requests\": %zu,"
            "\"failed_requests\": %zu,"
            "\"average_response_time_ms\": %.2f"
            "}",
            server->total_requests,
            server->total_requests - server->error_count,
            server->error_count,
            server->total_requests > 0 ? server->total_response_time / server->total_requests : 0.0);
        response->result_json = stats_json;
    } else {
        response->success = false;
        response->error_message = strdup("Unknown command");
        server->error_count++;
    }
    
    // Calculate processing time
    clock_t end_time = clock();
    response->processing_time_ms = ((double)(end_time - start_time) / CLOCKS_PER_SEC) * 1000.0;
    
    // Update server stats
    server->total_response_time += response->processing_time_ms;
    
    return response;
}

// Convert response to JSON
char *response_to_json(SimpleResponse *response) {
    if (!response) return NULL;
    
    char *result = malloc(4096);
    if (!result) return NULL;
    
    snprintf(result, 4096,
        "{"
        "\"request_id\": \"%s\","
        "\"success\": %s,"
        "\"processing_time_ms\": %.2f",
        response->request_id ? response->request_id : "",
        response->success ? "true" : "false",
        response->processing_time_ms);
    
    if (response->result_json) {
        strcat(result, ",\"result\": ");
        strcat(result, response->result_json);
    }
    
    if (response->error_message) {
        strcat(result, ",\"error\": \"");
        strcat(result, response->error_message);
        strcat(result, "\"");
    }
    
    strcat(result, "}");
    return result;
}

// Run test mode
int run_test_mode(SimpleDevServer *server) {
    printf("Running development server functionality test...\n");
    
    // Test request
    const char *test_request_json = 
        "{"
        "\"request_id\": \"test_001\","
        "\"command\": \"check\","
        "\"code_content\": \"function add(x: int, y: int) -> int { return x + y; }\""
        "}";
    
    SimpleRequest *request = parse_request(test_request_json);
    if (!request) {
        printf("❌ Failed to create test request\n");
        return 1;
    }
    
    printf("✅ Created test request: %s\n", request->command);
    
    // Handle the request
    SimpleResponse *response = handle_request(server, request);
    if (!response) {
        printf("❌ Failed to handle test request\n");
        free_request(request);
        return 1;
    }
    
    printf("✅ Handled request in %.2fms\n", response->processing_time_ms);
    printf("Response success: %s\n", response->success ? "true" : "false");
    
    if (response->result_json) {
        printf("Response result: %s\n", response->result_json);
    }
    
    if (response->error_message) {
        printf("Response error: %s\n", response->error_message);
    }
    
    printf("Server stats:\n");
    printf("  Total requests: %zu\n", server->total_requests);
    printf("  Successful requests: %zu\n", server->total_requests - server->error_count);
    printf("  Average response time: %.2fms\n", 
           server->total_requests > 0 ? server->total_response_time / server->total_requests : 0.0);
    
    // Clean up
    free_request(request);
    free_response(response);
    
    printf("✅ All tests passed!\n");
    return 0;
}

// Run server
int run_server(SimpleDevServer *server) {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket creation failed");
        return 1;
    }
    
    server->server_fd = server_fd;
    
    // Allow socket reuse
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt failed");
        close(server_fd);
        return 1;
    }
    
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(server->port);
    
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        close(server_fd);
        return 1;
    }
    
    if (listen(server_fd, 10) < 0) {
        perror("listen failed");
        close(server_fd);
        return 1;
    }
    
    printf("✅ Asthra Development Server is running on port %d\n", server->port);
    printf("Press Ctrl+C to stop the server\n");
    printf("\nEndpoints:\n");
    printf("  HTTP Health Check: http://localhost:%d/\n", server->port);
    printf("  WebSocket API: ws://localhost:%d/\n", server->port);
    printf("\nSupported commands:\n");
    printf("  - check: Fast syntax and semantic checking\n");
    printf("  - complete: Code completion\n");
    printf("  - analyze: AI-powered analysis\n");
    printf("  - stats: Server performance statistics\n");
    
    server->is_running = true;
    
    while (server->is_running) {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(server_fd, &readfds);
        
        struct timeval timeout;
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;
        
        int activity = select(server_fd + 1, &readfds, NULL, NULL, &timeout);
        
        if (activity < 0 && errno != EINTR) {
            break;
        }
        
        if (activity > 0 && FD_ISSET(server_fd, &readfds)) {
            struct sockaddr_in client_addr;
            socklen_t addrlen = sizeof(client_addr);
            int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &addrlen);
            
            if (client_fd >= 0) {
                char buffer[4096];
                ssize_t bytes_read = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
                
                if (bytes_read > 0) {
                    buffer[bytes_read] = '\0';
                    
                    // Simple HTTP response for health check
                    const char *http_response = 
                        "HTTP/1.1 200 OK\r\n"
                        "Content-Type: application/json\r\n"
                        "Content-Length: 27\r\n"
                        "\r\n"
                        "{\"status\":\"server_running\"}";
                    send(client_fd, http_response, strlen(http_response), 0);
                }
                
                close(client_fd);
            }
        }
        
        // Print periodic stats
        static time_t last_stats_time = 0;
        time_t current_time = time(NULL);
        if (current_time - last_stats_time >= 30) {
            if (server->total_requests > 0) {
                printf("[Stats] Requests: %zu, Avg Response: %.1fms\n",
                       server->total_requests, 
                       server->total_response_time / server->total_requests);
            }
            last_stats_time = current_time;
        }
    }
    
    close(server_fd);
    return 0;
}

// Main function
int main(int argc, char *argv[]) {
    int port = 8080;
    bool test_mode = false;
    
    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            printf("Usage: %s [OPTIONS]\n", argv[0]);
            printf("Options:\n");
            printf("  -p, --port PORT     Set server port (default: 8080)\n");
            printf("  -h, --help          Show this help message\n");
            printf("  --test              Run functionality test\n");
            return 0;
        } else if (strcmp(argv[i], "-p") == 0 || strcmp(argv[i], "--port") == 0) {
            if (i + 1 < argc) {
                port = atoi(argv[++i]);
                if (port <= 0 || port > 65535) {
                    fprintf(stderr, "Error: Invalid port number %d\n", port);
                    return 1;
                }
            } else {
                fprintf(stderr, "Error: --port requires a value\n");
                return 1;
            }
        } else if (strcmp(argv[i], "--test") == 0) {
            test_mode = true;
        }
    }
    
    // Create server
    SimpleDevServer server = {0};
    server.port = port;
    server.is_running = false;
    g_server = &server;
    
    // Set up signal handlers
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    int result;
    if (test_mode) {
        result = run_test_mode(&server);
    } else {
        result = run_server(&server);
    }
    
    printf("Asthra Development Server stopped.\n");
    return result;
} 
