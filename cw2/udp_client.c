#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Użycie: %s <adres_ip> <port>\n", argv[0]);
        exit(1);
    }

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == -1) { perror("socket"); exit(1); }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family      = AF_INET;
    server_addr.sin_port        = htons(atoi(argv[2]));
    server_addr.sin_addr.s_addr = inet_addr(argv[1]);


    ssize_t sent = sendto(sock, "", 0, 0,
                          (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (sent == -1) { perror("sendto"); exit(1); }

    printf("Wysłano pusty datagram — czekam na wizytówkę...\n");

    char buf[1024];
    ssize_t n = recvfrom(sock, buf, sizeof(buf), 0, NULL, NULL);
    if (n == -1) { perror("recvfrom"); exit(1); }

    for (ssize_t i = 0; i < n; i++) {
        char byte = buf[i];
        if ((byte >= 32 && byte <= 126) || byte == '\n' || byte == '\r' || byte == '\t') {
            putchar(byte);
        }
    }

    close(sock);
    return 0;
}
