/**
 * Asthra Programming Language Compiler
 * Development Server - Network Operations
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Socket operations, WebSocket handling, and network communication
 */

#include "server_network.h"
#include "websocket_handler.h"
#include "connection_manager.h"
#include "request_handler.h"
#include "request_processing.h"
#include "realtime_analysis.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

// =============================================================================
// Network Thread Function
// =============================================================================

void *server_network_thread_function(void *arg) {
    AsthraDevelopmentServer *server = (AsthraDevelopmentServer *)arg;
    
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        server->is_running = false;
        return NULL;
    }
    
    // Allow socket reuse
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        close(server_fd);
        server->is_running = false;
        return NULL;
    }
    
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(server->port);
    
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        close(server_fd);
        server->is_running = false;
        return NULL;
    }
    
    if (listen(server_fd, 10) < 0) {
        close(server_fd);
        server->is_running = false;
        return NULL;
    }
    
    printf("Asthra Development Server listening on port %d\n", server->port);
    server->is_running = true;
    
    while (!server->should_stop) {
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
                    
                    // Simple HTTP/WebSocket upgrade handling
                    if (strstr(buffer, "Upgrade: websocket") && strstr(buffer, "Sec-WebSocket-Key:")) {
                        char *websocket_key_start = strstr(buffer, "Sec-WebSocket-Key:");
                        char *websocket_key_end = strstr(websocket_key_start, "\r\n");
                        char websocket_key[256];
                        
                        if (websocket_key_start && websocket_key_end) {
                            websocket_key_start += strlen("Sec-WebSocket-Key:");
                            // Trim leading/trailing whitespace
                            while (*websocket_key_start == ' ' || *websocket_key_start == '\t') websocket_key_start++;
                            size_t key_len = (size_t)(websocket_key_end - websocket_key_start);
                            if (key_len >= sizeof(websocket_key)) key_len = sizeof(websocket_key) - 1;
                            strncpy(websocket_key, websocket_key_start, key_len);
                            websocket_key[key_len] = '\0';

                            char *accept_key = generate_websocket_accept_key(websocket_key);
                            if (accept_key) {
                                char response[1024];
                                snprintf(response, sizeof(response),
                                    "HTTP/1.1 101 Switching Protocols\r\n"
                                    "Upgrade: websocket\r\n"
                                    "Connection: Upgrade\r\n"
                                    "Sec-WebSocket-Accept: %s\r\n"
                                    "\r\n",
                                    accept_key);
                                send(client_fd, response, strlen(response), 0);
                                free(accept_key);

                                // Add connection to server's active connections
                                char client_id[32];
                                snprintf(client_id, sizeof(client_id), "client_ws_%d", client_fd);
                                asthra_dev_server_add_connection(server, client_fd, client_id, websocket_key);

                                // Keep connection open for WebSocket communication (loop below)
                                // For now, we will break after handshake for simplicity
                                // A full implementation would enter a WebSocket message handling loop here
                                // For the scope of this task, we will just handle the handshake.
                                continue; // Continue to next select() call to handle other connections

                            } else {
                                fprintf(stderr, "Error: Failed to generate WebSocket accept key.\n");
                                close(client_fd);
                            }
                        } else {
                            fprintf(stderr, "Warning: Malformed Sec-WebSocket-Key header.\n");
                            close(client_fd);
                        }
                    } else {
                        // Handle HTTP request (maybe for health check)
                        const char *http_response = 
                            "HTTP/1.1 200 OK\r\n"
                            "Content-Type: application/json\r\n"
                            "Content-Length: 27\r\n"
                            "\r\n"
                            "{\"status\":\"server_running\"}";
                        send(client_fd, http_response, strlen(http_response), 0);
                        close(client_fd);
                    }
                } else if (bytes_read == 0) {
                    // Client closed connection
                    asthra_dev_server_remove_connection(server, client_fd);
                } else {
                    // Error reading from client
                    perror("recv failed");
                    close(client_fd);
                }
            } else {
                perror("accept failed");
            }
        }

        // Handle existing WebSocket connections (read loop for active connections)
        pthread_mutex_lock(&server->connection_mutex);
        for (size_t i = 0; i < server->connection_count; ) {
            DevServerConnection *conn = &server->connections[i];
            if (conn->is_active) {
                // For now, simply check if the socket is readable and close if not.
                // A full implementation would parse WebSocket frames here.
                fd_set conn_readfds;
                FD_ZERO(&conn_readfds);
                FD_SET(conn->socket_fd, &conn_readfds);

                struct timeval conn_timeout;
                conn_timeout.tv_sec = 0;
                conn_timeout.tv_usec = 10000; // 10ms timeout

                int conn_activity = select(conn->socket_fd + 1, &conn_readfds, NULL, NULL, &conn_timeout);

                if (conn_activity < 0 && errno != EINTR) {
                    perror("select on client socket failed");
                    asthra_dev_server_remove_connection(server, conn->socket_fd);
                    // Do not increment i, as element at i has been replaced by the last element
                } else if (conn_activity == 0) {
                    // No data, continue
                    i++;
                } else if (FD_ISSET(conn->socket_fd, &conn_readfds)) {
                    char ws_buffer[4096];
                    ssize_t ws_bytes_read = recv(conn->socket_fd, ws_buffer, sizeof(ws_buffer) - 1, 0);

                    if (ws_bytes_read <= 0) {
                        // Client closed connection or error
                        asthra_dev_server_remove_connection(server, conn->socket_fd);
                        // Do not increment i
                    } else {
                        ws_buffer[ws_bytes_read] = '\0';
                        // Process WebSocket frame (real implementation)
                        char *payload = NULL;
                        size_t payload_len = 0;
                        
                        if (parse_websocket_frame(ws_buffer, ws_bytes_read, &payload, &payload_len)) {
                            // Parse JSON request from WebSocket payload
                            DevServerRequest *request = dev_server_request_create(payload);
                            if (request) {
                                // Handle the request and send response
                                DevServerResponse *response = process_development_server_request(server, request);
                                if (response) {
                                    char *response_json = dev_server_response_to_json(response);
                                    if (response_json) {
                                        send_websocket_text_frame(conn->socket_fd, response_json);
                                        free(response_json);
                                    }
                                    dev_server_response_destroy(response);
                                }
                                dev_server_request_destroy(request);
                            }
                            free(payload);
                        } else {
                            fprintf(stderr, "Failed to parse WebSocket frame from %s\n", conn->client_id);
                            asthra_dev_server_remove_connection(server, conn->socket_fd);
                        }
                        i++;
                    }
                }
            } else {
                i++;
            }
        }

        // Periodically perform real-time analysis for subscribed clients
        for (size_t i = 0; i < server->connection_count; i++) {
            DevServerConnection *conn = &server->connections[i];
            if (conn->is_active && conn->is_subscribed_for_analysis) {
                handle_real_time_analysis(server, conn);
            }
        }
        pthread_mutex_unlock(&server->connection_mutex);
    }
    
    close(server_fd);
    server->is_running = false;
    return NULL;
}