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

	client_t newClient; //If a new client is joining, store it's info here
	int isJoinRequest = 0; //1 if there is a pending join request, else false

	//START PASSING TOKEN!
	while(1 == 1) {

		//Wait for and receive token
		/* MSG_PEEK stopped working :|
		int bufferlen = recvfrom(sockfd, NULL, 0, MSG_PEEK, NULL, NULL); //Gets length of message in socket buffer. MSG_PEEK specifies check socket buffer but leave it unread
		if(bufferlen <0) {
			fprintf(stderr, "Error receiving(peeking) message from server at the client, error number %d: ", errno);
			perror("");
			exit(errno);
		}
		*/
		int bufferlen = 999; //arbitrary, but large enough
		char buffer[bufferlen+1]; //sets buffer size to exact length of message +1 char for null terminator
		memset(buffer, 0, bufferlen+1); //init's the buffer
		bufferlen = recvfrom(sockfd, buffer, bufferlen, 0, NULL, NULL); //copies message into buffer

		//Save a copy of buffer, we're about to mutilate this string lol
		char temp[strlen(buffer)+1]; //+1 for null term
		strncpy(temp, buffer, strlen(buffer)+1);
		char *token;
		char delim[2] = "\n";
		char *saveptr = temp; //THIS IS NEEDED SO STRTOK DOESN'T GET CONFUSED!
		token = strtok_r(temp, delim, &saveptr);  // first call returns pointer to first part of user_input separated by delim
		if(token==NULL) { //error checking
			fprintf(stderr, "Token empty or incorrect format!\n");
			exit(1);
		}
		if(strcmp(token, "<join request>")==0) {	//if this is a join request
			token = strtok_r(NULL, delim, &saveptr);
			char temp2[BUFFER_SIZE];
			strncpy(temp2, token, BUFFER_SIZE);
			fprintf(stdout, "Joining client: %s\n", temp2);
			newClient = string2client_t(temp2, BUFFER_SIZE);
			isJoinRequest = 1;
		}
		else if(strcmp(token, "<token>")==0) {	//if this is a token
			client_t clientNeighbor;
			token = strtok_r(NULL, delim, &saveptr); 	//This line is number of clients!
			//TODO CHECK IF CLIENT WANTS TO EXIT. IF SO, -- THIS NUMBER!!!
			int numberOfClients = atoi(token);
			if(numberOfClients <= 1) {
				fprintf(stdout, "Client is only client left in ring, exiting!");
				exit(0);
			}
			token = strtok_r(NULL, delim, &saveptr); 	//Skip this line, it's this client's IP and port
			token = strtok_r(NULL, delim, &saveptr); //IP and port of neighbor client
			char temp2[BUFFER_SIZE];
			strncpy(temp2, token, BUFFER_SIZE);
			clientNeighbor = string2client_t(temp2, BUFFER_SIZE);

			//Set up token to send to neighbor client
			char *tokenMessage;
			asprintf(&tokenMessage, "<token>\n");
			if(isJoinRequest) {
				asprintf(&tokenMessage, "%s%d\n", tokenMessage, numberOfClients+1); //+1 for new client!
				asprintf(&tokenMessage, "%s%s %d\n", tokenMessage, newClient.hostname, newClient.port); //Joining client added to ring after this client
				//isJoinRequest = 0; //RESET JOIN REQUEST STATUS
			} else
				asprintf(&tokenMessage, "%s%d\n", tokenMessage, numberOfClients);
			asprintf(&tokenMessage, "%s%s %d\n", tokenMessage, clientNeighbor.hostname, clientNeighbor.port);
			int i;
			for (i=0; i<numberOfClients-2; i++) {//minus 2 because next client is already in list, and last client will be added separately
				token = strtok_r(NULL, delim, &saveptr);
				asprintf(&tokenMessage, "%s%s\n", tokenMessage, token);
			}
			asprintf(&tokenMessage, "%s%s %d\n", tokenMessage, hostname, clientPort);
			asprintf(&tokenMessage, "%s</token>\n",tokenMessage);
			//TOKEN MESSAGE COMPLETE!

			fprintf(stdout, "\n\n%s\n", tokenMessage);
			n = sendMessage(sockfd, clientNeighbor.hostname, clientNeighbor.port, tokenMessage); //Sends message to server
				//Check sendto success
			if (n < 0){
				fprintf(stderr,"sendto(neighbor) failed with error number: %d: ", errno);
				perror("");
				exit(errno);
			}

			fprintf(stdout, "Sent %d bytes to %s %d, Waiting for next token\n", n, clientNeighbor.hostname, clientNeighbor.port);
			if(isJoinRequest)
				exit(0);

		}
		else {

		}
		//exit(0);
		//TODO CHECK IF JOIN REQUEST!!!!!

	}

	//TODO remember to open the bbfile with mode a+ ( fopen("a.txt", "a+") )





	return(0);
}
