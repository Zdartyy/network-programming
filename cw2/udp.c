#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Użycie: %s <port>\n", argv[0]);
        exit(1);
    }

    int port = atoi(argv[1]);

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == -1) { perror("socket"); exit(1); }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        perror("bind"); exit(1);
    }


    printf("Serwer UDP nasłuchuje na porcie %d...\n", port);

    while (1) {
        char buf[1024];
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);

        ssize_t n = recvfrom(sock, buf, sizeof(buf), 0,
                             (struct sockaddr *)&client_addr, &client_len);
        if (n == -1) { perror("recvfrom"); continue; }

        printf("Otrzymano datagram (%zd bajtów) — wysyłam wizytówkę\n", n);

        const char *message = "Hello, world!\r\n";
        ssize_t sent = sendto(sock, message, strlen(message), 0,
                              (struct sockaddr *)&client_addr, client_len);
        if (sent == -1) { perror("sendto"); continue; }
    }
}
