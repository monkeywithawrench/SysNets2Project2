/*************************************************************** 
 * Student Name: Jeff Morton
 * File Name: bbclient.c
 * Assignment Number: 
 * Date Due: Apr 15, 2018
 * 
 *  Created on: Apr 15, 2018
 *      Author: Jeff Morton
 ***************************************************************/
#include <stdio.h>          //Standard IO
#include <stdlib.h>         //Standard library
#include <string.h>        //String Library
//#include <strings.h>         //Strings Library
#include <sys/socket.h>     //API and definitions for the sockets
#include <sys/types.h>      //more definitions
#include <netinet/in.h>     //Structures to store address information
#include <netdb.h>          //Definitions for network's functions
#include <unistd.h>			//Needed for read() and write()
#include <errno.h>			//Used for accessing errno, an int variable set by some system calls and library functions to an error number

#include "bbutils.h"

//The client
int main(int argc, char *argv[]){

	struct sockaddr_in serverAddr, clientAddr;//IPV4
	struct hostent *server;//store info of host
	int sockfd, clientPort, serverPort, n;
	char buffer[BUFFER_SIZE]; //TODO handle this
	char filename[BUFFER_SIZE];
	char serverHostname[BUFFER_SIZE];

	//init our strings
	memset(buffer, 0, BUFFER_SIZE); //TODO this probably needs to be bigger
	memset(filename, 0, BUFFER_SIZE);
	memset(serverHostname, 0, BUFFER_SIZE);

	//exe >> purpose >> port#
	if (argc < 5){
		fprintf(stderr,"please run as follows: bbclient localhost <PortNum> <hostPort> <filenameBulletinBoard>\n");
		exit(0);
	}

	strcpy(serverHostname, argv[1]);
	clientPort = atoi(argv[2]);
	serverPort = atoi(argv[3]);
	strcpy(filename, argv[4]);

	//Check hostname
	server = gethostbyname(argv[1]);//server name, return hostent
	if (server == NULL){
		fprintf(stderr,"Host does not exist\n");
		exit(0);
	}

	//Check for socket
	sockfd = socket(AF_INET, SOCK_STREAM, 0);//return file descriptor, else -1
	if (sockfd < 0 ){
		fprintf(stderr,"Socket Failed with error number: %d\n", errno); //TODO double check that socket() actually sets errno
		exit(errno);
	}

	memset((char*) &serverAddr,0,sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;//For internet, IP address, address family
	memmove((char*) &serverAddr.sin_addr.s_addr,(char*)server->h_addr,server->h_length);
	serverAddr.sin_port = htons(serverPort);

	/*
	//Check the connection
	int checkConnect = connect(sockfd,(struct sockaddr *)&server_address,sizeof(server_address));//(reference to socket by file descriptor,the specified address, address space of socket)
	if (checkConnect < 0) {
		fprintf(stderr,"Failed connection with error number: %d\n", errno);
      		exit(errno);
	}
	*/ //TODO remove this


	char *joinRequest;
	//Set up join request to send to server
	asprintf(&joinRequest, "<join request>\n");
	asprintf(&joinRequest, "%s%s %s\n", joinRequest, serverHostname, clientPort);
	asprintf(&joinRequest, "%s%s\n", joinRequest, filename);
	asprintf(&joinRequest, "%s</join request>\n");

	//n = write(sockfd,joinRequest,strlen(joinRequest));//(reference to socket by file descriptor, the message written, write up to this length
	n = sendto(sockfd,joinRequest,strlen(joinRequest),0,(struct sockaddr *)&serverAddr,sizeof(serverAddr));//for more info, see https://beej.us/guide/bgnet/html/multi/sockaddr_inman.html

	//Check sendto success
	if (n < 0){
		fprintf(stderr,"sendto(server) failed with error number: %d\n", errno);
		exit(errno);
	}


	//TODO wait for token

	//TODO remember to open the bbfile with mode a+ ( fopen("a.txt", "a+") )


	return(0);
}
