#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <curl/curl.h>

static size_t header_cb(char *buffer, size_t size, size_t nitems, void *userdata) {
    size_t len = nitems * size;

    if (len > 7 && strncasecmp(buffer, "Server:", 7) == 0) {
        char *val = buffer + 7;
        size_t vlen = len - 7;

        while (vlen > 0 && (*val == ' ' || *val == '\t')) { val++; vlen--; }
        while (vlen > 0 && (val[vlen-1] == '\r' || val[vlen-1] == '\n')) vlen--;

        char *out = (char *)userdata;
        size_t copy = vlen < 255 ? vlen : 255;
        memcpy(out, val, copy);
        out[copy] = '\0';
    }
    return len;
}

static size_t discard_cb(char *buf, size_t size, size_t nmemb, void *ud) {
    (void)buf; (void)ud;
    return size * nmemb;
}

static void probe(const char *host, int port) {
    char url[512];
    char server[256] = "(no Server header)";

    snprintf(url, sizeof(url), "%s://%s/",
             port == 443 ? "https" : "http", host);

    CURL *curl = curl_easy_init();
    if (!curl) { printf("port %d: (curl init failed)\n", port); return; }

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, header_cb);
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, server);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, discard_cb);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
    curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_0);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 0L);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK)
        snprintf(server, sizeof(server), "(%s)", curl_easy_strerror(res));

    printf("  port %d: %s\n", port, server);
    curl_easy_cleanup(curl);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Użycie: %s <host1> [host2] ...\n", argv[0]);
        return 1;
    }

    curl_global_init(CURL_GLOBAL_DEFAULT);

    for (int i = 1; i < argc; i++) {
        printf("%s:\n", argv[i]);
        probe(argv[i], 80);
        probe(argv[i], 443);
    }

    curl_global_cleanup();
    return 0;
}