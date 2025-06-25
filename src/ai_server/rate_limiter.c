#include "rate_limiter.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// ============================================================================
// Rate Limiting Functions
// ============================================================================

// Get or create a client's rate limit entry
ClientRateLimit *asthra_dev_server_get_or_create_rate_limit(AsthraDevelopmentServer *server,
                                                            const char *client_id, double capacity,
                                                            double refill_rate) {
    if (!server || !client_id)
        return NULL;

    pthread_mutex_lock(&server->client_rate_limit_mutex);

    // Try to find existing entry
    for (size_t i = 0; i < server->client_rate_limit_count; i++) {
        if (server->client_rate_limits[i] &&
            strcmp(server->client_rate_limits[i]->client_id, client_id) == 0) {
            pthread_mutex_unlock(&server->client_rate_limit_mutex);
            return server->client_rate_limits[i];
        }
    }

    // If not found, create a new one
    ClientRateLimit *new_limit = calloc(1, sizeof(ClientRateLimit));
    if (!new_limit) {
        fprintf(stderr, "Error: Failed to allocate new client rate limit.\n");
        pthread_mutex_unlock(&server->client_rate_limit_mutex);
        return NULL;
    }

    new_limit->client_id = strdup(client_id);
    new_limit->bucket_capacity = capacity;
    new_limit->refill_rate = refill_rate;
    new_limit->tokens = capacity; // Start with full bucket
    new_limit->last_refill_time = time(NULL);
    if (pthread_mutex_init(&new_limit->mutex, NULL) != 0) {
        fprintf(stderr, "Error: Failed to initialize rate limit mutex.\n");
        free(new_limit->client_id);
        free(new_limit);
        pthread_mutex_unlock(&server->client_rate_limit_mutex);
        return NULL;
    }

    // Add to server's list
    if (server->client_rate_limit_count >= server->client_rate_limit_capacity) {
        size_t new_capacity = server->client_rate_limit_capacity * 2;
        ClientRateLimit **new_array =
            realloc(server->client_rate_limits, sizeof(ClientRateLimit *) * new_capacity);
        if (!new_array) {
            fprintf(stderr, "Error: Failed to reallocate client rate limits array.\n");
            asthra_dev_server_destroy_client_rate_limit(new_limit);
            pthread_mutex_unlock(&server->client_rate_limit_mutex);
            return NULL;
        }
        server->client_rate_limits = new_array;
        server->client_rate_limit_capacity = new_capacity;
    }

    server->client_rate_limits[server->client_rate_limit_count++] = new_limit;

    pthread_mutex_unlock(&server->client_rate_limit_mutex);
    return new_limit;
}

// Check and consume tokens from a client's rate limit bucket
bool asthra_dev_server_check_rate_limit(ClientRateLimit *limit, double tokens_needed) {
    if (!limit)
        return true; // No limit, allow

    pthread_mutex_lock(&limit->mutex);

    time_t current_time = time(NULL);
    double time_elapsed = difftime(current_time, limit->last_refill_time);

    // Refill tokens
    limit->tokens += time_elapsed * limit->refill_rate;
    if (limit->tokens > limit->bucket_capacity) {
        limit->tokens = limit->bucket_capacity;
    }
    limit->last_refill_time = current_time;

    // Check if enough tokens are available
    if (limit->tokens >= tokens_needed) {
        limit->tokens -= tokens_needed;
        pthread_mutex_unlock(&limit->mutex);
        return true;
    } else {
        pthread_mutex_unlock(&limit->mutex);
        return false;
    }
}

// Destroy a client rate limit entry
void asthra_dev_server_destroy_client_rate_limit(ClientRateLimit *limit) {
    if (!limit)
        return;
    pthread_mutex_destroy(&limit->mutex);
    free(limit->client_id);
    free(limit);
}