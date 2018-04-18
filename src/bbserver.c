/*************************************************************** 
 * Student Name: Jeff Morton
 * File Name: bbserver.c
 * Assignment Number: 
 * Date Due: Apr 15, 2018
 * 
 *  Created on: Apr 15, 2018
 *      Author: Jeff Morton
 ***************************************************************/
#include <stdio.h>          //Standard library
#include <stdlib.h>         //Standard library
#include <strings.h>        //Strings Library
#include <string.h>         //String Library
#include <sys/socket.h>     //API and definitions for the sockets
#include <sys/types.h>      //more definitions
#include <netinet/in.h>     //Structures to store address information
#include <unistd.h>         //Needed for access() to check file information, as well as read() and write()
#include <errno.h>			//Used for accessing errno, an int variable set by some system calls and library functions to an error number

#include "bbutils.h"		//Library for functions and data types needed by both client and server


void getClients(int sockfd, client_t *clientList, int numberOfClients);


//The server
int main(int argc, char *argv[]) {
	//exe >> port#
	if (argc < 3) {
		fprintf(stderr,"Invalid arguments. Run with ./bbserver.c <server port number> <number of initial clients>\n");
		exit(1);
	}

	struct sockaddr_in server_address;
	int sockfd, newsocket, port, readWriteStatus, numberOfClients; //sockfd is socket file descriptor returned by socket()
	numberOfClients = atoi(argv[2]);
	if(numberOfClients<3) { //there must be at least 3 clients to form a the token ring, per project specifications
		fprintf(stderr,"There must be at least 3 clients to start the token ring.\n");
		exit(1);
	}
	//size_t client;
	client_t clientList[numberOfClients];

	//Check socket success
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		fprintf(stderr,"Opening socket failed, error number %d\n", errno);
		exit(errno);
	}

	//Start socketing process
	memset((char *) &server_address,0,sizeof(server_address));
	port = atoi(argv[1]);
	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = INADDR_ANY;
	server_address.sin_port = htons(port);

	//Check binding success
	if (bind(sockfd, (struct sockaddr *) &server_address,sizeof(server_address)) != 0) {//the socket, its cast, the size
		fprintf(stderr,"Binding failed with error number %d\n", errno);
		exit(errno);
	}





}


void getClients(int sockfd, client_t *clientList, int numberOfClients) {
	int i;
	struct sockaddr fromaddr;
	char buffer[BUFFER_SIZE];
	for(i=0; i<numberOfClients; i++) {
		memset(&fromaddr, 0, sizeof(fromaddr));
		memset(buffer, 0, BUFFER_SIZE);
		int messagelen = recvfrom(sockfd, buffer, BUFFER_SIZE-1, 0, &fromaddr, sizeof(fromaddr)); //BUFFER_SIZE-1 so null term doesn't overflow buffer
		if(messagelen <0) {
			fprintf(stderr, "Error receiving message from client at the server, error number %d\n", errno);
			exit(errno);
		}
		//if we made it this far, no errors yet
		//TODO make a function in bbutils.c that reads messages in message format, including tags like connect
	}
}
