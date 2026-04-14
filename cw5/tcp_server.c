#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>

#define PORT 2020
#define BACKLOG 128
#define MAX_LINE 1026
#define BUF_SIZE 4096


int is_palindrome(const char *word, int len) {
    for (int i = 0; i < len / 2; i++) {
        if (tolower((unsigned char)word[i]) != tolower((unsigned char)word[len-1-i]))
            return 0;
    }
    return 1;
}


void process_line(const char *line, int len, char *response, int resp_size) {
    if (len > 0 && (line[0] == ' ' || line[len-1] == ' ')) {
        snprintf(response, resp_size, "ERROR");
        return;
    }
    for (int i = 0; i < len; i++) {
        if (!isalpha((unsigned char)line[i]) && line[i] != ' ') {
            snprintf(response, resp_size, "ERROR");
            return;
        }
        if (line[i] == ' ' && i + 1 < len && line[i+1] == ' ') {
            snprintf(response, resp_size, "ERROR");
            return;
        }
    }

    int total = 0, palindromes = 0;
    char tmp[MAX_LINE];
    memcpy(tmp, line, len + 1);

    char *word = strtok(tmp, " ");
    while (word != NULL) {
        int wlen = strlen(word);
        if (wlen > 0) {
            total++;
            if (is_palindrome(word, wlen))
                palindromes++;
        }
        word = strtok(NULL, " ");
    }

    snprintf(response, resp_size, "%d/%d", palindromes, total);
}


int write_all(int fd, const char *data, size_t len) {
    while (len > 0) {
        ssize_t n = write(fd, data, len);
        if (n == -1) { perror("write"); return -1; }
        data += n;
        len -= n;
    }
    return 0;
}


void close_sock(int sock) {
    if (close(sock) == -1)
        perror("close");
}


void handle_client(int sock) {
    char buf[BUF_SIZE];
    int buf_used = 0;

    while (1) {
        ssize_t n = read(sock, buf + buf_used, sizeof(buf) - buf_used - 1);
        if (n == -1) { 
            perror("read");
             break; 
            }
        if (n == 0) { 
            break;
        }
        buf_used += n;

        char *line_start = buf;
        char *crlf;

        while ((crlf = (char*)memchr(line_start, '\r',
                        buf_used - (line_start - buf))) != NULL) {

            int offset = crlf - buf;
            if (offset + 1 >= buf_used) break;
            if (crlf[1] != '\n') {
                close_sock(sock); 
                return; 
            }

            int line_len = crlf - line_start;

            if (line_len > 1022) { 
                close_sock(sock);
                return;
            }

            for (int i = 0; i < line_len; i++) {
                unsigned char c = (unsigned char)line_start[i];
                if (c < 32 || c > 126) { 
                    close_sock(sock);
                    return;
                }
            }

            line_start[line_len] = '\0';
            char response[64];
            process_line(line_start, line_len, response, sizeof(response));

            char out[72];
            int  out_len = snprintf(out, sizeof(out), "%s\r\n", response);
            if (write_all(sock, out, out_len) == -1) {
                close_sock(sock);
                return;
            }

            line_start = crlf + 2;
        }

        int remaining = buf_used - (line_start - buf);
        memmove(buf, line_start, remaining);
        buf_used = remaining;

        if (buf_used >= (int)(sizeof(buf) - 1)) {
            close_sock(sock);
            return;
        }
    }

    close_sock(sock);
}


int main(void) {
    if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) {
        perror("signal SIGPIPE");
         exit(1);
    }
    if (signal(SIGCHLD, SIG_IGN) == SIG_ERR) {
        perror("signal SIGCHLD");
         exit(1);
    }

    int server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock == -1) { 
        perror("socket"); 
        exit(1); 
    }

    int opt = 1;
    if (setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        perror("setsockopt"); 
        close_sock(server_sock); 
        exit(1);
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_sock, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        perror("bind"); 
        close_sock(server_sock); 
        exit(1);
    }
    if (listen(server_sock, BACKLOG) == -1) {
        perror("listen"); 
        close_sock(server_sock); 
        exit(1);
    }

    printf("Serwer TCP palindromów nasłuchuje na porcie %d...\n", PORT);

    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);

        int client_sock = accept(server_sock,
                                 (struct sockaddr *)&client_addr, &client_len);
        if (client_sock == -1) { 
            perror("accept");
            continue;
        }

        pid_t pid = fork();
        if (pid == -1) {
            perror("fork");
            close_sock(client_sock);
            continue;
        }

        if (pid == 0) {
            close_sock(server_sock);
            handle_client(client_sock);
            exit(0);
        } else {
            close_sock(client_sock);
        }
    }
}