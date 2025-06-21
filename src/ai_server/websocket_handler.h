#ifndef WEBSOCKET_HANDLER_H
#define WEBSOCKET_HANDLER_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// WebSocket frame structure for basic implementation
typedef struct {
    uint8_t fin : 1;
    uint8_t rsv1 : 1;
    uint8_t rsv2 : 1;
    uint8_t rsv3 : 1;
    uint8_t opcode : 4;
    uint8_t mask : 1;
    uint8_t payload_len : 7;
} websocket_frame_header;

// WebSocket handling functions
char *generate_websocket_accept_key(const char *websocket_key);
bool parse_websocket_frame(const char *buffer, size_t buffer_len, char **payload, size_t *payload_len);
void send_websocket_text_frame(int fd, const char *message);

// SHA1 and Base64 helper functions
void sha1_hash(const char *input, unsigned char *output);
char *base64_encode(const unsigned char *input, int length);

#endif // WEBSOCKET_HANDLER_H