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
 */
void readFile(char*filename,int msgNum);

/**Open up the filename for write operation and
 * write it in the specified format
 *
 * @param filename the name of the bulletin board text file 
 * @param msgNum the message number 
 */
void writeFile(char*filename,int  msgNum);


