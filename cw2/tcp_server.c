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

    int server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock == -1) { perror("socket"); exit(1); }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;
    if (bind(server_sock, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        perror("bind"); exit(1);
    }

    if (listen(server_sock, 10) == -1) { perror("listen"); exit(1); }

    printf("Serwer nasłuchuje na porcie %d...\n", port);

    while (1) {
        int client_sock = accept(server_sock, NULL, NULL);
        if (client_sock == -1) { perror("accept"); continue; }

        const char *message = "Hello, world!\r\n";
        write(client_sock, message, strlen(message));
        close(client_sock);
    }
}
