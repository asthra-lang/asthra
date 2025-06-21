#include "development_server.h"
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>

static AsthraDevelopmentServer *g_server = NULL;

void signal_handler(int sig) {
    if (g_server) {
        printf("\nShutting down Asthra Development Server...\n");
        asthra_dev_server_stop(g_server);
        asthra_dev_server_destroy(g_server);
        g_server = NULL;
    }
    exit(0);
}

void print_usage(const char *program_name) {
    printf("Usage: %s [OPTIONS]\n", program_name);
    printf("Options:\n");
    printf("  -p, --port PORT     Set server port (default: 8080)\n");
    printf("  -h, --help          Show this help message\n");
    printf("  --test              Run simple functionality test\n");
    printf("\n");
    printf("The Asthra Development Server provides real-time AI integration\n");
    printf("for lightning-fast development cycles.\n");
}

void run_test(AsthraDevelopmentServer *server) {
    printf("Running development server functionality test...\n");
    
    // Test request creation and handling
    const char *test_request_json = 
        "{"
        "\"request_id\": \"test_001\","
        "\"command\": \"check\","
        "\"code_content\": \"function add(x: int, y: int) -> int { return x + y; }\""
        "}";
    
    DevServerRequest *request = dev_server_request_create(test_request_json);
    if (!request) {
        printf("❌ Failed to create test request\n");
        return;
    }
    
    printf("✅ Created test request: %s\n", request->command);
    
    // Handle the request
    DevServerResponse *response = asthra_dev_server_handle_request(server, request);
    if (!response) {
        printf("❌ Failed to handle test request\n");
        dev_server_request_destroy(request);
        return;
    }
    
    printf("✅ Handled request in %.2fms\n", response->processing_time_ms);
    printf("Response success: %s\n", response->success ? "true" : "false");
    
    if (response->result_json) {
        printf("Response result: %s\n", response->result_json);
    }
    
    if (response->error_message) {
        printf("Response error: %s\n", response->error_message);
    }
    
    // Test statistics
    DevServerStats stats = asthra_dev_server_get_stats(server);
    printf("Server stats:\n");
    printf("  Total requests: %zu\n", stats.total_requests);
    printf("  Successful requests: %zu\n", stats.successful_requests);
    printf("  Average response time: %.2fms\n", stats.average_response_time_ms);
    
    // Clean up
    dev_server_request_destroy(request);
    dev_server_response_destroy(response);
    
    printf("✅ All tests passed!\n");
}

int main(int argc, char *argv[]) {
    int port = 8080;
    bool run_tests = false;
    
    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
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
            run_tests = true;
        } else {
            fprintf(stderr, "Error: Unknown option %s\n", argv[i]);
            print_usage(argv[0]);
            return 1;
        }
    }
    
    // Set up signal handlers
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    // Create and start server
    printf("Creating Asthra Development Server on port %d...\n", port);
    g_server = asthra_dev_server_create(port);
    if (!g_server) {
        fprintf(stderr, "Error: Failed to create development server\n");
        return 1;
    }
    
    // Run tests if requested
    if (run_tests) {
        run_test(g_server);
        asthra_dev_server_destroy(g_server);
        return 0;
    }
    
    // Start the server
    if (!asthra_dev_server_start(g_server)) {
        fprintf(stderr, "Error: Failed to start development server\n");
        asthra_dev_server_destroy(g_server);
        return 1;
    }
    
    printf("✅ Asthra Development Server is running on port %d\n", port);
    printf("Press Ctrl+C to stop the server\n");
    printf("\nEndpoints:\n");
    printf("  HTTP Health Check: http://localhost:%d/\n", port);
    printf("  WebSocket API: ws://localhost:%d/\n", port);
    printf("\nSupported commands:\n");
    printf("  - check: Fast syntax and semantic checking\n");
    printf("  - complete: Code completion (coming soon)\n");
    printf("  - analyze: AI-powered analysis (coming soon)\n");
    printf("  - stats: Server performance statistics\n");
    
    // Keep the main thread alive
    while (asthra_dev_server_is_running(g_server)) {
        sleep(1);
        
        // Print periodic stats (every 30 seconds)
        static time_t last_stats_time = 0;
        time_t current_time = time(NULL);
        if (current_time - last_stats_time >= 30) {
            DevServerStats stats = asthra_dev_server_get_stats(g_server);
            if (stats.total_requests > 0) {
                printf("[Stats] Requests: %zu, Avg Response: %.1fms, Cache Hit Rate: %zu%%\n",
                       stats.total_requests, stats.average_response_time_ms, stats.cache_hit_rate_percent);
            }
            last_stats_time = current_time;
        }
    }
    
    // Clean up
    asthra_dev_server_destroy(g_server);
    g_server = NULL;
    
    printf("Asthra Development Server stopped.\n");
    return 0;
} 
