//*******************************************************
//server.c
//written by Tom Gehman
//
//Start up a battleroom server that many udp clients can
// connect to.  Run the game as a separate thread and
// process messages to make changes in the game
//*******************************************************

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
#define BUFFSIZE 100

//use ansi to indicate certain messages
//red is for some errors
#define RED "\e[0;31m"
//green is for some helpful messages and the client's messages in the log file
#define GREEN "\e[0;32m"
//cyan is to indicate messages written by the game thread (room.c) and server messages in the log file
#define CYAN "\e[0;36m"
//reset.  Other helpful messages are written in light gray (default)
#define RESET "\e[0m"

//type that contains the game room
struct Room *room;
//log file
FILE*log_f;

//playerlist contains a list of players with extra 
// attributes not includedin the entity struct
// in retrospect, this was not entirely necessary, 
// but it allows a nice way to work with players in
// the main thread
//
//A linked list is used here, and the game room 
// (second thread) uses a resizeable array.
// This list reflects which players are actively in
// the room; the other array has all players (even 
// those who have left, but their ghost is still in)
struct node { //each node in the playerlist
    char player_name[NAMELEN];
    int seq;
    struct node *next;
    struct Entity *me;
};
struct playerlist {
    struct node *head;
};


//*******************************************************
//run_game()
//
//A second thread that simply starts the room.
//*******************************************************
void*run_game(void*arg) {
    run(room);
}

//*******************************************************
//parse_input()
//
//parse message received from client.  This makes a copy
// of the buffer and does not modify it.  It modifies
// the player_name, action, and seq instead of returning
// output.
//*******************************************************
void parse_input(char*buffer, char*player_name, char*action, int*seq){
    //make a copy
    char temp[BUFFSIZE];
    memcpy(temp, buffer, BUFFSIZE);
    //message consists of NAME`ACTION`SEQ
    char*str = strtok(temp, "`");
    strncpy(player_name, str, NAMELEN);
    str = strtok(NULL, "`");
    *action = str[0];
    str = strtok(NULL, "`");
    *seq = atoi(str);
}

//*******************************************************
//list_add_player()
//
//Add a new player--a node--to the playerlist
//*******************************************************
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

//*******************************************************
//list_print_players()
//
//print out the contents of the playerlist, for debugging
//*******************************************************
void list_print_players(struct playerlist*players){
    if (players->head == NULL) return;
    struct node*work = players->head;
    while (work != NULL) {
        printf("%s ", work->player_name);
        work = work->next;
    }
    printf("\n");
}

//*******************************************************
//list_get_player()
//
//search for a player's name and return its node
//*******************************************************
struct Entity*list_get_player(struct playerlist*players, char*player_name) {
    struct node*work = players->head;
    while (work != NULL) {
        if (!strcmp(player_name, work->player_name)) {
            return work->me;
        }
        work = work->next;
    }
    return NULL;
}

//*******************************************************
//list_get_set_seq()
//
//get a player's sequence number by their name and return
// the old sequence number.  This is to allow a check
// without looking up the player twice.  
//*******************************************************
int list_get_set_seq(struct playerlist*players, char*player_name, int newseq) {
    struct node*work = players->head;
    while (work != NULL) {
        if (!strcmp(player_name, work->player_name)) {
            int temp = work->seq;
            work->seq = newseq;
            return temp; 
        }
        work = work->next;
    }
    return -1;
}
 

//*******************************************************
//list_remove_player()
//
//remove a node from the list.  This happens when a 
// player sends disconnect ("e").  This however does not
// remove the player from the game, and they can re-join.
// Their "ghost" continues to fight until dead, even 
// though no more actions can be sent unless rejoined.  
//*******************************************************
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


//*******************************************************
//main()
//
//Set up the server and run the game!
//*******************************************************
int main(int argc, char**argv) {
    printf(GREEN "Starting game, waiting for players to connect\n" RESET);
    
    //open the log file
    log_f = fopen("server_log", "w");

    //create player list
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
    //added sockfd to the room so it can use the same socket
    //the room also sends messages now (cyan)
    room = malloc(sizeof(struct Room));
    initialize_room(room,1.0,20000, sockfd, log_f);

    //thread to run the game
    //this handles the existing game function and all messages sent back to players
    //this second thread only sends messages and does not receive any
    //the main thread only receives messages and does not send any (with one exception)
    pthread_t th_run_game;
    pthread_create(&th_run_game, NULL, run_game, NULL);

    //receive data.  this thread handles all the messages received by the server
    //messages include joining the game, sending actions, and exiting the game
    char buffer[BUFFSIZE];
    char player_name[NAMELEN];
    char action;
    int seq;
    char serveraddrstr[BUFFSIZE], clientaddrstr[BUFFSIZE];
    inet_ntop(AF_INET, &server_addr.sin_addr, serveraddrstr, BUFFSIZE);
    len = sizeof(client_addr);
    while (1){
        //receive the packet and parse into the name, action, and seq
        int n = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr*)&client_addr, &len);
        buffer[n] = '\0';
        parse_input(buffer, player_name, &action, &seq);

        //see if packet is out of order or not; if so, print warning
        int testseq = list_get_set_seq(&players, player_name, seq);
        if (testseq+1 < seq) printf(RED "Warning, missing packet from %s\n" RESET, player_name);
        else if (testseq+1 > seq) printf(RED "Warning, out-of-order packet from %s\n" RESET, player_name);

        //grab the client's port and address for logging purposes
        int clientport = ntohs(client_addr.sin_port);
        inet_ntop(AF_INET, &client_addr.sin_addr, clientaddrstr, BUFFSIZE);

        if (action=='a') { //a(dd) new player
            struct Entity *me = malloc(sizeof(struct Entity));
            struct sockaddr_in*copy = malloc(sizeof(client_addr)); //remember to free this later
            memcpy(copy, &client_addr, sizeof(client_addr));
            //I added the client connection info to this data for ease 
            // of sending info back to client from the room
            initialize_entity(me,player_name,100,100,10,0.6,1000,5000, copy, sizeof(client_addr));
            add_player(room, me);

            //add the player to the playerlist as well
            list_add_player(&players, player_name, seq,  me);

            //write helpful messages to log and stdout
            fprintf(log_f, GREEN "Received join on %s:%d from %s:%d as %s:\n" RESET "%s\n", serveraddrstr, PORT, clientaddrstr, clientport, player_name, buffer);
            fflush(log_f);
            printf("%s Has joined the room\nActive players include:  ", player_name);
            list_print_players(&players);
            printf("\n");

        } else if (action=='e'){ //e(xit)
            //note:  this removes the player from this thread's list (perhaps temporarily)
            //but the other thread will keep the player in the game as a ghost of sorts
            list_remove_player(&players, player_name);

            //write helpful messages to log and stdout
            printf("%s Has left the room\nRemaining players include:  ", player_name);
            list_print_players(&players);
            printf("\n");
            fprintf(log_f, GREEN "Received exit on %s:%d from %s:%d as %s:\n" RESET "%s\n", serveraddrstr, PORT, clientaddrstr, clientport, player_name, buffer);
            fflush(log_f);

            //hack to terminate client thread waiting for one last message
            //just send a blank message to the client to ensure that 
            // the thread caling recvfrom() terminates
            //This is the exception mentioned above; 
            // otherwise this thread sends no udp messages
            char*m = "\n";
            sendto(sockfd, m, strlen(m), 0, (struct sockaddr*)&client_addr, sizeof(client_addr));
            sendto(sockfd, m, strlen(m), 0, (struct sockaddr*)&client_addr, sizeof(client_addr));
            
            //write log
            fprintf(log_f, CYAN "Sent `\\n` from %s:%d to %s:%d:\n" RESET "\n\n", serveraddrstr, PORT, clientaddrstr, clientport);
            fflush(log_f);

        } else { //action must be '0' '1' or '2'
            //write log
            fprintf(log_f, GREEN "Received action %c on %s:%d from %s:%d as %s:\n" RESET "%s\n", action, serveraddrstr, PORT, clientaddrstr, clientport, player_name, buffer);
            fflush(log_f);

            //send the action.  Thread safe
            struct Entity*me = list_get_player(&players, player_name);
            set_action(me, action-'0');
        }

        //for debugging, helpful to see
        printf("Received action %c from %s, seq %d\n", action, player_name, seq);
    }

    //cleanup -- shouldn't be reached; program should be terminated by C-c
    pthread_join(th_run_game, NULL);
    fclose(log_f);
    exit(0);

}
