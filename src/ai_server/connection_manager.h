#ifndef CONNECTION_MANAGER_H
#define CONNECTION_MANAGER_H

#include "development_server.h"
#include <pthread.h>

// Connection management functions
void asthra_dev_server_add_connection(AsthraDevelopmentServer *server, int client_fd, const char *client_id, const char *websocket_key);
void asthra_dev_server_remove_connection(AsthraDevelopmentServer *server, int client_fd);
void asthra_dev_server_broadcast_notification(AsthraDevelopmentServer *server, const char *notification_json);

#endif // CONNECTION_MANAGER_H