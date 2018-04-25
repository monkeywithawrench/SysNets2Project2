/*************************************************************** 
 * Student Name: Jeff Morton
 * Student Name: Thanh Tran
 * File Name: bbutils.c
 * Assignment Number: 
 * Date Due: Apr 16, 2018
 * 
 *  Created on: Apr 16, 2018
 *      Author: Jeff Morton
 *      Author: Thanh Tran
 ***************************************************************/
#include <stdio.h>          //Standard IO
#include <stdlib.h>         //Standard library
#include <string.h>        	//String Library
//#include <strings.h>         //Strings Library
#include <sys/socket.h>     //API and definitions for the sockets
#include <sys/types.h>      //more definitions
#include <netinet/in.h>     //Structures to store address information
#include <errno.h>			//Used for accessing errno, an int variable set by some system calls and library functions to an error number

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


/** Converts a string of format "hostname port" to a client_t struct
 *
 * @param string the hostname port string to convert
 * @param strlen the length of the string
 * @return client_t struct with hostname and port specified
 */
client_t string2client_t(char *string, int strlen) {
	char *token; //Token pointer from strtok_r
	char temp[strlen+1];
	char *saveptr = temp; //THIS IS NEEDED SO STRTOK DOESN'T GET CONFUSED!
	strncpy(temp, string, strlen+1);
	char delim[2] = " ";
	client_t client;
	token = strtok_r(temp, delim, &saveptr); //gets the hostname
	strcpy(client.hostname, token); //Sets the client IP/hostname
	token = strtok_r(NULL, delim, &saveptr); //gets the port number
	client.port = atoi(token);
	return(client);
}


/**Open up the filename for read operation and
 * search for the requested message to display to the console
 *
 * @param filename the name of the bulletin board text file 
 * @param msgNum the requested message 
 * @param readWriteMutex pointer to the readWriteMutex, used to halt passing the token until fileIO completes
 * @return returns 0 on success, returns -1 if an error occurs
 */
int readFile(char *filename, int msgNum, pthread_mutex_t *readWriteMutex) {
	pthread_mutex_lock(readWriteMutex); //this will lock the main thread once it acquires a token
	int messageCount = getMesssageCount(filename, readWriteMutex, 1); //get the current number of messages in the file
	if(messageCount == -1)
		return(-1); //Pass error code from getMessageCount up the function stack
	else if(messageCount < msgNum) {
		printf("Message number %d does not exist yet, current message count is %d\n", msgNum, messageCount);
		pthread_mutex_unlock(readWriteMutex); //unlock the mutex now that we're done
		return(0);
	}
	FILE *fp = fopen(filename, "r");
	char match[BUFFER_SIZE];//the message number that'll be search
	char buff[BUFFER_SIZE];//file read goes into this buffer
	if (fp == NULL) {
		fprintf(stderr, "\nOops, '%s' could not open for read.\n",filename);
		return(-1); //return error code
	}
	sprintf(match,"<message n=%d>\n",msgNum);//set text w/ iterated number like in a printf format to this string
	while(fgets (buff, BUFFER_SIZE, fp)!=NULL){//look through the whole file
		if(strcmp(buff, match) == 0){
			printf("%s", buff );
			while(strcmp(fgets (buff, BUFFER_SIZE, fp),"</message>\n") != 0){//print out the matching message
				printf("%s", buff );
			}
		}
	}
	fclose(fp);
	pthread_mutex_unlock(readWriteMutex); //unlock the mutex now that we're done
	return(0);
}


/**Open up the filename for write operation and
 * write it in the specified format
 *
 * @param filename the name of the bulletin board text file 
 * @param readWriteMutex pointer to the readWriteMutex, used to halt passing the token until fileIO completes
 * @return returns 0 on success, returns -1 if an error occurs
 */
int writeFile(char *filename, pthread_mutex_t *readWriteMutex) {
	char str[BUFFER_SIZE];
	printf( "Enter a string-->");
	fgets(str,sizeof(str),stdin); //get user input string. Locking before here would cause deadlock until user entered a string...
	pthread_mutex_lock(readWriteMutex); //this will lock the main thread once it acquires a token
	int messageCount = getMesssageCount(filename, readWriteMutex, 1); //get the current number of messages in the file
	if(messageCount == -1)
		return(-1); //Pass error code from getMessageCount up the function stack
	FILE *fp = fopen(filename, "a+"); //a+ to read and append. Change to a if we're only appending
	char header[BUFFER_SIZE];
	char footer[BUFFER_SIZE];

	sprintf(header,"<message n=%d>\n",messageCount+1);//set text w/ iterated number like in a printf format to this string
	sprintf(footer,"</message>\n");

	if(fp == NULL) {//checks file can open
		fprintf(stderr, "\nOops, '%s' could not open for write.\n",filename);
		pthread_mutex_unlock(readWriteMutex); //unlock the mutex now that we're done
		return(-1); //return error code
	}
	fputs(header,fp);
	fputs(str,fp);//write/append to file
	fputs(footer,fp);
	fclose(fp);
	pthread_mutex_unlock(readWriteMutex); //unlock the mutex now that we're done
	return(0);
}


/** Returns the current number of messages in the bbfile
 *
 * @param filename filename of the bbfile
 * @param readWriteMutex pointer to the readWriteMutex, used to halt passing the token until fileIO completes
 * @param mutexLocked an signaling whether readWriteMutex has already been locked or not, to avoid deadlock
 * @return the number of messages currently in the bulletinboard file. Returns -1 if an error occurred.
 */
int getMesssageCount(char *filename, pthread_mutex_t *readWriteMutex, int mutexLocked) {
	if(!mutexLocked) //if the mutex is not already locked by another function, lock mutex
		pthread_mutex_lock(readWriteMutex); //this will lock the main thread once it acquires a token
	FILE *fp = fopen(filename, "r");
	char buff[BUFFER_SIZE];//file read goes into this buffer
	int messageCount = 0;
	if (fp == NULL) {
		fprintf(stderr, "\nOops, '%s' could not open for read.\n",filename);
		if(!mutexLocked)
			pthread_mutex_unlock(readWriteMutex); //unlock the mutex now that we're done
		return(-1); //return error code
	}
	while(fgets (buff, BUFFER_SIZE, fp)!=NULL){//look through the whole file
		if(strcmp(buff, "<message n=") == 0)
			messageCount++;
	}
	fclose(fp);
	if(!mutexLocked) //if the mutex is not already locked by another function, lock mutex
		pthread_mutex_unlock(readWriteMutex); //unlock the mutex now that we're done
	return(messageCount);
}

