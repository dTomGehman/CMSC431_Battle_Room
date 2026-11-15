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

void parse_input(char*buffer, char*player_name, char*action, int*seq){
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
    printf("\n");
    if (players->head == NULL) return;
    struct node*work = players->head;
    while (work != NULL) {
        printf("%s ", work->player_name);
        work = work->next;
    }
    printf("\n\n");
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

    //create room
    room = malloc(sizeof(struct Room));
    initialize_room(room,1.0,20000);

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

    pthread_t th_run_game;
    pthread_create(&th_run_game, NULL, run_game, NULL);

    //receive data
    char buffer[100];
    char player_name[NAMELEN]; //was namelen+1
    char action;
    int seq;
    len = sizeof(client_addr);
    while (1){
        int n = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr*)&client_addr, &len);
        buffer[n] = '\0';
        parse_input(buffer, player_name, &action, &seq);
        if (action=='a') {
            struct Entity *me = malloc(sizeof(struct Entity));
            initialize_entity(me,player_name,100,100,10,0.6,1000,5000);
            add_player(room, me);
            list_add_player(&players, player_name, 0,  me);
            printf("%s Has joined the room\n", player_name);
        } else if (action=='e'){
            list_remove_player(&players, player_name);
            printf("%s Has left the room\n", player_name);
        } else {
            struct Entity*me = list_get_player(&players, player_name);
            set_action(me, action-'0');
        }

        printf("Received %c from %s, seq %d\n", action, player_name, seq);
        list_print_players(&players);
    }

    pthread_join(th_run_game, NULL);
    exit(0);

}
