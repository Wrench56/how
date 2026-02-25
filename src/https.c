#include <stddef.h>
#include <stdint.h>

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <openssl/ssl.h>

#include "errors.h"

#include "https.h"

#define HTTPS_PORT 443
#define REQUEST_SIZE 8196
#define RESPONSE_SIZE 8196

static void https_cleanup(sockres_t* sockres) {
    SSL_shutdown(sockres->ssl);
    SSL_CTX *ctx = SSL_get_SSL_CTX(sockres->ssl);
    SSL_free(sockres->ssl);
    SSL_CTX_free(ctx);
    close(sockres->sockfd);
}

static inline SSL* https_ssl_handshake(int32_t sockfd, char* host) {
    SSL_load_error_strings();
    SSL_library_init();
    SSL_CTX* ctx = SSL_CTX_new(TLS_client_method());

    SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, NULL);
    if (SSL_CTX_set_default_verify_paths(ctx) != 1) {
        SSL_CTX_free(ctx);
        close(sockfd);
        FATAL(SSL_CONNECT_FAIL_EC, "Failed to load system CA certificates!\n");
    }

    SSL* conn = SSL_new(ctx);

    if (SSL_set_tlsext_host_name(conn, host) != 1) {
        sockres_t sockres = (sockres_t) {
            .sockfd = sockfd,
            .ssl = conn
        };
        https_cleanup(&sockres);
        FATAL(SSL_CONNECT_FAIL_EC, "SSL_set_tlsext_host_name() failed!\n");
    }

    if (SSL_set1_host(conn, host) != 1) {
        sockres_t sockres = (sockres_t) {
            .sockfd = sockfd,
            .ssl = conn
        };
        https_cleanup(&sockres);
        FATAL(SSL_CONNECT_FAIL_EC, "SSL_set1_host() failed!\n");
    }

    SSL_set_fd(conn, sockfd);

    int32_t error = SSL_connect(conn);
    if (error != 1) {
        close(sockfd);
        FATAL(SSL_CONNECT_FAIL_EC, "SSL_connect() failed!");
    }

    if (SSL_get_verify_result(conn) != X509_V_OK) {
        sockres_t sockres = (sockres_t) {
            .sockfd = sockfd,
            .ssl = conn
        };
        https_cleanup(&sockres);
        FATAL(SSL_CONNECT_FAIL_EC, "Certificate verification failed!\n");
    }

    return conn;
}

static void https_ssl_fatal(sockres_t* sockres, int32_t ret, int ec, const char* msg) {
    int32_t error = SSL_get_error(sockres->ssl, ret);
    https_cleanup(sockres);
    FATAL(ec, "%s (SSL Error = %d)\n", msg, error);
}

sockres_t* https_connect(char* host) {
    struct sockaddr_in sockaddr;
    
    int32_t sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0) {
        FATAL(SOCK_FAIL_EC, "Creation of socket failed!\n");
    }

    struct hostent* sockhost = gethostbyname(host);
    if (sockhost == NULL) {
        close(sockfd);
        FATAL(SOCK_NOT_FOUND_EC, "Connection to location \"%s\" could not be establish!\n", host);
    }

    memset(&sockaddr, 0, sizeof(sockaddr));
    memcpy(&sockaddr.sin_addr, sockhost->h_addr_list[0], sockhost->h_length);
    sockaddr.sin_family = AF_INET;
    sockaddr.sin_port = htons(HTTPS_PORT);

    if (connect(sockfd, (struct sockaddr*) &sockaddr, sizeof(sockaddr)) < 0) {
        close(sockfd);
        FATAL(SOCK_FAIL_EC, "connect() failed!\n");
    }

    SSL* ssl = https_ssl_handshake(sockfd, host);
    sockres_t* sockres = (sockres_t*) malloc(sizeof(sockres_t));
    if (sockres == NULL) {
        sockres_t sockres = (sockres_t) {
            .sockfd = sockfd,
            .ssl = ssl
        };
        https_cleanup(&sockres);

        FATAL(MALLOC_FAIL_EC, "malloc() could not allocate %ld bytes for sockret_t", sizeof(sockres_t));
    }

    sockres->sockfd = sockfd;
    sockres->ssl = ssl;

    return sockres;
}

void https_post(sockres_t* sockres, const char* host, const char* path, char* extras, char* body, size_t body_len) {
    char req[REQUEST_SIZE];
    int32_t req_len = snprintf(req, sizeof(req),
        "POST /%s HTTP/1.1\r\n"
        "Host: %s\r\n"
        "User-Agent: how-socket\r\n"
        "Content-Type: application/json\r\n"
        "Content-Length: %zu\r\n"
        "%s"
        "Connection: close\r\n"
        "\r\n",
        path,
        host,
        body_len,
        extras
    );

    if (req_len <= 0 || req_len >= (int32_t) sizeof(req)) {
        FATAL(SSL_SEND_FAIL_EC, "Request buffer too small!\n");
    }

    uint32_t written = 0;
    do {
        int32_t n = SSL_write(sockres->ssl, req + written, req_len - written);
        if (n <= 0) {
            https_ssl_fatal(sockres, n, SSL_SEND_FAIL_EC, "SSL_write() for headers failed!");
        }

        written += n;
    } while (written < (uint32_t) req_len);

    written = 0;
    while (written < body_len) {
        int n = SSL_write(sockres->ssl, body + written, body_len - written);
        if (n <= 0) {
            https_ssl_fatal(sockres, n, SSL_SEND_FAIL_EC, "SSL_write() for body failed!");
        }

        written += n;
    }

    char buf[RESPONSE_SIZE];
    while (1) {
        int32_t n = SSL_read(sockres->ssl, buf, (int32_t) sizeof(buf));
        if (n > 0) {
            fwrite(buf, 1, (size_t) n, stdout);
            continue;
        } else if (n == 0) {
            break;
        }

        https_ssl_fatal(sockres, n, SSL_RECV_FAIL_EC, "SSL_read() failed!");
    }

    https_cleanup(sockres);
}
