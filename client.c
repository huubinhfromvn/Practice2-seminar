#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8080
#define BUF_SIZE 1024

int main() {
    int sockfd;
    struct sockaddr_in server_addr;
    char buffer[BUF_SIZE] = {0};

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

    connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    printf("Connected to server.\n");

    // Nhận danh sách file
    recv(sockfd, buffer, BUF_SIZE, 0);
    printf("Available files:\n%s\n", buffer);

    // Gửi tên file muốn tải
    char filename[128];
    printf("Enter file name to download: ");
    fgets(filename, sizeof(filename), stdin);
    filename[strcspn(filename, "\n")] = 0;
    send(sockfd, filename, strlen(filename), 0);

    // Nhận phản hồi
    char status[8] = {0};
    recv(sockfd, status, sizeof(status), 0);
    if (strcmp(status, "OK") != 0) {
        printf("File not found on server.\n");
        close(sockfd);
        return 1;
    }

    // Nhận kích thước file
    int filesize;
    recv(sockfd, &filesize, sizeof(filesize), 0);

    // Nhận file và ghi
    char path[128];
    snprintf(path, sizeof(path), "downloaded/%s", filename);
    FILE *fp = fopen(path, "wb");
    int received = 0;
    while (received < filesize) {
        int bytes = recv(sockfd, buffer, BUF_SIZE, 0);
        fwrite(buffer, 1, bytes, fp);
        received += bytes;
    }
    fclose(fp);

    printf("File downloaded: %s (%d bytes)\n", path, filesize);
    close(sockfd);
    return 0;
}
