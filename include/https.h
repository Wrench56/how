#ifndef HOW_HTTPS_H
#define HOW_HTTPS_H

#include <stdint.h>

#include <openssl/ssl.h>

typedef struct {
    int32_t sockfd;
    SSL* ssl;
} sockres_t;

sockres_t* https_connect(char* host);
void https_post(sockres_t* sockres, const char* host, const char* path, char* extras, char* body, size_t body_len);

#endif // HOW_HTTPS_H
