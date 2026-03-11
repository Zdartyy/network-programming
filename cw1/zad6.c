#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#define BUFFER_SIZE 4096

int main(int argc, char *argv[]) {

    if (argc != 3) {
        fprintf(stderr, "Usage: %s source_file destination_file\n", argv[0]);
        exit(1);
    }

    char *source_name = argv[1];
    char *dest_name   = argv[2];


    int source_file = open(source_name, O_RDONLY);
    if (source_file == -1) {
        perror(source_name);
        exit(1);
    }

    int dest_file = open(dest_name, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (dest_file == -1) {
        perror(dest_name);
        close(source_file);
        exit(1);
    }

    char    buffer[BUFFER_SIZE];
    ssize_t bytes_read;

    while ((bytes_read = read(source_file, buffer, BUFFER_SIZE)) > 0) {

        ssize_t bytes_written = write(dest_file, buffer, bytes_read);
        if (bytes_written == -1) {
            perror(dest_name);
            close(source_file);
            close(dest_file);
            exit(1);
        }
    }

    if (bytes_read == -1) {
        perror(source_name);
        close(source_file);
        close(dest_file);
        exit(1);
    }

    close(source_file);
    close(dest_file);

    printf("Copied: %s -> %s\n", source_name, dest_name);
    return 0;
}
