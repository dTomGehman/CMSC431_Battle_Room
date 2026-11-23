//*******************************************************
//client.c
//written by Tom Gehman
//
//Connect to the battleroom server, choose a name, and 
// send actions to be executed.  
//*******************************************************

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

//use ansi sequences for messages
//cyan is for messages from the server, both in stdout and the log file
#define CYAN  "\x1b[36m"
//green is for helpful messages from this program and sent messages in the log
#define GREEN  "\x1b[32m"
//red is for a couple error messages in this program
#define RED  "\x1b[31m"
//reste back to default
#define RESET "\x1b[0m"

//socket file descriptor, client's port, and boolean check, respectively
int sockfd, client_port, playing;
//log file to write to
FILE*log_f;
//strings to keep track of client and server addresses
char*serveraddrstr;
char*clientaddrstr;
//sockaddr structs for server and client
struct sockaddr_in server_addr, client_addr;
//sequence number
int sequence;
//buffers for various purposes
char seq_str[BUFFSIZE-NAMESIZE-5];
char buff[BUFFSIZE];
char name[NAMESIZE];

//*******************************************************
//receive()
//
//function to constantly wait for and write out any 
// messages given by the server.  This is to be run as a
// separate thread.  
//
//The thread is terminated by waiting until playing is 
// false and receiving a final message from the server.  
//*******************************************************
void*receive(void*arg){
    char buff2[BUFFSIZE];
    while(playing){
        //read the buffer, print it out
        bzero(buff2, BUFFSIZE);
        recvfrom(sockfd, buff2, sizeof(buff2), 0, (struct sockaddr*)NULL, NULL);
        printf(CYAN "%s" RESET "\n", buff2);
        //add an entry to the log file and flush the buffer
        fprintf(log_f, CYAN "received from %s:%d to %s:%d: " RESET "\n%s\n", serveraddrstr, PORT, clientaddrstr, client_port, buff2);
        fflush(log_f);
    }
}

//*******************************************************
//runner()
//
//main gameplay functionality.  
//
//After main() creates the socket and gets the player's
// name, this function talks to the server and gets
// the player's input to send commands.  
//
//This function is called recursively to allow replaying
// of the game on the same socket with the same name.
//*******************************************************
void runner() {
    //set sequence to be 0 to start, then count by 1 for each message sent
    sequence = 0;

    //send the connect message with your name, "a", and sequence
    sprintf(buff, "%s`a`%d", name, sequence);
    sendto(sockfd, buff, sizeof(buff), 0, (struct sockaddr*)NULL, sizeof(server_addr));

    //grab the client's address and port to use later
    socklen_t len = sizeof(client_addr);
    bzero(&client_addr, len);
    getsockname(sockfd, (struct sockaddr *) &client_addr, &len);
    inet_ntop(AF_INET, &client_addr.sin_addr, clientaddrstr, BUFFSIZE);
    client_port = ntohs(client_addr.sin_port);
    
    //write to the log file indicating that this message was sent
    fprintf(log_f, GREEN "sent from %s:%d to %s:%d: (add player)" RESET "\n%s\n", clientaddrstr, client_port, serveraddrstr, PORT, buff);
    fflush(log_f);

    //assert that we are playing the game and start the thread that
    // listens for server output.  
    playing=1;
    pthread_t thread;
    pthread_create(&thread, NULL, receive, NULL);
 
    //main loop:  receive actions from stdin and send to the server
    printf(GREEN "\nEnter actions (0/1/2) or e to exit\n");
    printf(" 0:Defend 1:Attack 2:Heal\n" RESET);
    char action;
    while(1){
        scanf(" %c", &action); 
        if (action=='e') { // e(xit).  Indicate exit to the server and break the loop
            sprintf(buff, "%s`e`%d", name, ++sequence);
            sendto(sockfd, buff, sizeof(buff), 0, (struct sockaddr*)NULL, sizeof(server_addr));
            fprintf(log_f, GREEN "sent from %s:%d to %s:%d: (exit)" RESET "\n%s\n", clientaddrstr, client_port, serveraddrstr, PORT, buff);
            fflush(log_f);
            break;
        }
 
        //otherwise, the action must be '0' '1' or '2'
        if (action < '0' || action > '2') {
            fprintf(stderr, RED "Error:  enter a single char:  0, 1, 2, or e\n" RESET);
            continue;
        } 

        //send the action with updated sequence number and write to the log
        sequence++;
        sprintf(buff, "%s`%c`%d", name, action, sequence);
        sendto(sockfd, buff, sizeof(buff), 0, (struct sockaddr*)NULL, sizeof(server_addr));
        fprintf(log_f, GREEN "sent action %c from %s:%d to %s:%d: " RESET "\n%s\n", action, clientaddrstr, client_port, serveraddrstr, PORT, buff);
        fflush(log_f);
    }

    //upon exit, wait for the thread to terminate
    playing=0;
    pthread_join(thread, NULL);

    //ask to play again and call recursively
    printf("Play again?  (y/n):\n");
    char in[10];
    fgets(in, 10, stdin); //this flushes extra chars from stdin
    fgets(in, 10, stdin); //this reads actual input
    if (!strcmp(in, "y\n")) {
        printf("ok\n");
        runner(); //start again
    }
}

//*******************************************************
//main()
//
//Main function, set up the connection, take the player
// name, make the log file, run the game, and close
// everything out when done.  
//*******************************************************
int main() {
    //allocate these buffers
    serveraddrstr = malloc(BUFFSIZE);
    clientaddrstr = malloc(BUFFSIZE);

    //read the server address in, trim newline
    bzero(&server_addr, sizeof(server_addr));
    printf(GREEN "Enter address to connect (port %d):  " RESET, PORT);
    fgets(serveraddrstr, BUFFSIZE, stdin);
    int addrlen = strlen(serveraddrstr);
    if (serveraddrstr[addrlen-1]=='\n') serveraddrstr[addrlen-1]='\0';
    
    //prepare sockaddr struct
    server_addr.sin_addr.s_addr = inet_addr(serveraddrstr);
    server_addr.sin_port = htons(PORT);
    server_addr.sin_family = AF_INET;
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    //make connection
    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        printf("error\n");
        exit(1);
    }

    //take the player name in
    printf(GREEN "Enter player name (max len is %d chars):  " RESET, NAMESIZE);
    fgets(name, sizeof(name), stdin);
    int namelen = strlen(name);
    if (namelen<2) {
        fprintf(stderr, RED "Bad name--must not be empty\n" RESET);
        close(sockfd);
        exit(1);
    }
    
    //check for backquotes in name; these are used as delimeters
    for (int x = 0; x < namelen; x++) if (name[x] == '`') {
        fprintf(stderr, RED "Bad name--please don't use backquotes (`)\n" RESET);
        close(sockfd);
        exit(1);
    }
    if (name[namelen-1]=='\n') name[namelen-1]='\0'; //trim

    //open a log file, create or overwrite
    char buff3[BUFFSIZE];
    sprintf(buff3, "%s_log", name);
    log_f = fopen(buff3, "w");

    //run the game
    runner();

    //cleanup
    free(serveraddrstr);
    free(clientaddrstr);
    close(sockfd);
    fclose(log_f);
}

