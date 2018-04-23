/*************************************************************** 
 * Student Name: Jeff Morton
 * File Name: bbclient.c
 * Assignment Number: 
 * Date Due: Apr 15, 2018
 * 
 *  Created on: Apr 15, 2018
 *      Author: Jeff Morton
 ***************************************************************/
#define _GNU_SOURCE			//Need this for asprintf(), otherwise we get implicit declaration
#include <stdio.h>          //Standard IO
#include <stdlib.h>         //Standard library
#include <string.h>			//String Library
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

	struct sockaddr_in clientAddr;//IPV4
	struct hostent *host;//store info of host
	int sockfd, clientPort, serverPort, n;
	char filename[BUFFER_SIZE];
	char hostname[BUFFER_SIZE];

	//init our strings
	memset(filename, 0, BUFFER_SIZE);
	memset(hostname, 0, BUFFER_SIZE);

	//exe >> purpose >> port#
	if (argc < 5){
		fprintf(stderr,"please run as follows: bbclient localhost <PortNum> <hostPort> <filenameBulletinBoard>\n");
		exit(0);
	}

	strcpy(hostname, argv[1]);
	clientPort = atoi(argv[2]);
	serverPort = atoi(argv[3]);
	strcpy(filename, argv[4]);

	//Check hostname
	host = gethostbyname(hostname);//server name, return hostent
	if (host == NULL){
		fprintf(stderr,"Hostname passed does not exist, errno: %d: ", errno);
		perror("");
		exit(0);
	}

	//Check for socket
	sockfd = socket(AF_INET, SOCK_DGRAM, 0); //return file descriptor, else -1 //was SOCK_STREAM
	if (sockfd < 0 ){
		fprintf(stderr,"Getting sockfd from socket() failed, error number %d: ", errno);
		perror("");
		exit(errno);
	}

	//Setting up client sockaddr_in
	memset((char*) &clientAddr,0,sizeof(clientAddr));
	clientAddr.sin_family = AF_INET;//For internet, IP address, address family
	memmove((char*) &clientAddr.sin_addr.s_addr,(char*)host->h_addr,host->h_length);
	clientAddr.sin_port = htons(clientPort);

	//Check binding success //NOT NEEDED FOR UNCONNECTED UDP
	if (bind(sockfd, (struct sockaddr *) &clientAddr,sizeof(clientAddr)) != 0) {//the socket, its cast, the size
		fprintf(stderr,"Binding failed with error number %d: ", errno);
		perror("");
		exit(errno);
	}

	fprintf(stdout, "Sending client info to server\n");

	char *joinRequest;
	//Set up join request to send to server
	asprintf(&joinRequest, "<join request>\n");
	asprintf(&joinRequest, "%s%s %d\n", joinRequest, hostname, clientPort);
	asprintf(&joinRequest, "%s%s\n", joinRequest, filename);
	asprintf(&joinRequest, "%s</join request>\n", joinRequest);

	n = sendMessage(sockfd, hostname, serverPort, joinRequest); //Sends message to server
	//Check sendto success
	if (n < 0){
		fprintf(stderr,"sendto(server) failed with error number: %d: ", errno);
		perror("");
		exit(errno);
	}

	fprintf(stdout, "Sent %d bytes, Waiting for server response\n", n);

	//Wait for and receive token
	int bufferlen = recvfrom(sockfd, NULL, 0, MSG_PEEK, NULL, NULL); //Gets length of message in socket buffer. MSG_PEEK specifies check socket buffer but leave it unread
	if(bufferlen <0) {
		fprintf(stderr, "Error receiving(peeking) message from server at the client, error number %d: ", errno);
		perror("");
		exit(errno);
	}
	if(bufferlen<1) {
		fprintf(stderr, "Client received empty message from server!\n");
		exit(0);
	}
	char buffer[bufferlen+1]; //sets buffer size to exact length of message +1 char for null terminator
	memset(buffer, 0, bufferlen+1); //init's the buffer
	bufferlen = recvfrom(sockfd, buffer, bufferlen, 0, NULL, NULL); //copies message into buffer

	fprintf(stdout, "Server response: \n%s\n", buffer);


	//TODO remember to open the bbfile with mode a+ ( fopen("a.txt", "a+") )


	return(0);
}
