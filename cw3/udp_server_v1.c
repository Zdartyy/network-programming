#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 2020
#define BUF_SIZE 65536

int is_palindrome(const char *word, int len) {
    for (int i = 0; i < len / 2; i++) {
        if (tolower((unsigned char)word[i]) != tolower((unsigned char)word[len - 1 - i]))
            return 0;
    }
    return 1;
}

int main(void) {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == -1) { perror("socket"); exit(1); }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        perror("bind"); exit(1);
    }

    printf("Serwer palindromów nasłuchuje na porcie %d...\n", PORT);

    while (1) {
        char buf[BUF_SIZE];
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);

        ssize_t n = recvfrom(sock, buf, sizeof(buf) - 1, 0, (struct sockaddr *)&client_addr, &client_len);
        if (n == -1) { perror("recvfrom"); continue; }

        buf[n] = '\0';

        if (n >= 2 && buf[n-2] == '\r' && buf[n-1] == '\n') n -= 2;
        else if (n >= 1 && buf[n-1] == '\n')                 n -= 1;
        buf[n] = '\0';

        if (n > 0 && (buf[0] == ' ' || buf[n-1] == ' ')) {
            sendto(sock, "ERROR", 5, 0,
                   (struct sockaddr *)&client_addr, client_len);
            continue;
        }

        int valid = 1;
        for (int i = 0; i < n; i++) {
            if (!isalpha((unsigned char)buf[i]) && buf[i] != ' ') {
                valid = 0;
                break;
            }
            if (buf[i] == ' ' && buf[i+1] == ' ') {
                valid = 0;
                break;
            }
        }

        if (!valid) {
            sendto(sock, "ERROR", 5, 0,
                   (struct sockaddr *)&client_addr, client_len);
            continue;
        }

        int total = 0, palindromes = 0;

        char tmp[BUF_SIZE];
        memcpy(tmp, buf, n + 1);

        char *word = strtok(tmp, " ");
        while (word != NULL) {
            int wlen = strlen(word);
            total++;
            if (is_palindrome(word, wlen))
                palindromes++;
            word = strtok(NULL, " ");
        }

        char response[64];
        int resp_len = snprintf(response, sizeof(response), "%d/%d", palindromes, total);

        sendto(sock, response, resp_len, 0,
               (struct sockaddr *)&client_addr, client_len);

        printf("Zapytanie: \"%s\" → %s", buf, response);
    }
}
