#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8080
#define BUF_SIZE 1024
#define BIN_DIR "bin_files/"

void send_file_list(int client_sock) {
    DIR *d = opendir(BIN_DIR);
    struct dirent *dir;
    char buffer[BUF_SIZE] = "";

    while ((dir = readdir(d)) != NULL) {
        if (dir->d_type == DT_REG) {
            strcat(buffer, dir->d_name);
            strcat(buffer, "\n");
        }
    }
    closedir(d);
    send(client_sock, buffer, strlen(buffer), 0);
}

void send_file(int client_sock, char *filename) {
    char fullpath[256];
    snprintf(fullpath, sizeof(fullpath), BIN_DIR "%s", filename);
    FILE *fp = fopen(fullpath, "rb");
    if (!fp) {
        send(client_sock, "ERROR", 5, 0);
        return;
    }

    send(client_sock, "OK", 2, 0);
    fseek(fp, 0, SEEK_END);
    int filesize = ftell(fp);
    rewind(fp);
    send(client_sock, &filesize, sizeof(filesize), 0);

    char buffer[BUF_SIZE];
    int bytes;
    while ((bytes = fread(buffer, 1, BUF_SIZE, fp)) > 0) {
        send(client_sock, buffer, bytes, 0);
    }
    fclose(fp);
}

int main() {
    int server_fd, client_sock;
    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(addr);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);

    bind(server_fd, (struct sockaddr*)&addr, sizeof(addr));
    listen(server_fd, 5);
    printf("Server ready on port %d...\n", PORT);

    while (1) {
        client_sock = accept(server_fd, (struct sockaddr*)&addr, &addr_len);
        printf("Client connected\n");

        send_file_list(client_sock);

        char filename[128] = {0};
        recv(client_sock, filename, sizeof(filename), 0);
        send_file(client_sock, filename);

        close(client_sock);
    }

    close(server_fd);
    return 0;
}
