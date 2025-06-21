/**
 * Asthra Programming Language Compiler
 * Development Server - Lifecycle Management
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Server creation, destruction, and control operations
 */

#include "server_lifecycle.h"
#include "server_network.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>

// =============================================================================
// Server Lifecycle Management
// =============================================================================

AsthraDevelopmentServer *asthra_dev_server_create(int port) {
    AsthraDevelopmentServer *server = malloc(sizeof(AsthraDevelopmentServer));
    if (!server) {
        return NULL;
    }
    
    memset(server, 0, sizeof(AsthraDevelopmentServer));
    
    // Initialize fast check engine
    server->fast_check = fast_check_engine_create();
    if (!server->fast_check) {
        free(server);
        return NULL;
    }
    
    // Server configuration
    server->port = port > 0 ? port : 8080;
    server->is_running = false;
    server->should_stop = false;
    
    // Connection management
    server->connection_capacity = 64; // Start with 64 connections
    server->connections = malloc(sizeof(DevServerConnection) * server->connection_capacity);
    if (!server->connections) {
        fast_check_engine_destroy(server->fast_check);
        free(server);
        return NULL;
    }
    
    memset(server->connections, 0, sizeof(DevServerConnection) * server->connection_capacity);
    server->connection_count = 0;
    
    // Initialize connection mutex
    if (pthread_mutex_init(&server->connection_mutex, NULL) != 0) {
        free(server->connections);
        fast_check_engine_destroy(server->fast_check);
        free(server);
        return NULL;
    }
    
    // Initialize rate limiting management
    server->client_rate_limit_capacity = 16; // Start with 16 rate limit entries
    server->client_rate_limits = calloc(server->client_rate_limit_capacity, sizeof(ClientRateLimit*));
    if (!server->client_rate_limits) {
        pthread_mutex_destroy(&server->connection_mutex);
        free(server->connections);
        fast_check_engine_destroy(server->fast_check);
        free(server);
        return NULL;
    }
    server->client_rate_limit_count = 0;

    if (pthread_mutex_init(&server->client_rate_limit_mutex, NULL) != 0) {
        free(server->client_rate_limits);
        pthread_mutex_destroy(&server->connection_mutex);
        free(server->connections);
        fast_check_engine_destroy(server->fast_check);
        free(server);
        return NULL;
    }
    
    // Performance metrics
    server->total_requests = 0;
    server->total_response_time = 0.0;
    server->average_response_time = 0.0;
    server->error_count = 0;
    
    // Rate limiting (global settings, will be replaced by per-client)
    server->max_requests_per_minute = 1000; // Default rate limit
    server->rate_limit_window_start = time(NULL);
    server->current_window_requests = 0;
    
    return server;
}

void asthra_dev_server_destroy(AsthraDevelopmentServer *server) {
    if (!server) return;
    
    // Stop server if running
    if (server->is_running) {
        asthra_dev_server_stop(server);
    }
    
    // Clean up connections
    if (server->connections) {
        pthread_mutex_lock(&server->connection_mutex);
        for (size_t i = 0; i < server->connection_count; i++) {
            free(server->connections[i].client_id);
            free(server->connections[i].websocket_key); // Free websocket key
        }
        pthread_mutex_unlock(&server->connection_mutex);
        free(server->connections);
    }
    
    // Clean up client rate limits
    pthread_mutex_lock(&server->client_rate_limit_mutex);
    for (size_t i = 0; i < server->client_rate_limit_count; i++) {
        asthra_dev_server_destroy_client_rate_limit(server->client_rate_limits[i]);
    }
    pthread_mutex_unlock(&server->client_rate_limit_mutex);
    free(server->client_rate_limits);

    // Destroy mutexes
    pthread_mutex_destroy(&server->connection_mutex);
    pthread_mutex_destroy(&server->client_rate_limit_mutex);
    
    // Destroy fast check engine
    if (server->fast_check) {
        fast_check_engine_destroy(server->fast_check);
    }
    
    free(server);
}

bool asthra_dev_server_start(AsthraDevelopmentServer *server) {
    if (!server || server->is_running) {
        return false;
    }
    
    server->should_stop = false;
    
    if (pthread_create(&server->server_thread, NULL, server_network_thread_function, server) != 0) {
        return false;
    }
    
    // Wait a moment for server to start
    usleep(100000); // 100ms
    
    return server->is_running;
}

void asthra_dev_server_stop(AsthraDevelopmentServer *server) {
    if (!server || !server->is_running) {
        return;
    }
    
    server->should_stop = true;
    
    // Wait for server thread to finish
    pthread_join(server->server_thread, NULL);
    
    server->is_running = false;
}

bool asthra_dev_server_is_running(const AsthraDevelopmentServer *server) {
    return server ? server->is_running : false;
}