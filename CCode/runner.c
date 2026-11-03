#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "pthread.h"

#include "room.h"

struct Room *room;

void*run_game(void*arg) {
    run(room);
}

int main(int argc, char**argv) {

    room = malloc(sizeof(struct Room));
    initialize_room(room,1.0,20000);

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
    pthread_create(&th_run_game, NULL, run_game, NULL);

    char buff[30];
    int action, tmp_player;

    while (1) {
        fgets(buff, sizeof(buff), stdin);
        tmp_player = atoi(buff);
        fgets(buff, sizeof(buff), stdin);
        action = atoi(buff);
        set_action(tmp_player==1?player1:player2, action);
    }
        

    pthread_join(th_run_game, NULL);
}
