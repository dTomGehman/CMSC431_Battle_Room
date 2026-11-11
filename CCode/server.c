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
    char player_name[NAMELEN+1];
    int seq;
    struct node *next;
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

void list_add_player(struct playerlist*players, char*player_name, int seq){
    struct node*new = malloc(sizeof(struct node));
    if (new == NULL) {fprintf(stderr, "malloc fail\n"); return;}

    new -> seq = seq;
    strcpy(new->player_name, player_name);
    new -> next = NULL;

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

void list_remove_player(struct playerlist*players, char*player_name) {
    if (players->head == NULL) return;
    struct node*work = players->head;
    if (!strcmp(player_name, work->player_name)) {
        players->head = work->next;
        free(work);
        return;
    }
    while (work->next != NULL) {
        if (!strcmp(player_name, work->next->player_name)) {
            struct node*temp = work->next;
            work->next = work->next->next;
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

    printf("nothing\n");
    list_print_players(&players);

    list_add_player(&players, "Tom", 0);
    printf("add tom\n");
    list_print_players(&players);

    list_add_player(&players, "Daniel", 0);
    printf("add daniel\n");
    list_print_players(&players);

    list_add_player(&players, "Bryton", 0);
    printf("add bryton\n");
    list_print_players(&players);

    list_remove_player(&players, "Daniel");
    printf("remove daniel\n");
    list_print_players(&players);

    printf("remove Joe (nonexistent)\n");
    list_remove_player(&players, "Joe");
    list_print_players(&players);

    list_remove_player(&players, "Tom");
    printf("remove tom\n");
    list_print_players(&players);

    list_add_player(&players, "Daniel", 0);
    printf("add daniel\n");
    list_print_players(&players);

    list_remove_player(&players, "Daniel");
    printf("remove daniel\n");
    list_print_players(&players);

    list_remove_player(&players, "Bryton");
    printf("remove Bryton\n");
    list_print_players(&players);

    list_remove_player(&players, "Daniel");
    printf("remove daniel (empty)\n");
    list_print_players(&players);

    list_add_player(&players, "Tom", 0);
    printf("add tom\n");
    list_print_players(&players);

    exit(0);


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
    char player_name[NAMELEN+1];
    char action;
    int seq;
    len = sizeof(client_addr);
    while (1){
        int n = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr*)&client_addr, &len);
        buffer[n] = '\0';
        parse_input(buffer, player_name, &action, &seq);
        printf("Received %c from %s, seq %d\n", action, player_name, seq);
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
    //int action, tmp_player;

/*    while (1) {
        fgets(buff, sizeof(buff), stdin);
        tmp_player = atoi(buff);
        fgets(buff, sizeof(buff), stdin);
        action = atoi(buff);
        set_action(tmp_player==1?player1:player2, action);
    }*/
        

//    pthread_join(th_run_game, NULL);
}
