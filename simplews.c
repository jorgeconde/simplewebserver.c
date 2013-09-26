/*
 *  Simple Web Server
 *  
 *  file: simplews.c
 *
 *  Created by Jorge Conde Gomez Llanos.
 *  V00723209
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
 
#define MAXBUFLEN 256

int socketfd, server_portno;
char *root_directory;
struct sockaddr_in serv_addr, cli_addr;


int main(int argc, char* argv[]){
    
    socklen_t cli_len;
    int numbytes;
    char buffer[MAXBUFLEN]; // data buffer
    
    /* Check command line arguments */
    if (argc != 3) {
        printf("Syntax error: Usage is './sws <port> <directory>'\n\n");
        return -1;
    }
    else{
        server_portno = atoi(argv[1]);  // Read the port number value
        if (server_portno < 0) {
            printf("Syntax error: the port number is invalid.\n");
            printf("Usage is './sws <port> <directory>'\n\n");
            return -1;
        }
        
        root_directory = argv[2];

        if (root_directory[0]=='.' && root_directory[1]=='.' && root_directory[2]=='/') {
            printf("Syntax error: invalid directory request.\n");
            printf("Usage is './sws <port> <directory>'\n\n");
            return -1;
        }
    }
    
    /* Create a socket type UDP */
    if ((socketfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1){
        perror("sws: error on socket()");
        return -1;
    }
    
    
    /* Clear the used structure */
    bzero((char *) &serv_addr, sizeof(serv_addr));
    
    /* Prepare address information for server side */
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(server_portno);
    
    /* Bind the socket with the address information */
    if (bind(socketfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        close(socketfd);
        perror("sws: error on binding!");
        return -1;
    }
    
    
    printf("sws: waiting to recvfrom...\n");
    cli_len = sizeof(cli_addr);
    
    if ((numbytes = recvfrom(socketfd, buffer, MAXBUFLEN-1 , 0,
                             (struct sockaddr *)&cli_addr, &cli_len)) == -1) {
        perror("sws: error on recvfrom()!");
        return -1;
    }
    
    printf("sws is running on UDP port %d and serving %s\n",server_portno,root_directory);
    printf("press 'q' to quit ...\n");
    
    
    if ((numbytes = sendto(socketfd, buffer, strlen(buffer), 0,
            (struct sockaddr *) &cli_addr, cli_len)) == -1) {
        perror("sws: error in sendto()");
        return -1;
    }
    
    close(socketfd);
    
    return 0;

}
