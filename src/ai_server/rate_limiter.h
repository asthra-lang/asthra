#ifndef RATE_LIMITER_H
#define RATE_LIMITER_H

#include "development_server.h"
#include <stdbool.h>

// Rate limiting functions
ClientRateLimit* asthra_dev_server_get_or_create_rate_limit(AsthraDevelopmentServer *server, const char *client_id, double capacity, double refill_rate);
bool asthra_dev_server_check_rate_limit(ClientRateLimit *limit, double tokens_needed);
void asthra_dev_server_destroy_client_rate_limit(ClientRateLimit *limit);

#endif // RATE_LIMITER_H