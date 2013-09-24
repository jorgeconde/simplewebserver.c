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


int main(int argc, char* argv[]){
    
    /* Check command line arguments */
    if (argc != 3) {
        printf("Syntax error: Usage is './sws <port> <directory>'\n");
        return 0;
    }

}