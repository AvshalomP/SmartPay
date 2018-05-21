/**************************************************************/
/*                                                            */
/*  This file invoke a thread that will manage the terminals  */ 
/*  equipment by performing read/write oprations to the       */
/*  terminals DB.                                             */
/*                                                            */
/**************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <pthread.h>

#include "microhttpd.h"

//outcome message of DB oporation
#define FAIL    0
#define SUCCESS 1
//when requesting all terminals from DB
#define ALL    -1

#define DATASIZE 		1024 
#define SHAREDQUEUESIZE 10 
#define METHODSIZE		10
#define ERRORMSGSIZE	512


//terminal node
struct terminals
{
  int id;
  char transactions[DATASIZE];
  struct terminals * next;
};

//for shared queue
struct requestQueueItem
{
	char method[METHODSIZE]; 			//i.e GET, POST
	char json[DATASIZE];				//request json from HTTP server (for GET, POST)
	int termId;							//terminal ID (if GET) 
	bool isResponseReady;				//flag to see if request was handled
	struct requestQueueItem * next;		//next request
};

//shared queue list
struct requestQueueItem * requestQueueList;	//head of shared requests list (queue)
struct requestQueueItem * requestQueueTail;	//tail of shated requests list (queue)
pthread_mutex_t reqQueueMutex;				//mutex to handle queue

extern bool sigEqMgrFlag;		//signal equipmentMgr to shutdown

//This method will initiatilze after equipmentMgr thread has created
int mgrInit();
//main routine Equipment manager thread
void * mgrRoutine(void * httpSrvInitMsg);

/* DB manipulations: write, read */
//write to database - returns SUCCESS or FAIL
char * writeToDb(char * json, char* errMsg);
//read from database - returns SUCCESS or FAIL
const int readFromDb(int termId, char * respJson, char* errMsg);


/* Linked List manipulations: add, getById, getAll */
//create node - returns pointer to the created node
struct terminals * createNode(const char * transData);
//add node to linked list - returns the assigned node ID
int addNodeToList(struct terminals * newNode);
//get node by id from list - returns pointer to the node with ID "id" 
struct terminals* getTerminalById(int id);
//returning the head of the terminals' list
struct terminals* getAllTerminals();
//free all nodes in list
void freeNodeList();

/* request queue handling */
//check and get new request from request queue
struct requestQueueItem * checkQueueAndGetRequest();
//adding request to request queue
void addRequestToQueue(struct requestQueueItem * reqNodeToAdd);
//creaing new request node
struct requestQueueItem * createRequestQueueItem(char * jsonReq, const char * method, int termId, bool isResponseReady, struct MHD_Connection *connection);
//create & add new request to 
int queueReq(struct requestQueueItem * request);




