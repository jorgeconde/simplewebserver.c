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
#include <fcntl.h>
 
#define MAXBUFLEN 256
#define BYTES 1024

int socketfd, server_portno;
char *root_directory;
struct sockaddr_in serv_addr, cli_addr;
socklen_t cli_len;
char buffer[MAXBUFLEN]; // data buffer


void setReusable(int socket);
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
	
    }
    
    close(socketfd);
    
    return 0;
}

void setReusable(int socket) {
	int opt = 1;

	if (setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0) {
		printf("Socket error: Unable to set server socket %d reusable \n", socket);
	}

	return;
}

void respond(int numbytes){

    char *request[3];
    int i=0, count=1;
    int fd, bytes_read;
    char path[999], data_to_send[BYTES];
    int length = strlen(buffer);

    printf("sws: received packet from IP: %s and Port: %d\n", inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));
    printf("listener: received packet is %d bytes long\n", numbytes);
    buffer[numbytes] = '\0';
    printf("listener: packet contains \"%s\" \n", buffer);


    if(strcmp(buffer,"\n") == 0) return;


    request[0] = strtok(buffer," \t\n");

    if (strncmp(request[0], "GET\0", 4)==0) {
	
	
	request[1] = strtok (NULL, " \t");
	if( request[1] != NULL) count++;
        request[2] = strtok (NULL, " \t\n");
	if( request[2] != NULL) count++;

	if( count < 3 ){
		if ((numbytes = sendto(socketfd, "HTTP/1.0 400 Bad Request\n", 25, 0,
                	(struct sockaddr *) &cli_addr, cli_len)) == -1) {
            		perror("sws: error in sendto()");
            		return;
        	}
	}else if(strncmp( request[2], "HTTP/1.0", 8)!=0 ){
	    	if ((numbytes = sendto(socketfd, "HTTP/1.0 400 Bad Request\n", 25, 0,
                	(struct sockaddr *) &cli_addr, cli_len)) == -1) {
            		perror("sws: error in sendto()");
            		return;
        	}
	}else{

	    if(strncmp(request[1], "/\0", 2)==0 )
                    request[1] = "/index.html";

            printf("request[1] is: %s\n", request[1]);

	    strcpy(path, root_directory);
	    strcpy(&path[strlen(root_directory)], request[1]);

	    printf("file is %s\n", path);

	    /* Check if the file object is found */
	    if ( (fd=open(path, O_RDONLY))!=-1 ){

		if ((numbytes = sendto(socketfd, "HTTP/1.0 200 OK\n", 17, 0,
                	(struct sockaddr *) &cli_addr, cli_len)) == -1) {
            		perror("sws: error in sendto()");
            		return;
        	}

                while ( (bytes_read=read(fd, data_to_send, BYTES))>0 ){
		    if ((numbytes = sendto(socketfd, data_to_send, BYTES, 0,
                	(struct sockaddr *) &cli_addr, cli_len)) == -1) {
            		perror("sws: error in sendto()");
            		return;
        	    }
		}

            }else{
		if ((numbytes = sendto(socketfd, "HTTP/1.0 404 Not Found\n", 23, 0,
                	(struct sockaddr *) &cli_addr, cli_len)) == -1) {
            		perror("sws: error in sendto()");
            		return;
        	}
	    }
	}
    }  

	

}

int startServer(){

    /* Create a socket type UDP */
    if ((socketfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1){
        perror("sws: error on socket()");
        return -1;
    }

    setReusable(socketfd);


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
