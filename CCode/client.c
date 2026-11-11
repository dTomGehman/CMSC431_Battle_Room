#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <unistd.h>

#define PORT 9001
#define BUFFSIZE 100
#define NAMESIZE 20

int main(int argc, char**argv) {
    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    int sockfd;
    int sequence=0;
    char seq_str[BUFFSIZE-NAMESIZE-5];
    char buff[BUFFSIZE];
    char name[NAMESIZE];

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

    printf("Enter player name (max len is %d chars):  ", NAMESIZE);
    fgets(name, sizeof(name), stdin);
    int namelen = strlen(name);
    if (namelen<2) {
        fprintf(stderr, "Bad name--must not be empty\n");
        close(sockfd);
        exit(1);
    }
    for (int x = 0; x < namelen; x++) if (name[x] == '`') {
        fprintf(stderr, "Bad name--please don't use backquotes (`)\n");
        close(sockfd);
        exit(1);
    }


    if (name[namelen-1]=='\n') name[namelen-1]='\0';
    sprintf(buff, "%s`a`%d", name, sequence);
    sendto(sockfd, buff, sizeof(buff), 0, (struct sockaddr*)NULL, sizeof(server_addr));
 
    printf("\nEnter actions (0/1/2) or e to exit\n");
    printf(" 0:Defend 1:Attack 2:Heal\n");
    char action;
    while(1){
        scanf(" %c", &action); 

        if (action=='e') {
            sprintf(buff, "%s`e`%d", name, ++sequence);
            sendto(sockfd, buff, sizeof(buff), 0, (struct sockaddr*)NULL, sizeof(server_addr));
            break;
        }
 
        if (action < '0' || action > '2') {
            fprintf(stderr, "Error:  enter a single char:  0, 1, 2, or e\n");
            continue;
        }
        sequence++;

        sprintf(buff, "%s`%c`%d", name, action, sequence);

        sendto(sockfd, buff, sizeof(buff), 0, (struct sockaddr*)NULL, sizeof(server_addr));
    }
    close(sockfd);
}
