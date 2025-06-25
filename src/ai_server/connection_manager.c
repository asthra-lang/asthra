#include "connection_manager.h"
#include "websocket_handler.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

// ============================================================================
// Connection Management Functions
// ============================================================================

void asthra_dev_server_add_connection(AsthraDevelopmentServer *server, int client_fd,
                                      const char *client_id, const char *websocket_key) {
    if (!server)
        return;

    pthread_mutex_lock(&server->connection_mutex);

    // Check if capacity needs to be increased
    if (server->connection_count >= server->connection_capacity) {
        size_t new_capacity =
            server->connection_capacity == 0 ? 4 : server->connection_capacity * 2;
        DevServerConnection *new_connections =
            realloc(server->connections, sizeof(DevServerConnection) * new_capacity);
        if (!new_connections) {
            fprintf(stderr, "Error: Failed to reallocate connections array.\n");
            pthread_mutex_unlock(&server->connection_mutex);
            return;
        }
        server->connections = new_connections;
        server->connection_capacity = new_capacity;
    }

    // Add new connection
    DevServerConnection *new_conn = &server->connections[server->connection_count];
    new_conn->client_id = strdup(client_id);
    new_conn->socket_fd = client_fd;
    new_conn->is_active = true;
    new_conn->last_activity = time(NULL);
    new_conn->websocket_key = strdup(websocket_key); // Store WebSocket key
    new_conn->is_subscribed_for_analysis = false;    // Initialize subscription flag
    new_conn->last_analysis_request_time = 0;        // Initialize debouncing time
    new_conn->subscribed_file_path = NULL;           // Initialize subscribed file path
    server->connection_count++;
    server->stats.active_connections = server->connection_count; // Update stats

    fprintf(stdout, "New WebSocket connection from %s (fd: %d). Total active: %zu\n", client_id,
            client_fd, server->connection_count);

    pthread_mutex_unlock(&server->connection_mutex);
}

void asthra_dev_server_remove_connection(AsthraDevelopmentServer *server, int client_fd) {
    if (!server)
        return;

    pthread_mutex_lock(&server->connection_mutex);

    size_t i;
    bool found = false;
    for (i = 0; i < server->connection_count; i++) {
        if (server->connections[i].socket_fd == client_fd) {
            found = true;
            break;
        }
    }

    if (found) {
        // Free resources for the removed connection
        free(server->connections[i].client_id);
        free(server->connections[i].websocket_key);
        free(server->connections[i].subscribed_file_path); // Free subscribed file path
        close(server->connections[i].socket_fd);

        // Shift remaining connections to fill the gap
        for (size_t j = i; j < server->connection_count - 1; j++) {
            server->connections[j] = server->connections[j + 1];
        }
        server->connection_count--;
        server->stats.active_connections = server->connection_count; // Update stats

        fprintf(stdout, "Removed WebSocket connection (fd: %d). Total active: %zu\n", client_fd,
                server->connection_count);
    } else {
        fprintf(stderr, "Warning: Attempted to remove non-existent connection (fd: %d).\n",
                client_fd);
    }

    pthread_mutex_unlock(&server->connection_mutex);
}

void asthra_dev_server_broadcast_notification(AsthraDevelopmentServer *server,
                                              const char *notification_json) {
    if (!server || !notification_json)
        return;

    pthread_mutex_lock(&server->connection_mutex);

    // Broadcast to all active connections
    for (size_t i = 0; i < server->connection_count; i++) {
        if (server->connections[i].is_active) {
            // Send the notification as a WebSocket text frame
            send_websocket_text_frame(server->connections[i].socket_fd, notification_json);
        }
    }

    pthread_mutex_unlock(&server->connection_mutex);
}