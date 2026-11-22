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


int sockfd, client_port, playing;
FILE*log_f;
char*serveraddrstr;
char*clientaddrstr;
struct sockaddr_in server_addr, client_addr;

int sequence;
char seq_str[BUFFSIZE-NAMESIZE-5];
char buff[BUFFSIZE];
char name[NAMESIZE];

void*receive(void*arg){
    char buff2[BUFFSIZE];
    while(playing){
        bzero(buff2, BUFFSIZE);
        recvfrom(sockfd, buff2, sizeof(buff2), 0, (struct sockaddr*)NULL, NULL);
        printf(CYAN "%s" RESET "\n", buff2);
        fprintf(log_f, CYAN "received from %s:%d to %s:%d: " RESET "\n%s\n", serveraddrstr, PORT, clientaddrstr, client_port, buff2);
        fflush(log_f);
    }
}


void runner() {
    sequence = 0;

    sprintf(buff, "%s`a`%d", name, sequence);
    sendto(sockfd, buff, sizeof(buff), 0, (struct sockaddr*)NULL, sizeof(server_addr));

    socklen_t len = sizeof(client_addr);
    bzero(&client_addr, len);
    getsockname(sockfd, (struct sockaddr *) &client_addr, &len);
    inet_ntop(AF_INET, &client_addr.sin_addr, clientaddrstr, BUFFSIZE);
    client_port = ntohs(client_addr.sin_port);
    
    fprintf(log_f, GREEN "sent from %s:%d to %s:%d: (add player)" RESET "\n%s\n", clientaddrstr, client_port, serveraddrstr, PORT, buff);
    fflush(log_f);

    playing=1;

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
            fprintf(log_f, GREEN "sent from %s:%d to %s:%d: (exit)" RESET "\n%s\n", clientaddrstr, client_port, serveraddrstr, PORT, buff);
            fflush(log_f);
            break;
        }
 
        if (action < '0' || action > '2') {
            fprintf(stderr, RED "Error:  enter a single char:  0, 1, 2, or e\n" RESET);
            continue;
        } sequence++;

        sprintf(buff, "%s`%c`%d", name, action, sequence);

        sendto(sockfd, buff, sizeof(buff), 0, (struct sockaddr*)NULL, sizeof(server_addr));

        fprintf(log_f, GREEN "sent action %c from %s:%d to %s:%d: " RESET "\n%s\n", action, clientaddrstr, client_port, serveraddrstr, PORT, buff);
        fflush(log_f);
    }

    playing=0;
    pthread_join(thread, NULL);

    printf("Play again?  (y/n):\n");
    char in[10];
    fgets(in, 10, stdin);
    fgets(in, 10, stdin);
    if (!strcmp(in, "y\n")) {
        printf("ok\n");
        runner();
    }
}

int main() {
    bzero(&server_addr, sizeof(server_addr));
    //int sockfd;
    serveraddrstr = malloc(BUFFSIZE);
    clientaddrstr = malloc(BUFFSIZE);
    printf(GREEN "Enter address to connect (port %d):  " RESET, PORT);
    fgets(serveraddrstr, BUFFSIZE, stdin);
    
    int addrlen = strlen(serveraddrstr);
    if (serveraddrstr[addrlen-1]=='\n') serveraddrstr[addrlen-1]='\0';
    
    server_addr.sin_addr.s_addr = inet_addr(serveraddrstr);
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

    char buff3[BUFFSIZE];
    sprintf(buff3, "%s_log", name);
    log_f = fopen(buff3, "w");

    runner();


    close(sockfd);
    fclose(log_f);
}
