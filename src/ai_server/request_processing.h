#ifndef REQUEST_PROCESSING_H
#define REQUEST_PROCESSING_H

#include "development_server.h"

// Request processing functions
DevServerResponse *process_development_server_request(AsthraDevelopmentServer *server, DevServerRequest *request);

// API compatibility wrappers
DevServerResponse *asthra_dev_server_handle_request(AsthraDevelopmentServer *server, DevServerRequest *request);

// Request/Response creation functions
DevServerResponse *dev_server_response_create(const char *request_id, bool success);

#endif // REQUEST_PROCESSING_H