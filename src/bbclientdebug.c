/*************************************************************** 
 * Student Name: Jeff Morton
 * Student Name: Thanh Tran
 * File Name: bbclient.c
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
#include <netdb.h>          //Definitions for network's functions
#include <unistd.h>			//Needed for read() and write()
#include <errno.h>			//Used for accessing errno, an int variable set by some system calls and library functions to an error number
#include <pthread.h>		//Needed for mutex and pthread_create

#include "bbutils.h"
//int iterate = 0;//global variable, message number for bulletin board //Doesn't need to be here, only used by IO thread

/** Struct typedef containing all needed parameters for userIO(void *), which takes a single arg of type (void *)
 *
 * @var filename Pointer to filename of the bbfile
 * @var exitRequestMutexPtr Pointer to mutex to lock and unlock isExitRequest in main
 * @var isExitRequestPtr Pointer to isExitRequest in main. isExitRequest is usually 0, set to 1 to exit the token ring
 */
typedef struct {
	char *filename;
	pthread_mutex_t *exitRequestMutexPtr; //Mutex for isExitRequest
	int *isExitRequestPtr; //0 normally, only 1 when client wants to exit ring
}userIO_t;

/** This function will be launched from main via a pthread_create call. As such, it takes (void *ptr) and returns (void *ptr)
 * This function is responsible for interacting with the user at the command line.
 * It will prompt the user for input on what the program should do
 *
 * @param arg the void * pointer to the arguments/parameters for this function.
 */
void * userIO(void *arg);

//The client
int main(int argc, char *argv[]){

	FILE *fp2 = fopen("log.txt", "a"); //TODO remove
	fprintf(fp2, "boop\n"); //TODO remove
	fflush(fp2);
	struct sockaddr_in clientAddr;//IPV4
	struct hostent *host;//store info of host
	int sockfd, clientPort, serverPort, n;
	char filename[BUFFER_SIZE];
	char hostname[BUFFER_SIZE];

	int isJoinRequest = 0; //1 if there is a pending join request, else 0
	int isExitRequest = 0; //0 normally, only 1 when client wants to exit ring
	pthread_mutex_t exitRequestMutex = PTHREAD_MUTEX_INITIALIZER; //Mutex for isExitRequest

	//init our strings
	memset(filename, 0, BUFFER_SIZE);
	memset(hostname, 0, BUFFER_SIZE);

	//exe >> purpose >> port#
	if (argc < 5){
		fprintf(fp2,"please run as follows: bbclient localhost <PortNum> <hostPort> <filenameBulletinBoard>\n");
		exit(0);
	}

	strcpy(hostname, argv[1]);
	clientPort = atoi(argv[2]);
	serverPort = atoi(argv[3]);
	strcpy(filename, argv[4]);

	//Check hostname
	host = gethostbyname(hostname);//server name, return hostent
	if (host == NULL){
		fprintf(fp2,"Hostname passed does not exist, errno: %d: ", errno);
		perror("");
		exit(0);
	}

	//Check for socket
	sockfd = socket(AF_INET, SOCK_DGRAM, 0); //return file descriptor, else -1 //was SOCK_STREAM
	if (sockfd < 0 ){
		fprintf(fp2,"Getting sockfd from socket() failed, error number %d: ", errno);
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
		fprintf(fp2,"Binding failed with error number %d: ", errno);
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
		fprintf(fp2,"sendto(server) failed with error number: %d: ", errno);
		perror("");
		exit(errno);
	}

	fprintf(stdout, "Sent %d bytes, Waiting for server response\n", n);
	fprintf(stdout, "Launching userIO thread.\n");

	fprintf(fp2, "before thread\n"); //TODO remove
	fflush(fp2); //TODO remove
	pthread_t userIOthread;	//New thread
	userIO_t userIOparams;	//Params struct of type userIO_t for thread function
	userIOparams.filename = filename;
	userIOparams.exitRequestMutexPtr = &exitRequestMutex;	//Address of exitRequestMutex
	userIOparams.isExitRequestPtr = &isExitRequest;		//Address of isExitRequest
	errno = pthread_create(&userIOthread, NULL, userIO, &userIOparams);
	if(errno) {
		fprintf(fp2, "Error creating pthread for userIO function! errno: %d: ", errno);
		perror("");
		pthread_mutex_lock(&exitRequestMutex); //Lock mutex around isExitRequest. Avoids race condition
		isExitRequest = 1; //Specifies that this client is exiting. This will perform a graceful exit, leaving the token ring intact!
		pthread_mutex_unlock(&exitRequestMutex); //Unlocks mutex around isExitRequest
		fprintf(fp2,"\nCritical thread failure, requesting to exit the ring...\n");
	}
	fprintf(fp2, "after thread\n"); //TODO remove
	fflush(fp2); //TODO remove

	client_t newClient; //If a new client is joining, store it's info here

	//START PASSING TOKEN!
	while(1 == 1) {
		fprintf(fp2, "inside while loop\n"); //TODO remove
		fflush(fp2); //TODO remove
		//Wait for and receive token
		/* MSG_PEEK stopped working :|
		int bufferlen = recvfrom(sockfd, NULL, 0, MSG_PEEK, NULL, NULL); //Gets length of message in socket buffer. MSG_PEEK specifies check socket buffer but leave it unread
		if(bufferlen <0) {
			fprintf(fp2, "Error receiving(peeking) message from server at the client, error number %d: ", errno);
			perror("");
			exit(errno);
		}
		*/
		int bufferlen = 999; //arbitrary, but large enough
		char buffer[bufferlen+1]; //sets buffer size to exact length of message +1 char for null terminator
		memset(buffer, 0, bufferlen+1); //init's the buffer
		bufferlen = recvfrom(sockfd, buffer, bufferlen, 0, NULL, NULL); //copies message into buffer

		fprintf(fp2, "after receipt\n"); //TODO remove
		fflush(fp2); //TODO remove

		//Save a copy of buffer, we're about to mutilate this string lol
		char temp[strlen(buffer)+1]; //+1 for null term
		strncpy(temp, buffer, strlen(buffer)+1);
		char *token;
		char delim[2] = "\n";
		char *saveptr = temp; //THIS IS NEEDED SO STRTOK DOESN'T GET CONFUSED!
		token = strtok_r(temp, delim, &saveptr);  // first call returns pointer to first part of user_input separated by delim
		if(token==NULL) { //error checking
			fprintf(fp2, "Token empty or incorrect format!\n");
			exit(1);
		}
		if(strcmp(token, "<join request>")==0) {	//if this is a join request
			fprintf(fp2, "join request\n"); //TODO remove
			fflush(fp2); //TODO remove

			token = strtok_r(NULL, delim, &saveptr);
			char temp2[BUFFER_SIZE];
			strncpy(temp2, token, BUFFER_SIZE);
			fprintf(stdout, "Joining client: %s\n", temp2);
			newClient = string2client_t(temp2, BUFFER_SIZE);
			isJoinRequest = 1;
		}
		else if(strcmp(token, "<token>")==0) {	//if this is a token
			fprintf(fp2, "is token\n"); //TODO remove
			fflush(fp2); //TODO remove

			client_t clientNeighbor;
			token = strtok_r(NULL, delim, &saveptr); 	//This line is number of clients!

			//CHECK IF CLIENT WANTS TO EXIT. IF SO, -- THIS NUMBER!!!
			int numberOfClients = atoi(token);
			int futureNumberOfClients = numberOfClients;
			int tempExitStatus; //will hold the current value of isExitRequest until next iteration
			if(isJoinRequest)
				futureNumberOfClients++; //if join request, increase number of clients in the token we're creating


			fprintf(fp2, "before mutex\n"); //TODO remove
			fflush(fp2); //TODO remove

			pthread_mutex_lock(&exitRequestMutex); //Lock mutex around isExitRequest. Avoids race condition
			tempExitStatus = isExitRequest;; //Saves a copy of this var. Safe until next receipt of token.
			pthread_mutex_unlock(&exitRequestMutex); //Unlocks mutex around isExitRequest

			fprintf(fp2, "after mutex\n"); //TODO remove
			fflush(fp2); //TODO remove

			if(tempExitStatus)
				futureNumberOfClients--; //if client is exiting, decrease the number of future clients

			if(futureNumberOfClients <= 1) {
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
			asprintf(&tokenMessage, "%s%d\n", tokenMessage, futureNumberOfClients);
			if(isJoinRequest)
				asprintf(&tokenMessage, "%s%s %d\n", tokenMessage, newClient.hostname, newClient.port); //Joining client added to ring after this client
			asprintf(&tokenMessage, "%s%s %d\n", tokenMessage, clientNeighbor.hostname, clientNeighbor.port);
			int i;
			for (i=0; i<numberOfClients-2; i++) {//minus 2 because next client is already in list, and last client will be added separately
				token = strtok_r(NULL, delim, &saveptr);
				asprintf(&tokenMessage, "%s%s\n", tokenMessage, token);
			}
			if(tempExitStatus == 0) //If client is NOT exiting, append client info to end of token routing table
				asprintf(&tokenMessage, "%s%s %d\n", tokenMessage, hostname, clientPort);
			asprintf(&tokenMessage, "%s</token>\n",tokenMessage);
			//TOKEN MESSAGE COMPLETE!

			fprintf(fp2, "finished creating token\n"); //TODO remove
			fflush(fp2); //TODO remove

			fprintf(stdout, "\n\n%s\n", tokenMessage);
			if(isJoinRequest) {
				strncpy(clientNeighbor.hostname, newClient.hostname, BUFFER_SIZE);
				clientNeighbor.port = newClient.port;
				isJoinRequest = 0; //RESET JOIN REQUEST STATUS
			}
			n = sendMessage(sockfd, clientNeighbor.hostname, clientNeighbor.port, tokenMessage); //Sends message to server
				//Check sendto success
			if (n < 0){
				fprintf(fp2,"sendto(neighbor) failed with error number: %d: ", errno);
				perror("");
				exit(errno);
			}

			fprintf(fp2, "sent message\n"); //TODO remove
			fflush(fp2); //TODO remove


			if(tempExitStatus) {
				//fprintf(stdout, "Sent %d bytes to %s %d. Client requested to exit\n", n, clientNeighbor.hostname, clientNeighbor.port);
				//fprintf(stdout, "Client has been removed from the ring. Exiting.");
				fprintf(fp2, "Sent %d bytes to %s %d. Client requested to exit\n", n, clientNeighbor.hostname, clientNeighbor.port);
				fprintf(fp2, "Client has been removed from the ring. Exiting.");
				errno = pthread_join(userIOthread, NULL);
				if(errno) {
					fprintf(fp2,"Failure joining userIO thread back into main thread. Errno: %d ", errno);
					perror("");
				}
				exit(0);
			}
//			fprintf(stdout, "Sent %d bytes to %s %d, Waiting for next token\n", n, clientNeighbor.hostname, clientNeighbor.port); //if above statement passed, this won't be reached
			fprintf(fp2, "Sent %d bytes to %s %d, Waiting for next token\n", n, clientNeighbor.hostname, clientNeighbor.port); //if above statement passed, this won't be reached
			fflush(fp2);
		}
		else {

		}
		//exit(0);

	}

}


/** This function will be launched from main via a pthread_create call. As such, it takes (void *ptr) and returns (void *ptr)
 * This function is responsible for interacting with the user at the command line.
 * It will prompt the user for input on what the program should do
 *
 * @param arg the void * pointer to the arguments/parameters for this function.
 */
void * userIO(void *arg) {
	//TODO remember to open the bbfile with mode a+ ( fopen("a.txt", "a+") )
	//***new***
	userIO_t *parameters = (userIO_t *)arg; //cast our arg pointer to type userIO_t, a struct containing our params for this function
	char *filename = parameters->filename;
	//pthread_mutex_t *exitRequestMutex = parameters->exitRequestMutexPtr;
	//int *isExitRequest = parameters->isExitRequestPtr; //0 normally, only 1 when client wants to exit ring

	int iterate = 0;	//iterate Message number of the latest message in bulletin board, also the number of messages in bb
	int loop = 0;
	int sequence = 0;	//sequence Variable, this is message number the user would like to read
	char choice;

	while(loop == 0){
		printf( "\n-->Enter w for Write operation!! Appends a new message to the end of the message board\n");
		printf( "-->Enter r for Read operation!! Read a particular message from the message board using a message sequence number. # is the sequence number of the message on the board. \n");
		printf( "-->Enter l for List operation!! Displays the range of valid sequence numbers of messages posted to the board. \n");
		printf( "-->Enter e for Exit operation!! Closes the message board. Exit requires that the user leaves the logical token ring.\n\n");
		printf( "I choose-->");
		scanf(" %c", &choice);//get the user choice
		getchar();//handles the newline that remains in the buffer cause of scanf()
		//printf("|%c|",choice);
		switch(choice) {
		case 'w' :
			writeFile(filename, iterate);
			printf("Okay, writing to the file.\n" );
			iterate++;//increment the message number
			break;
		case 'r' :
			printf("Which message number do you want to read?");
			scanf(" %d", &sequence);
			printf("Users wanted to read message %d\n",sequence);
			getchar();
			readFile(filename,sequence);
			break;
		case 'l' :
			printf("Sequence number ranges from 0 to %d\n" ,iterate);
			break;
		case 'e' :
			printf("Requesting to exit.\n");
			//printf("Exiting now, token is release.\n" );
			pthread_mutex_lock(parameters->exitRequestMutexPtr); //lock mutex for isExitRequest;
			*(parameters->isExitRequestPtr) = 1; //set isExitRequest to 1, signaling our desire to exit
			pthread_mutex_unlock(parameters->exitRequestMutexPtr); //unlock mutex for isExitRequest;
			return(NULL); //exit IO thread
			loop = 1;//escape out of the while loop //this should never run lol

			break;
		default :
			printf("Please choose the correct option!!!\n" );
		}
	}
	//if we got here, something went wrong...
	fprintf(stdout, "Warning: userIO thread might not have exited correctly.\n"); //REALLY should never make it this far. I want to know if it does
	//***new***
	return(NULL);
}
