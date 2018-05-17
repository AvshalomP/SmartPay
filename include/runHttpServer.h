/************************************************************/
/*                                                        	*/
/*  This will be resposible to run the HTTP server as well	*/ 
/*  as supporting and handling various CRUD operations      */
/*                                                        	*/
/************************************************************/

#include <sys/types.h>
#ifndef _WIN32
#include <sys/select.h>
#include <sys/socket.h>
#else
#include <winsock2.h>
#endif
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "CUnit.h"
#include "microhttpd.h"
#include "equipmentMgr.h"
//#include "utils.h"



#define PORT 			8888	//HTTP server listning port
#define MAXNAMESIZE		32 		//max length of resourse size
#define MAXENDPOINTSIZE	10		//limiting the number of terminals to 10
#define MAXRESPONSESIZE 1024	//max json response size
#define ERRMSGSIZE		32		//error message buffer size - related to errors from DB (R/W errors)		
#define ALL 			-1		//when requesting all terminals

#define TERMINALSRESOURCE "/api/terminals/" //terminals resource path

struct postStatus {
    bool status;
    char *buff;
};

bool postWithDataFlag = false; 	//indicating if we got post with data to free allocated post->buff


//temporary jsons
const char *jsonGetById 		= "{ 'response': ['This is a GET BYID request!'] }";    		//json response to be returned
const char *jsonGetAll  		= "{ 'response': ['This is a GET ALL request!'] }";     		//json response to be returned
const char *jsonPostWithData	= "{ 'response': ['This is a POST request with data!'] }";     	//json response to be returned
const char *jsonPostWithNoData	= "{ 'response': ['This is a POST request with NO data!'] }";  	//json response to be returned
const char *jsonError   		= "{ 'error': ['request was neither proper GET nor POST!'] }"; 	//json response to be returned


//extracting terminal id from url
 int getTerminalId(const char* url);


