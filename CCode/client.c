#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#define PORT 9001
#define BUFFSIZE 100
#define NAMESIZE 20
#define CYAN  "\x1b[36m"
#define GREEN  "\x1b[32m"
#define RED  "\x1b[31m"
#define RESET "\x1b[0m"


int sockfd;

void*receive(void*arg){
    char buff2[BUFFSIZE];
    while(1){
        bzero(buff2, BUFFSIZE);
        recvfrom(sockfd, buff2, sizeof(buff2), 0, (struct sockaddr*)NULL, NULL);
        printf(CYAN "%s" RESET "\n", buff2);
    }
}

int main(int argc, char**argv) {
    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    //int sockfd;
    int sequence=0;
    char seq_str[BUFFSIZE-NAMESIZE-5];
    char buff[BUFFSIZE];
    char name[NAMESIZE];

    printf(GREEN "Enter address to connect (port %d):  " RESET, PORT);
    fgets(buff, sizeof(buff), stdin);
    server_addr.sin_addr.s_addr = inet_addr(buff);
    server_addr.sin_port = htons(PORT);
    server_addr.sin_family = AF_INET;
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        printf("error\n");
        exit(1);
    }

    printf(GREEN "Enter player name (max len is %d chars):  " RESET, NAMESIZE);
    fgets(name, sizeof(name), stdin);
    int namelen = strlen(name);
    if (namelen<2) {
        fprintf(stderr, RED "Bad name--must not be empty\n" RESET);
        close(sockfd);
        exit(1);
    }
    for (int x = 0; x < namelen; x++) if (name[x] == '`') {
        fprintf(stderr, RED "Bad name--please don't use backquotes (`)\n" RESET);
        close(sockfd);
        exit(1);
    }


    if (name[namelen-1]=='\n') name[namelen-1]='\0';
    sprintf(buff, "%s`a`%d", name, sequence);
    sendto(sockfd, buff, sizeof(buff), 0, (struct sockaddr*)NULL, sizeof(server_addr));

    pthread_t thread;
    pthread_create(&thread, NULL, receive, NULL);
 
    printf(GREEN "\nEnter actions (0/1/2) or e to exit\n");
    printf(" 0:Defend 1:Attack 2:Heal\n" RESET);
    char action;
    while(1){
        scanf(" %c", &action); 

        if (action=='e') {
            sprintf(buff, "%s`e`%d", name, ++sequence);
            sendto(sockfd, buff, sizeof(buff), 0, (struct sockaddr*)NULL, sizeof(server_addr));
            break;
        }
 
        if (action < '0' || action > '2') {
            fprintf(stderr, RED "Error:  enter a single char:  0, 1, 2, or e\n" RESET);
            continue;
        } sequence++;

        sprintf(buff, "%s`%c`%d", name, action, sequence);

        sendto(sockfd, buff, sizeof(buff), 0, (struct sockaddr*)NULL, sizeof(server_addr));
    }
    pthread_join(thread, NULL);
    close(sockfd);
    
}
