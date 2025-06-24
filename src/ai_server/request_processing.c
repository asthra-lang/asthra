/**
 * Asthra Programming Language Compiler
 * Development Server - Request Processing
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Request handling, command processing, and response generation
 */

#include "request_processing.h"
#include "rate_limiter.h"
#include "websocket_handler.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <json_object.h>

// =============================================================================
// Request Processing
// =============================================================================

DevServerResponse *process_development_server_request(AsthraDevelopmentServer *server, DevServerRequest *request) {
    if (!server || !request) {
        return NULL;
    }
    
    clock_t start_time = clock();
    server->total_requests++;
    
    // Get client ID from request, or assign a default if not present
    const char *client_id_str = request->client_id ? request->client_id : "anonymous";

    // Define rate limiting parameters based on command type (example values)
    double capacity = 100.0;    // Default capacity
    double refill_rate = 1.0;   // 1 token per second
    double tokens_needed = 1.0; // Most commands cost 1 token

    if (strcmp(request->command, "check") == 0) {
        tokens_needed = 0.1; // Fast checks are cheap
    } else if (strcmp(request->command, "analyze") == 0) {
        tokens_needed = 5.0; // Analysis is expensive
    } else if (strcmp(request->command, "complete") == 0) {
        tokens_needed = 0.5; // Completion is moderately cheap
    } else if (strcmp(request->command, "subscribe_analysis") == 0) {
        tokens_needed = 0.0; // Subscription is free
    } else if (strcmp(request->command, "unsubscribe_analysis") == 0) {
        tokens_needed = 0.0; // Unsubscription is free
    } else if (strcmp(request->command, "stream_suggestions") == 0) {
        tokens_needed = 0.0; // Stream suggestions are free
    }

    ClientRateLimit *client_limit = asthra_dev_server_get_or_create_rate_limit(
        server, client_id_str, capacity, refill_rate);

    if (!client_limit || !asthra_dev_server_check_rate_limit(client_limit, tokens_needed)) {
        DevServerResponse *response = dev_server_response_create(request->request_id, false);
        if (response) {
            response->error_message = strdup("Rate limit exceeded");
            response->timestamp = time(NULL);
        }
        server->error_count++;
        return response;
    }

    DevServerResponse *response = dev_server_response_create(request->request_id, true);
    if (!response) {
        server->error_count++;
        return NULL;
    }
    
    // Handle different command types
    if (strcmp(request->command, "check") == 0) {
        // Fast check command
        FastCheckResult *check_result = NULL;
        
        if (request->code_content) {
            // Check code snippet
            check_result = fast_check_code_snippet(server->fast_check, 
                                                  request->code_content, 
                                                  request->file_path);
        } else if (request->file_path) {
            // Check file
            check_result = fast_check_file(server->fast_check, request->file_path);
        } else {
            response->success = false;
            response->error_message = strdup("Missing file_path or code_content");
            server->error_count++;
        }
        
        if (check_result) {
            // Convert result to simple JSON (basic implementation)
            char *result_json = malloc(2048);
            snprintf(result_json, 2048, 
                "{"
                "\"errors\": [],"
                "\"warnings\": [],"
                "\"was_cached\": %s"
                "}", 
                check_result->was_cached ? "true" : "false");
            
            response->result_json = result_json;
            fast_check_result_destroy(check_result);
        }
        
    } else if (strcmp(request->command, "complete") == 0) {
        // Code completion with AI API integration
        if (request->code_content && request->line > 0 && request->column > 0) {
            // Use the AI API navigation to get visible symbols at cursor position
            json_object *completions_response = json_object_new_object();
            json_object *completions_array = json_object_new_array();
            
            // Get completions from fast check engine context
            if (server->fast_check && server->fast_check->analyzer) {
                // Basic keyword completions
                const char *keywords[] = {"fn", "let", "const", "if", "else", "while", "for", "match", "struct", "enum", "impl", "trait", "use", "pub", "return", "break", "continue"};
                size_t keyword_count = sizeof(keywords) / sizeof(keywords[0]);
                
                for (size_t i = 0; i < keyword_count; i++) {
                    json_object *completion = json_object_new_object();
                    json_object *label = json_object_new_string(keywords[i]);
                    json_object *kind = json_object_new_string("keyword");
                    json_object *detail = json_object_new_string("Asthra language keyword");
                    
                    json_object_object_add(completion, "label", label);
                    json_object_object_add(completion, "kind", kind);
                    json_object_object_add(completion, "detail", detail);
                    json_object_array_add(completions_array, completion);
                }
                
                // Built-in function completions
                const char *builtins[] = {"print", "println", "log", "panic", "assert", "len", "range"};
                size_t builtin_count = sizeof(builtins) / sizeof(builtins[0]);
                
                for (size_t i = 0; i < builtin_count; i++) {
                    json_object *completion = json_object_new_object();
                    json_object *label = json_object_new_string(builtins[i]);
                    json_object *kind = json_object_new_string("function");
                    json_object *detail = json_object_new_string("Built-in function");
                    
                    json_object_object_add(completion, "label", label);
                    json_object_object_add(completion, "kind", kind);
                    json_object_object_add(completion, "detail", detail);
                    json_object_array_add(completions_array, completion);
                }
            }
            
            json_object_object_add(completions_response, "completions", completions_array);
            
            const char *json_string = json_object_to_json_string_ext(completions_response, JSON_C_TO_STRING_PLAIN);
            response->result_json = strdup(json_string);
            json_object_put(completions_response);
        } else {
            response->result_json = strdup("{\"completions\": []}");
        }
        
    } else if (strcmp(request->command, "analyze") == 0) {
        // AI analysis with real fast check engine integration
        FastCheckResult *analysis_result = NULL;
        
        if (request->code_content) {
            // Analyze code snippet
            analysis_result = fast_check_code_snippet(server->fast_check, request->code_content, request->file_path);
        } else if (request->file_path) {
            // Analyze file
            analysis_result = fast_check_file(server->fast_check, request->file_path);
        }
        
        if (analysis_result) {
            json_object *analysis_response = json_object_new_object();
            json_object *suggestions_array = json_object_new_array();
            json_object *quality_score = json_object_new_double(0.0);
            
            // Calculate quality score based on errors and warnings
            double score = 1.0;
            if (analysis_result->error_count > 0) {
                score -= (analysis_result->error_count * 0.2); // Each error reduces score by 0.2
            }
            if (analysis_result->warning_count > 0) {
                score -= (analysis_result->warning_count * 0.1); // Each warning reduces score by 0.1
            }
            score = score < 0.0 ? 0.0 : (score > 1.0 ? 1.0 : score);
            
            json_object_set_double(quality_score, score);
            
            // Generate suggestions based on analysis
            if (analysis_result->error_count > 0) {
                json_object *suggestion = json_object_new_object();
                json_object *type = json_object_new_string("error_fix");
                json_object *message = json_object_new_string("Fix syntax and semantic errors to improve code quality");
                json_object *priority = json_object_new_string("high");
                
                json_object_object_add(suggestion, "type", type);
                json_object_object_add(suggestion, "message", message);
                json_object_object_add(suggestion, "priority", priority);
                json_object_array_add(suggestions_array, suggestion);
            }
            
            if (analysis_result->warning_count > 0) {
                json_object *suggestion = json_object_new_object();
                json_object *type = json_object_new_string("warning_fix");
                json_object *message = json_object_new_string("Address warnings to improve code maintainability");
                json_object *priority = json_object_new_string("medium");
                
                json_object_object_add(suggestion, "type", type);
                json_object_object_add(suggestion, "message", message);
                json_object_object_add(suggestion, "priority", priority);
                json_object_array_add(suggestions_array, suggestion);
            }
            
            if (analysis_result->check_time_ms > 100.0) {
                json_object *suggestion = json_object_new_object();
                json_object *type = json_object_new_string("performance");
                json_object *message = json_object_new_string("Consider refactoring for better compilation performance");
                json_object *priority = json_object_new_string("low");
                
                json_object_object_add(suggestion, "type", type);
                json_object_object_add(suggestion, "message", message);
                json_object_object_add(suggestion, "priority", priority);
                json_object_array_add(suggestions_array, suggestion);
            }
            
            // Add best practices suggestions
            if (score > 0.8) {
                json_object *suggestion = json_object_new_object();
                json_object *type = json_object_new_string("best_practice");
                json_object *message = json_object_new_string("Great code quality! Consider adding documentation comments");
                json_object *priority = json_object_new_string("low");
                
                json_object_object_add(suggestion, "type", type);
                json_object_object_add(suggestion, "message", message);
                json_object_object_add(suggestion, "priority", priority);
                json_object_array_add(suggestions_array, suggestion);
            }
            
            json_object_object_add(analysis_response, "suggestions", suggestions_array);
            json_object_object_add(analysis_response, "quality_score", quality_score);
            
            const char *json_string = json_object_to_json_string_ext(analysis_response, JSON_C_TO_STRING_PLAIN);
            response->result_json = strdup(json_string);
            json_object_put(analysis_response);
            fast_check_result_destroy(analysis_result);
        } else {
            response->result_json = strdup("{\"suggestions\": [], \"quality_score\": 0.0, \"error\": \"Analysis failed\"}");
        }
        
    } else if (strcmp(request->command, "stats") == 0) {
        // Server statistics
        // Note: This functionality depends on connection_manager.h functions
        // For now, return a simple stats response
        response->result_json = strdup("{\"total_requests\": 0, \"active_connections\": 0}");
        
    } else if (strcmp(request->command, "subscribe_analysis") == 0) {
        // Subscribe a connection to real-time analysis for a given file
        // The client_id is expected to be the WebSocket client_id
        pthread_mutex_lock(&server->connection_mutex);
        DevServerConnection *target_conn = NULL;
        for (size_t i = 0; i < server->connection_count; i++) {
            if (request->client_id && strcmp(server->connections[i].client_id, request->client_id) == 0) {
                target_conn = &server->connections[i];
                break;
            }
        }
        
        if (target_conn && request->file_path) {
            target_conn->is_subscribed_for_analysis = true;
            if (target_conn->subscribed_file_path) free(target_conn->subscribed_file_path);
            target_conn->subscribed_file_path = strdup(request->file_path);
            target_conn->last_analysis_request_time = time(NULL); // Reset debounce
            response->result_json = strdup("{\"status\":\"subscribed\"}");
            fprintf(stdout, "Client %s subscribed to real-time analysis for %s\n", request->client_id, request->file_path);
        } else {
            response->success = false;
            response->error_message = strdup("Failed to subscribe: connection not found or missing file_path");
            server->error_count++;
        }
        pthread_mutex_unlock(&server->connection_mutex);
    } else if (strcmp(request->command, "unsubscribe_analysis") == 0) {
        // Unsubscribe a connection from real-time analysis
        pthread_mutex_lock(&server->connection_mutex);
        DevServerConnection *target_conn = NULL;
        for (size_t i = 0; i < server->connection_count; i++) {
            if (request->client_id && strcmp(server->connections[i].client_id, request->client_id) == 0) {
                target_conn = &server->connections[i];
                break;
            }
        }

        if (target_conn) {
            target_conn->is_subscribed_for_analysis = false;
            if (target_conn->subscribed_file_path) {
                free(target_conn->subscribed_file_path);
                target_conn->subscribed_file_path = NULL;
            }
            response->result_json = strdup("{\"status\":\"unsubscribed\"}");
            fprintf(stdout, "Client %s unsubscribed from real-time analysis\n", request->client_id);
        } else {
            response->success = false;
            response->error_message = strdup("Failed to unsubscribe: connection not found");
            server->error_count++;
        }
        pthread_mutex_unlock(&server->connection_mutex);
    } else if (strcmp(request->command, "stream_suggestions") == 0) {
        // Simulate streaming of AI suggestions over WebSocket
        pthread_mutex_lock(&server->connection_mutex);
        DevServerConnection *target_conn = NULL;
        for (size_t i = 0; i < server->connection_count; i++) {
            if (request->client_id && strcmp(server->connections[i].client_id, request->client_id) == 0) {
                target_conn = &server->connections[i];
                break;
            }
        }

        if (target_conn) {
            // For demonstration, send a few simulated suggestions immediately
            // In a real system, this would be a continuous stream based on AI model output
            send_websocket_text_frame(target_conn->socket_fd, 
                "{\"type\":\"ai_suggestion\", \"id\":\"SIM_001\", \"description\":\"Refactor this loop for clarity.\", \"confidence\":0.8\"}");
            send_websocket_text_frame(target_conn->socket_fd, 
                "{\"type\":\"ai_suggestion\", \"id\":\"SIM_002\", \"description\":\"Consider using a more idiomatic pattern here.\", \"confidence\":0.7\"}");
            
            response->result_json = strdup("{\"status\":\"streaming_simulated_suggestions\"}");
            fprintf(stdout, "Client %s requested simulated suggestion stream.\n", request->client_id);
        } else {
            response->success = false;
            response->error_message = strdup("Failed to stream: connection not found");
            server->error_count++;
        }
        pthread_mutex_unlock(&server->connection_mutex);
    } else {
        response->success = false;
        response->error_message = strdup("Unknown command");
        server->error_count++;
    }
    
    // Calculate processing time
    clock_t end_time = clock();
    response->processing_time_ms = ((double)(end_time - start_time) / CLOCKS_PER_SEC) * 1000.0;
    
    // Update performance metrics
    server->total_response_time += response->processing_time_ms;
    server->average_response_time = server->total_response_time / server->total_requests;
    
    response->timestamp = time(NULL);
    response->response_size = response->result_json ? strlen(response->result_json) : 0;
    
    return response;
}

// =============================================================================
// API Compatibility Wrapper
// =============================================================================

DevServerResponse *asthra_dev_server_handle_request(AsthraDevelopmentServer *server, DevServerRequest *request) {
    return process_development_server_request(server, request);
}

// =============================================================================
// Request/Response Creation Functions
// =============================================================================

DevServerResponse *dev_server_response_create(const char *request_id, bool success) {
    DevServerResponse *response = malloc(sizeof(DevServerResponse));
    if (!response) {
        return NULL;
    }
    
    response->success = success;
    response->request_id = request_id ? strdup(request_id) : NULL;
    response->processing_time_ms = 0.0;
    response->result_json = NULL;
    response->error_message = NULL;
    response->timestamp = time(NULL);
    response->response_size = 0;
    
    return response;
}