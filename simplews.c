/*
* Simple Web Server
*
* file: simplews.c
*
* Created by Jorge Conde Gomez Llanos.
* V00723209
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
socklen_t cli_len;
char buffer[MAXBUFLEN]; // data buffer


int ready(int socketfd);
int startServer();
int readCommandLine(int c, char* a[]);
void respond(int numbytes);


int main(int argc, char* argv[]){

    int numbytes;

    

    if(readCommandLine(argc, argv) != 0) return -1;    

    if(startServer() != 0) return -1;    
    

    
    while(1){

        printf("sws: waiting to recvfrom...\n");
        cli_len = sizeof(cli_addr);

	int ret = ready(socketfd);

        if(ret == 1)
            exit(1);
    
        if ((numbytes = recvfrom(socketfd, buffer, MAXBUFLEN-1 , 0,
                             (struct sockaddr *)&cli_addr, &cli_len)) == -1) {
            perror("sws: error on recvfrom()!");
            return -1;
        }
    

    	respond(numbytes);
	


        /*if ((numbytes = sendto(socketfd, buffer, strlen(buffer), 0,
                (struct sockaddr *) &cli_addr, cli_len)) == -1) {
            perror("sws: error in sendto()");
            return -1;
        }*/
    }
    
    close(socketfd);
    
    return 0;
}

void respond(int numbytes){

    char *request[3];

    printf("sws: received packet from IP: %s and Port: %d\n", inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));
    printf("listener: received packet is %d bytes long\n", numbytes);
    buffer[numbytes] = '\0';
    printf("listener: packet contains \"%s\" \n", buffer);

    request[0] = strtok(buffer," \t\n");
    if(strncmp(request[0], "GET\0", 4)==0){
        printf("YEI\n\n\n");
    }

	

}

int startServer(){

    /* Clear the used structure */
    bzero((char *) &serv_addr, sizeof(serv_addr));
    
    /* Prepare address information for server side */
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(server_portno);


    /* Create a socket type UDP */
    if ((socketfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1){
        perror("sws: error on socket()");
        return -1;
    }
    
    /* Bind the socket with the address information */
    if (bind(socketfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        close(socketfd);
        perror("sws: error on binding!");
        return -1;
    }

    printf("sws is running on UDP port %d and serving %s\n",server_portno,root_directory);
    printf("press 'q' to quit ...\n");

    return 0;
}

int readCommandLine(int c, char* a[]){


    /* Check command line arguments */
    if (c != 3) {
        printf("Syntax error: Usage is './sws <port> <directory>'\n\n");
        return -1;
    }
    else{
        server_portno = atoi(a[1]); // Read the port number value
        if (server_portno < 0) {
            printf("Syntax error: the port number is invalid.\n");
            printf("Usage is './sws <port> <directory>'\n\n");
            return -1;
        }
        
        root_directory = a[2];

        if (root_directory[0]=='.' && root_directory[1]=='.' && root_directory[2]=='/') {
            printf("Syntax error: invalid directory request.\n");
            printf("Usage is './sws <port> <directory>'\n\n");
            return -1;
        }
    }

    return 0;
}

int ready(int socketfd){

    while (1){
        char read_buffer[MAXBUFLEN];
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO, &readfds);
        FD_SET(socketfd, &readfds);
        int retval = select(socketfd+1, &readfds, NULL, NULL, NULL);
        if(retval <=0) //error or timeout
            return retval;
        else
        {
            if(FD_ISSET(STDIN_FILENO, &readfds) &&
               (fgets(read_buffer, MAXBUFLEN, stdin) != NULL) && 
                strchr(read_buffer, 'q') != NULL)  // 'q' pressed
                return 1;
            else if(FD_ISSET(socketfd, &readfds))   // recv buffer ready
                return 2;
        }
    }
    return -1;
}
