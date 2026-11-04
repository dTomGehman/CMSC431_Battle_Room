#include <stdio.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <unistd.h>

#define PORT 9001
#define BUFFSIZE 100

int main(int argc, char**argv) {
    struct sockaddr_in server_addr;
    int sockfd;
    bzero(&server_addr, sizeof(server_addr));
    char buff[BUFFSIZE];
    printf("Enter address to connect (port %d):  ", PORT);
    fgets(buff, sizeof(buff), stdin);
    server_addr.sin_addr.s_addr = inet_addr(buff);
    server_addr.sin_port = htons(PORT);
    server_addr.sin_family = AF_INET;
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        printf("error\n");
        exit(1);
    }

    while(1){
        printf("Enter message:  ");
        fgets(buff, sizeof(buff), stdin);
        sendto(sockfd, buff, sizeof(buff), 0, (struct sockaddr*)NULL, sizeof(server_addr));
    }
    close(sockfd);
}
