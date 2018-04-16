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




//The server
int main(int argc, char *argv[]){
	//exe >> port#
	if (argc < 3){
		fprintf(stderr,"Invalid arguments. Run with ./bbserver.c <server port number> <number of initial clients>\n");
		exit(1);
	}

	struct sockaddr_in server_address;
	int socket_hold, newsocket, port, readWriteStatus, numberOfClients;
	numberOfClients = atoi(argv[2]);
	if(numberOfClients<3) { //there must be at least 3 clients to form a the token ring, per project specifications
		fprintf(stderr,"There must be at least 3 clients to start the token ring.\n");
		exit(1);
	}
	//size_t client;


	//Check socket success
	socket_hold = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_hold < 0){
		fprintf(stderr,"Opening socket failed");
		exit(1);
	}

	//Start socketing process
	memset((char *) &server_address,0,sizeof(server_address));
	port = atoi(argv[1]);
	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = INADDR_ANY;
	server_address.sin_port = htons(port);

	//Check binding success
	if (bind(socket_hold, (struct sockaddr *) &server_address,sizeof(server_address)) < 0){//the socket, its cast, the size
		fprintf(stderr,"Binding failed");
		exit(1);
	}





}
