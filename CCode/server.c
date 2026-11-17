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
#define NAMELEN 20

struct Room *room;

struct node {
    char player_name[NAMELEN];
    int seq;
    struct node *next;
    struct Entity *me;
};

struct playerlist {
    struct node *head;
};

void*run_game(void*arg) {
    run(room);
}

//parse message received from client
void parse_input(char*buffer, char*player_name, char*action, int*seq){
    //message consists of NAME`ACTION`SEQ
    char*str = strtok(buffer, "`");
    strncpy(player_name, str, NAMELEN);
    str = strtok(NULL, "`");
    *action = str[0];
    str = strtok(NULL, "`");
    *seq = atoi(str);
}

void list_add_player(struct playerlist*players, char*player_name, int seq, struct Entity *me){
    struct node*new = malloc(sizeof(struct node));
    if (new == NULL) {fprintf(stderr, "malloc fail\n"); return;}

    new -> seq = seq;
    strcpy(new->player_name, player_name);
    new -> next = NULL;
    new -> me = me;

    struct node*end = players->head;
    if (end==NULL) {players->head = new; return;}
    while (end->next != NULL) end = end->next;
    end->next = new;
}

void list_print_players(struct playerlist*players){
//    printf("\n");
    if (players->head == NULL) return;
    struct node*work = players->head;
    while (work != NULL) {
        printf("%s ", work->player_name);
        work = work->next;
    }
    printf("\n");
}

struct Entity*list_get_player(struct playerlist*players, char*player_name) {
//    if (players->head == NULL) return NULL;
    struct node*work = players->head;
    while (work != NULL) {
        if (!strcmp(player_name, work->player_name)) {
            return work->me;
        }
        work = work->next;
    }
    return NULL;
}


void list_remove_player(struct playerlist*players, char*player_name) {
    if (players->head == NULL) return;
    struct node*work = players->head;
    if (!strcmp(player_name, work->player_name)) {
        players->head = work->next;
        //don't free this data in case game is still using it
        //if (work->me) free(work->me);
        free(work);
        return;
    }
    while (work->next != NULL) {
        if (!strcmp(player_name, work->next->player_name)) {
            struct node*temp = work->next;
            work->next = work->next->next;
            //don't free this data in case game is still using it
            //if (temp->me) free(temp->me);
            free(temp);
            return;
        }
        work = work->next;
    }
}


int main(int argc, char**argv) {

    //create list
    struct playerlist players;
    players.head = NULL;

    //create socket
    int sockfd, len;
    struct sockaddr_in server_addr, client_addr;
    bzero(&server_addr, sizeof(server_addr));
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(PORT);
    server_addr.sin_family = AF_INET;
    bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr));

    //create room
    room = malloc(sizeof(struct Room));
    //added sockfd to the room so it can use the same socket
    initialize_room(room,1.0,20000, sockfd);

    //thread to run the game
    //this handles the existing game function and all messages sent back to players
    pthread_t th_run_game;
    pthread_create(&th_run_game, NULL, run_game, NULL);

    //receive data.  this thread handles all the messages received by the server
    //messages include joining the game, sending actions, and exiting the game
    char buffer[100];
    char player_name[NAMELEN]; //was namelen+1
    char action;
    int seq;
    len = sizeof(client_addr);
    while (1){
        int n = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr*)&client_addr, &len);
//        printf("received from %s\n", inet_ntoa(client_addr.sin_addr));
        buffer[n] = '\0';
        parse_input(buffer, player_name, &action, &seq);

//        char*m = "test test received\n";
//        sendto(sockfd, m, strlen(m), 0, (struct sockaddr*)&client_addr, sizeof(client_addr));

        if (action=='a') {
            struct Entity *me = malloc(sizeof(struct Entity));
            struct sockaddr_in*copy = malloc(sizeof(client_addr)); //remember to free this later
            memcpy(copy, &client_addr, sizeof(client_addr));
            //I added the client connection info to this data for ease of sending info back to client
            initialize_entity(me,player_name,100,100,10,0.6,1000,5000, copy, sizeof(client_addr));
            add_player(room, me);
            list_add_player(&players, player_name, 0,  me);
            printf("%s Has joined the room\nPlayers include:  ", player_name);
            list_print_players(&players);
        } else if (action=='e'){
            //note:  this removes the player from this thread's list
            //but the other thread will keep the player in the game as a ghost of sorts
            //the original game doesn't have a clean way to remove players; I might work on that
            list_remove_player(&players, player_name);
            printf("%s Has left the room\nPlayers include:  ", player_name);
            //later I will ensure that dead players can rejoin; haven't decided on implementation
        } else {
            //send the action
            struct Entity*me = list_get_player(&players, player_name);
            set_action(me, action-'0');
        }

        //for debugging
        printf("Received action %c from %s, seq %d\n", action, player_name, seq);
    }

    pthread_join(th_run_game, NULL);
    exit(0);

}
