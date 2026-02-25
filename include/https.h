#ifndef HOW_HTTPS_H
#define HOW_HTTPS_H

#include <stdint.h>

#include <openssl/ssl.h>

typedef struct {
    int32_t sockfd;
    SSL* ssl;
} sockres_t;

typedef struct {
    char* body;
    size_t body_length;
} postresp_t;

sockres_t* https_connect(char* host);

/* Okay, this isnt great, but for now I'll take that 1 stack hit */
void https_post(sockres_t* sockres, const char* host, const char* path, char* extras, char* body, size_t body_len, postresp_t* postresp);

#endif // HOW_HTTPS_H
