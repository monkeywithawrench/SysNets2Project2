/*************************************************************** 
 * Student Name: Jeff Morton
 * Student Name: Thanh Tran
 * File Name: bbutils.h
 * Assignment Number: 
 * Date Due: Apr 16, 2018
 * 
 *  Created on: Apr 16, 2018
 *      Author: Jeff Morton
 *      Author: Thanh Tran
 ***************************************************************/
#include <pthread.h>		//Needed for mutex and pthread_create


#define BUFFER_SIZE 256


/* Data struct for list of clients */
typedef struct
{
	char hostname[BUFFER_SIZE];
	int port;
}client_t;


/**Sends a passed message to an address with the hostname and portnumber passed.
 * This function returns the status returned by the sendto() function, which will be -1 if an error occurred.
 *
 * @param sockfd the socket file descriptor
 * @param hostname hostname of destination
 * @param port port number of destination
 * @param message message to send to destination
 * @return status number of sendto() function. This will be -1 if an error occurred.
 */
int sendMessage(int sockfd, char *hostname, int port, char *message);

/** Converts a string of format "hostname port" to a client_t struct
 *
 * @param string the hostname port string to convert
 * @param strlen the length of the string
 * @return client_t struct with hostname and port specified
 */
client_t string2client_t(char *string, int strlen);

/**Open up the filename for read operation and
 * search for the requested message to display to the console
 *
 * @param filename the name of the bulletin board text file 
 * @param msgNum the requested message 
 * @param readWriteMutex pointer to the readWriteMutex, used to halt passing the token until fileIO completes
 * @return returns 0 on success, returns -1 if an error occurs
 */
int readFile(char *filename, int msgNum, pthread_mutex_t *readWriteMutex);

/**Open up the filename for write operation and
 * write it in the specified format
 *
 * @param filename the name of the bulletin board text file 
 * @param readWriteMutex pointer to the readWriteMutex, used to halt passing the token until fileIO completes
 * @return returns 0 on success, returns -1 if an error occurs
 */
int writeFile(char *filename, pthread_mutex_t *readWriteMutex);

/** Returns the current number of messages in the bbfile
 *
 * @param filename filename of the bbfile
 * @param readWriteMutex pointer to the readWriteMutex, used to halt passing the token until fileIO completes
 * @param mutexLocked an signaling whether readWriteMutex has already been locked or not, to avoid deadlock
 * @return the number of messages currently in the bulletinboard file. Returns -1 if an error occurred.
 */
int getMesssageCount(char *filename, pthread_mutex_t *readWriteMutex, int mutexLocked);
