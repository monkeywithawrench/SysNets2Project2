/*************************************************************** 
 * Student Name: Jeff Morton
 * File Name: bbutils.c
 * Assignment Number: 
 * Date Due: Apr 16, 2018
 * 
 *  Created on: Apr 16, 2018
 *      Author: Jeff Morton
 ***************************************************************/
#include <stdio.h>          //Standard IO
#include <stdlib.h>         //Standard library
#include <string.h>        //String Library
//#include <strings.h>         //Strings Library
#include <sys/socket.h>     //API and definitions for the sockets
#include <sys/types.h>      //more definitions
#include <netinet/in.h>     //Structures to store address information

#include "bbutils.h"


/**Sends a passed message to an address with the hostname and portnumber passed.
 * This function returns the status returned by the sendto() function, which will be -1 if an error occurred.
 *
 * @param sockfd the socket file descriptor
 * @param hostname hostname of destination
 * @param port port number of destination
 * @param message message to send to destination
 * @return status number of sendto() function. This will be -1 if an error occurred.
 */
int sendMessage(int sockfd, char *hostname, int port, char *message) {
	struct sockaddr_in destinationAddr;
	memset((char *) &destinationAddr,0,sizeof(destinationAddr));
	destinationAddr.sin_family = AF_INET;
	destinationAddr.sin_addr.s_addr = INADDR_ANY;
	destinationAddr.sin_port = htons(port);
	size_t addrLen = sizeof(destinationAddr);
	int n = sendto(sockfd,message,strlen(message),0,(struct sockaddr *)&destinationAddr,(socklen_t)addrLen);//for more info, see https://beej.us/guide/bgnet/html/multi/sockaddr_inman.html
	return(n);
}
