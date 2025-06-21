#ifndef SERVER_LIFECYCLE_H
#define SERVER_LIFECYCLE_H

#include "development_server.h"
#include <stdbool.h>

// Server lifecycle management
AsthraDevelopmentServer *asthra_dev_server_create(int port);
void asthra_dev_server_destroy(AsthraDevelopmentServer *server);

// Server control operations
bool asthra_dev_server_start(AsthraDevelopmentServer *server);
void asthra_dev_server_stop(AsthraDevelopmentServer *server);
bool asthra_dev_server_is_running(const AsthraDevelopmentServer *server);

#endif // SERVER_LIFECYCLE_H