#include "websocket_handler.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

#ifdef HAVE_OPENSSL
#include <openssl/sha.h> // For SHA1
#include <openssl/bio.h> // For Base64
#include <openssl/evp.h> // For Base64
#endif

#ifdef HAVE_OPENSSL
// Helper function for SHA1 hashing
void sha1_hash(const char *input, unsigned char *output) {
    SHA1((const unsigned char *)input, strlen(input), output);
}

// Helper function for Base64 encoding
char *base64_encode(const unsigned char *input, int length) {
    BIO *bmem, *b64;
    BUF_MEM *bptr;

    b64 = BIO_new(BIO_f_base64());
    bmem = BIO_new(BIO_s_mem());
    b64 = BIO_push(b64, bmem);
    (void)BIO_write(b64, input, length);
    BIO_flush(b64);
    BIO_get_buf_mem(b64, &bptr);

    char *buf = (char *)malloc(bptr->length + 1);
    memcpy(buf, bptr->data, bptr->length);
    buf[bptr->length] = '\0';
    BIO_free_all(b64);
    return buf;
}
#else
// Simple SHA1 stub when OpenSSL is not available
void sha1_hash(const char *input, unsigned char *output) {
    // This is a stub implementation - not cryptographically secure
    // For development server testing only
    memset(output, 0, 20);
    for (size_t i = 0; i < strlen(input) && i < 20; i++) {
        output[i] = input[i];
    }
}

// Simple Base64 encoding stub
char *base64_encode(const unsigned char *input, int length) {
    // Simple stub - returns hex representation instead
    char *buf = malloc(length * 2 + 1);
    if (!buf) return NULL;
    
    for (int i = 0; i < length; i++) {
        sprintf(buf + i * 2, "%02x", input[i]);
    }
    buf[length * 2] = '\0';
    return buf;
}
#endif

// Function to generate WebSocket accept key
char *generate_websocket_accept_key(const char *websocket_key) {
    const char *guid = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
    size_t key_len = strlen(websocket_key);
    size_t guid_len = strlen(guid);
    char *combined = malloc(key_len + guid_len + 1);
    if (!combined) return NULL;
    strcpy(combined, websocket_key);
    strcat(combined, guid);

    unsigned char sha1_result[20]; // SHA1 produces 20 bytes
    sha1_hash(combined, sha1_result);
    free(combined);

    char *accept_key = base64_encode(sha1_result, 20);
    return accept_key;
}

// Real WebSocket frame parser
bool parse_websocket_frame(const char *buffer, size_t buffer_len, char **payload, size_t *payload_len) {
    if (!buffer || buffer_len < 2 || !payload || !payload_len) {
        return false;
    }
    
    const unsigned char *frame = (const unsigned char *)buffer;
    size_t offset = 0;
    
    // Parse first byte: FIN and opcode
    unsigned char fin = (frame[offset] >> 7) & 0x01;
    (void)fin; // Suppress unused warning
    unsigned char opcode = frame[offset] & 0x0F;
    offset++;
    
    // Parse second byte: MASK and payload length
    unsigned char mask = (frame[offset] >> 7) & 0x01;
    unsigned char payload_len_initial = frame[offset] & 0x7F;
    offset++;
    
    // Only handle text frames (opcode 0x1) for now
    if (opcode != 0x1) {
        return false;
    }
    
    // Determine actual payload length
    size_t actual_payload_len = 0;
    if (payload_len_initial <= 125) {
        actual_payload_len = payload_len_initial;
    } else if (payload_len_initial == 126) {
        if (offset + 2 > buffer_len) return false;
        actual_payload_len = (frame[offset] << 8) | frame[offset + 1];
        offset += 2;
    } else if (payload_len_initial == 127) {
        // 64-bit length not supported for simplicity
        return false;
    }
    
    // Extract masking key if present
    unsigned char masking_key[4] = {0};
    if (mask) {
        if (offset + 4 > buffer_len) return false;
        memcpy(masking_key, frame + offset, 4);
        offset += 4;
    }
    
    // Check if we have enough data
    if (offset + actual_payload_len > buffer_len) {
        return false;
    }
    
    // Extract and unmask payload
    *payload = malloc(actual_payload_len + 1);
    if (!*payload) {
        return false;
    }
    
    for (size_t i = 0; i < actual_payload_len; i++) {
        if (mask) {
            (*payload)[i] = frame[offset + i] ^ masking_key[i % 4];
        } else {
            (*payload)[i] = frame[offset + i];
        }
    }
    (*payload)[actual_payload_len] = '\0';
    *payload_len = actual_payload_len;
    
    return true;
}

// Helper to send WebSocket text frame
void send_websocket_text_frame(int fd, const char *message) {
    if (!message) return;

    size_t len = strlen(message);
    uint8_t header[10]; // Max header size for 64-bit length
    size_t header_len = 0;

    // FIN + OPCODE (text frame)
    header[header_len++] = 0x81;

    // Payload length
    if (len <= 125) {
        header[header_len++] = (uint8_t)len;
    } else if (len <= 65535) {
        header[header_len++] = 126;
        header[header_len++] = (uint8_t)((len >> 8) & 0xFF);
        header[header_len++] = (uint8_t)(len & 0xFF);
    } else {
        header[header_len++] = 127;
        // For simplicity, handle up to 2GB, assuming size_t is 64-bit
        header[header_len++] = (uint8_t)((len >> 56) & 0xFF);
        header[header_len++] = (uint8_t)((len >> 48) & 0xFF);
        header[header_len++] = (uint8_t)((len >> 40) & 0xFF);
        header[header_len++] = (uint8_t)((len >> 32) & 0xFF);
        header[header_len++] = (uint8_t)((len >> 24) & 0xFF);
        header[header_len++] = (uint8_t)((len >> 16) & 0xFF);
        header[header_len++] = (uint8_t)((len >> 8) & 0xFF);
        header[header_len++] = (uint8_t)(len & 0xFF);
    }

    // Send header
    send(fd, header, header_len, 0);
    // Send payload
    send(fd, message, len, 0);
}