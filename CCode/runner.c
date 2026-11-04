#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include "room.h"

#define PORT 9001

struct Room *room;

void*run_game(void*arg) {
    run(room);
}

int main(int argc, char**argv) {

    //create room
    room = malloc(sizeof(struct Room));
    initialize_room(room,1.0,20000);

    //create socket
    int sockfd, len;
    struct sockaddr_in server_addr, client_addr;
    bzero(&server_addr, sizeof(server_addr));
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(PORT);
    server_addr.sin_family = AF_INET;
    bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr));

    //test receiving data
    char buffer[100];
    len = sizeof(client_addr);
    while (1){
        int n = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr*)&client_addr, &len);
        buffer[n] = '\0';
        printf("%s\n", buffer);
    }

    exit(0);

    struct Entity *player1 = malloc(sizeof(struct Entity));
    char *p_name = malloc(8*sizeof(char));
    p_name[0] = '\0';
    strcpy(p_name,"Player1");
    initialize_entity(player1,p_name,100,100,10,0.6,1000,5000);
    set_action(player1,2);
    add_player(room,player1);

    struct Entity *player2 = malloc(sizeof(struct Entity));
    player2 = malloc(sizeof(struct Entity));
    p_name = malloc(8*sizeof(char));
    p_name[0] = '\0';
    strcpy(p_name,"Player2");
    initialize_entity(player2,p_name,100,100,10,0.6,1000,5000);
    set_action(player2,2);
    add_player(room,player2);

    pthread_t th_run_game;
//    pthread_create(&th_run_game, NULL, run_game, NULL);

    char buff[30];
    int action, tmp_player;

/*    while (1) {
        fgets(buff, sizeof(buff), stdin);
        tmp_player = atoi(buff);
        fgets(buff, sizeof(buff), stdin);
        action = atoi(buff);
        set_action(tmp_player==1?player1:player2, action);
    }*/
        

//    pthread_join(th_run_game, NULL);
}
