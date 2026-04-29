#define _GNU_SOURCE
#define _POSIX_C_SOURCE 200112L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

#include <openssl/ssl.h>
#include <openssl/err.h>

#define BUF_SIZE 4096


static void parse_server_header(const char *response, char *out, size_t out_size) {
    const char *p = response;
    while (*p) {

        if (strncasecmp(p, "Server:", 7) == 0) {
            p += 7;

            while (*p == ' ' || *p == '\t') p++;

            size_t i = 0;
            while (*p && *p != '\r' && *p != '\n' && i < out_size - 1)
                out[i++] = *p++;
            out[i] = '\0';
            return;
        }

        while (*p && *p != '\n') p++;
        if (*p == '\n') p++;
    }
    strncpy(out, "(no Server header)", out_size - 1);
    out[out_size - 1] = '\0';
}


static int tcp_connect(const char *host, const char *port) {
    struct addrinfo hints, *res, *p;
    int fd = -1;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family   = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(host, port, &hints, &res) != 0)
        return -1;

    for (p = res; p != NULL; p = p->ai_next) {
        fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (fd < 0) continue;
        if (connect(fd, p->ai_addr, p->ai_addrlen) == 0) break;
        close(fd);
        fd = -1;
    }
    freeaddrinfo(res);
    return fd;
}


static void probe_http(const char *host) {
    char result[256] = "(connection refused)";

    int fd = tcp_connect(host, "80");
    if (fd >= 0) {
        char request[512];
        snprintf(request, sizeof(request),
                 "GET / HTTP/1.0\r\nHost: %s\r\nConnection: close\r\n\r\n", host);
        send(fd, request, strlen(request), 0);

        char response[BUF_SIZE] = {0};
        int total = 0, n;
        while (total < (int)sizeof(response) - 1 &&
               (n = recv(fd, response + total, sizeof(response) - total - 1, 0)) > 0) {
            total += n;
            if (strstr(response, "\r\n\r\n")) break;
        }
        close(fd);
        parse_server_header(response, result, sizeof(result));
    }

    printf("  port 80:  %s\n", result);
}


static void probe_https(const char *host) {
    char result[256] = "(connection refused)";

    int fd = tcp_connect(host, "443");
    if (fd < 0) {
        printf("  port 443: %s\n", result);
        return;
    }

    SSL_CTX *ctx = SSL_CTX_new(TLS_client_method());
    if (!ctx) {
        close(fd);
        printf("  port 443: (SSL_CTX error)\n");
        return;
    }
    SSL_CTX_set_verify(ctx, SSL_VERIFY_NONE, NULL);

    SSL *ssl = SSL_new(ctx);
    SSL_set_fd(ssl, fd);
    SSL_set_tlsext_host_name(ssl, host);

    if (SSL_connect(ssl) != 1) {
        snprintf(result, sizeof(result), "(TLS handshake failed)");
    } else {
        char request[512];
        snprintf(request, sizeof(request),
                 "GET / HTTP/1.0\r\nHost: %s\r\nConnection: close\r\n\r\n", host);
        SSL_write(ssl, request, (int)strlen(request));

        char response[BUF_SIZE] = {0};
        int total = 0, n;
        while (total < (int)sizeof(response) - 1 &&
               (n = SSL_read(ssl, response + total, sizeof(response) - total - 1)) > 0) {
            total += n;
            if (strstr(response, "\r\n\r\n")) break;
        }
        parse_server_header(response, result, sizeof(result));
    }

    SSL_shutdown(ssl);
    SSL_free(ssl);
    SSL_CTX_free(ctx);
    close(fd);

    printf("  port 443: %s\n", result);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Użycie: %s <host1> [host2] ...\n", argv[0]);
        return 1;
    }

    for (int i = 1; i < argc; i++) {
        printf("%s:\n", argv[i]);
        probe_http(argv[i]);
        probe_https(argv[i]);
    }
    return 0;
}