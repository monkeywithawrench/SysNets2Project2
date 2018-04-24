/*************************************************************** 
 * Student Name: Jeff Morton
 * Student Name: Thanh Tran
 * File Name: bbserver.c
 * Assignment Number: 
 * Date Due: Apr 15, 2018
 * 
 *  Created on: Apr 15, 2018
 *      Author: Jeff Morton
 *      Author: Thanh Tran
 ***************************************************************/
#define _GNU_SOURCE			//Need this for asprintf(), otherwise we get implicit declaration
#include <stdio.h>          //Standard IO
#include <stdlib.h>         //Standard library
#include <string.h>			//String Library
//#include <strings.h>         //Strings Library
#include <sys/socket.h>     //API and definitions for the sockets
#include <sys/types.h>      //more definitions
#include <netinet/in.h>     //Structures to store address information
#include <unistd.h>         //Needed for access() to check file information, as well as read() and write()
#include <errno.h>			//Used for accessing errno, an int variable set by some system calls and library functions to an error number

#include "bbutils.h"		//Library for functions and data types needed by both client and server

/** Takes in info from clients, populates clientList
 *
 * @param sockfd the socket file descriptor
 * @param serverAddr the address of the host (ip/hostname and port number) to listen for messages from
 * @param clientList array of client_t structs
 * @param numberOfClients number of client_t structs in the clientList array
 */
void getClients(int sockfd, struct sockaddr_in serverAddr, client_t *clientList, int numberOfClients);

/** Sends neighbor info to the clients in the clientList
 *
 * @param sockfd the socket file descriptor
 * @param clientList array of client_t structs
 * @param numberOfClients number of client_t structs in the clientList array
 */
void sendClients(int sockfd, client_t *clientList, int numberOfClients);


//The server
int main(int argc, char *argv[]) {
	//exe >> port#
	if (argc < 3) {
		fprintf(stderr,"Invalid arguments. Run with ./bbserver.c <server port number> <number of initial clients>\n");
		exit(1);
	}

	struct sockaddr_in serverAddr;
	int sockfd, port, numberOfClients; //sockfd is socket file descriptor returned by socket()
	numberOfClients = atoi(argv[2]);
	if(numberOfClients<3) { //there must be at least 3 clients to form a the token ring, per project specifications
		fprintf(stderr,"There must be at least 3 clients to start the token ring.\n");
		exit(1);
	}
	//size_t client;
	client_t clientList[numberOfClients];

	//Check socket success
	sockfd = socket(AF_INET, SOCK_DGRAM, 0); //return file descriptor, else -1 //was SOCK_STREAM
	if (sockfd < 0) {
		fprintf(stderr,"Getting sockfd from socket() failed, error number %d: ", errno);
		perror("");
		exit(errno);
	}

	//Setting up server sockaddr_in
	memset((char *) &serverAddr,0,sizeof(serverAddr));
	port = atoi(argv[1]);
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = INADDR_ANY;
	serverAddr.sin_port = htons(port);



	//Start socketing process //NOT NEEDED FOR UNCONNECTED UDP
	memset((char *) &serverAddr,0,sizeof(serverAddr));
	port = atoi(argv[1]);
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = INADDR_ANY;
	serverAddr.sin_port = htons(port);



	//Check binding success //NOT NEEDED FOR UNCONNECTED UDP
	if (bind(sockfd, (struct sockaddr *) &serverAddr,sizeof(serverAddr)) != 0) {//the socket, its cast, the size
		fprintf(stderr,"Binding failed with error number %d: ", errno);
		perror("");
		exit(errno);
	}


	fprintf(stdout, "Waiting for client info\n");

	//get all client info
	getClients(sockfd, serverAddr, clientList, numberOfClients);

	//TODO possibly create a new bbfile to read/write to, or simply determine that it exists.

	fprintf(stdout, "Sending client-neighbor info\n");

	//send client-neighbor info
	sendClients(sockfd, clientList, numberOfClients);

	return(0);
}


/** Takes in info from clients, populates clientList
 *
 * @param sockfd the socket file descriptor
 * @param serverAddr the address of the host (ip/hostname and port number) to listen for messages from
 * @param clientList array of client_t structs
 * @param numberOfClients number of client_t structs in the clientList array
 */
void getClients(int sockfd, struct sockaddr_in serverAddr, client_t *clientList, int numberOfClients) {
	int i;

	char buffer[BUFFER_SIZE];
	for(i=0; i<numberOfClients; i++) {
		memset(buffer, 0, BUFFER_SIZE);
		int messagelen = recvfrom(sockfd, buffer, BUFFER_SIZE-1, 0, NULL, NULL); //BUFFER_SIZE-1 so null term doesn't overflow buffer
		if(messagelen <0) {
			fprintf(stderr, "Error receiving message from client at the server, error number %d: ", errno);
			perror("");
			exit(errno);
		}
		fprintf(stdout, "Received client info\n");
		//if we made it this far, no errors yet
		char temp[strlen(buffer)+1]; //+1 for null term
		strcpy(temp, buffer);
		char *token;
		char delim[2] = "\n";
		token = strtok(temp, delim);  // first call returns pointer to first part of user_input separated by delim
		if(token==NULL) { //error checking
			fprintf(stderr, "Message from client empty or incorrect format, not a join request!\n");
			exit(1);
		}
		if(strcmp(token, "<join request>")==0) {//if this is a join request
			strncpy(delim, " ", 2);
			token = strtok(NULL, delim); //ip from 2nd line
			strcpy(clientList[i].hostname, token);
			token = strtok(NULL, delim); //get port# from 2nd line
			clientList[i].port = atoi(token);
		}
	}
}


/** Sends neighbor info to the clients in the clientList
 *
 * @param sockfd the socket file descriptor
 * @param clientList array of client_t structs
 * @param numberOfClients number of client_t structs in the clientList array
 */
void sendClients(int sockfd, client_t *clientList, int numberOfClients) {
	int i;
	char *response;
	//Set up response to send to clients
	asprintf(&response, "<token>\n");
	//asprintf(&response, "%s%s\n", response, filename);
	asprintf(&response, "%s%d\n", response, numberOfClients);
	for(i=0; i<numberOfClients; i++)
		asprintf(&response, "%s%s %d\n", response, clientList[i].hostname, clientList[i].port);
	asprintf(&response, "%sWinner! Sending first token\n", response);
	asprintf(&response, "%s</token>\n", response);

	fprintf(stdout, "Token:\n%s\n", response);
	//send response to client
	int n = sendMessage(sockfd, clientList[0].hostname, clientList[0].port, response);
	//Check for send success
	if (n < 0){
		fprintf(stderr,"sendto(client) failed with error number: %d: ", errno);
		perror("");
		exit(errno);
	}
}
