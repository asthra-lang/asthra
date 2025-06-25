#ifndef REQUEST_HANDLER_H
#define REQUEST_HANDLER_H

#include "development_server.h"

// Request/Response handling
DevServerRequest *dev_server_request_create(const char *json);
void dev_server_request_destroy(DevServerRequest *request);
// Note: dev_server_response_create is declared in development_server.h
void dev_server_response_destroy(DevServerResponse *response);
char *dev_server_response_to_json(const DevServerResponse *response);

// Error handling
const char *dev_server_error_to_string(DevServerErrorCode error);

// Performance monitoring
DevServerStats asthra_dev_server_get_stats(const AsthraDevelopmentServer *server);
void asthra_dev_server_reset_stats(AsthraDevelopmentServer *server);
char *asthra_dev_server_stats_to_json(const DevServerStats *stats);

#endif // REQUEST_HANDLER_H