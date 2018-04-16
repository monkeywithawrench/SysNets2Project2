/*************************************************************** 
 * Student Name: Jeff Morton
 * File Name: bbutils.h
 * Assignment Number: 
 * Date Due: Apr 16, 2018
 * 
 *  Created on: Apr 16, 2018
 *      Author: Jeff Morton
 ***************************************************************/


#define BUFFER_SIZE 256


/* Data struct for list of clients */
typedef struct
{
    char hostname[BUFFER_SIZE+1]; //+1 to include '\0'
    int port;
}client_t;
