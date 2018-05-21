/********************************************************/
/*														*/
/*	This file contains equipmentMgr.h implementaions 	*/
/*														*/
/********************************************************/

#include "equipmentMgr.h"

static struct terminals * termList;       //head of terminals list
static struct terminals * termListTail;   //tail of terminals list - so spare traversing the list to add new node
static int globalTermCtr = 0;             //global terminals counter 

//corresponding json response strings
const char * basicResponseWraper 	= "{ \"Response\": { %s } }";
const char * postIdTransWraper		= "\"terminalID\": %d, \"transactions\": [ %s ]";
const char * postIdWraper			= "\"terminalID\": %d";
const char * basicErrorWraper		= "{ \"error\": { %s } }";

//shared queue list
struct requestQueueItem * requestQueueList;			//head of shared requests list (queue)
struct requestQueueItem * requestQueueTail = NULL;	//tail of shated requests list (queue)
pthread_mutex_t reqQueueMutex = PTHREAD_MUTEX_INITIALIZER;	//mutex to handle queue

bool sigEqMgrFlag = true;		//signal equipmentMgr to shutdown


//creating node from json  
static struct terminals * createTerminalFromJson(char * json, int * error)
{
	struct terminals * newNode = NULL;
	int newTerminalId = -1;


	//create new terminal (assigning id + transactions data)
	newNode = createNode("");
	sprintf(newNode->transactions, "%s", json);
   
	return newNode;
}

//creating json response from node or list 
static int createJsonFromList(struct terminals* terms, const int termId, char * respJson)
{
  char tempTransHolder[DATASIZE];		//temporary buffer to the ID+transactions portion
  int lastIndex = 0; 					//this is the last index of valid char in json str 
  struct terminals * runner = termList;	//terminals pointer to traverse the list

  /* Creating response json out of list*/
  if(ALL != termId)
  {//creating response json out of specific terminal id
  	sprintf(tempTransHolder, postIdTransWraper, terms->id, terms->transactions);
  	sprintf(respJson, basicResponseWraper, tempTransHolder);
  }
  else
  {//creating response json out of all terminals/nodes in list 
  	lastIndex = sprintf(respJson, basicResponseWraper, " ");	//writing the response frame into our empty json string
  	lastIndex = lastIndex-3; 									//so we can append next transactions before the closing curly bracket

	while( NULL != runner )
	{
		lastIndex += sprintf((respJson+lastIndex), postIdTransWraper, runner->id, runner->transactions);
		lastIndex += sprintf((respJson+lastIndex), "%s", ", ");
  		runner = runner->next;
	}
	sprintf((respJson+lastIndex-2), "%s", " } }"); 
  }

  return SUCCESS;
}


/* Linked List manipulations: add, getById, getAll */
//free all nodes in list
void freeNodeList()
{
	struct terminals* headTerm = termList;			//holds the head of the terminals list to free
	struct requestQueueItem* headReq = requestQueueList;	//holds the head of the requests list to free

	//free terminals list
	while(termList != NULL)
	{
		termList = termList->next;
		free(headTerm);
		headTerm = termList;
	}
	//free request queue
	while(requestQueueList != NULL)
	{
		requestQueueList = requestQueueList->next;
		free(headReq);
		headReq = requestQueueList;
	}
}
//create node
struct terminals * createNode(const char * transData)
{
	struct terminals * newNode;
	
	/* Creating new node/terminal */
	//allocating mempry for new node
	if (!(newNode = (struct terminals *)malloc(sizeof(struct terminals))))
	{
		printf("Memory overflow in insert.\n");
	    exit(1);
	}
	newNode->id = ++globalTermCtr;						//assgining id to the new terminal
	sprintf(newNode->transactions, "%s", transData);	//adding data to the new terminal

	return newNode;
}
//add node to linked list
int addNodeToList(struct terminals * newNode)
{
	//adding node to list
	if(NULL == termListTail)
	{//case list is empty
		newNode->next = NULL;
		termList = termListTail = newNode;
	}
	else
	{
		newNode->next = NULL;
		termListTail->next = newNode;
		termListTail = newNode;
	}

	//return new terminal id created
	return newNode->id;
}
//get node by id from list
struct terminals* getTerminalById(int id)
{
	struct terminals* runner = termList; 	//head of the list
	
	//traversing the list to find the node with "id"
	while(runner != NULL)
	{
		if(runner->id == id)
		{
			return runner;
		}
		runner = runner->next;
	}

	//returns null if node with id "id", not found
	return NULL;
}
//returning the head of the terminals' list
struct terminals* getAllTerminals()
{
	return termList;
}

/* DB manipulations: write, read */
//write to database
char * writeToDb(char * json, char* errMsg)
{
	struct terminals * newNodePtr = NULL;	//new request holder
	char tempDataHolder[DATASIZE];

	//create new terminal node from json
	newNodePtr = createTerminalFromJson(json, NULL);

	//add new terminal to linked list
	addNodeToList(newNodePtr);

	//creating response json
	sprintf(tempDataHolder, postIdWraper, newNodePtr->id);	
	sprintf(json, basicResponseWraper, tempDataHolder);

	return json;
}

//read from database
const int readFromDb(int termId, char * respJson, char* errMsg)
{
	struct terminals * terms = NULL;

	//get terminal/terminal list
	if(ALL == termId)
		terms = getAllTerminals();
	else
		terms = getTerminalById(termId);

	if(NULL == terms)
	{
		sprintf(errMsg, basicErrorWraper, "No such terminal in DB"); 
		return FAIL;
	}

	createJsonFromList(terms, termId, respJson);
	
	return SUCCESS;
}

/* equipmentMger methods*/
//This method will initiatilze and create the Equipment thread
int mgrInit()
{
	/* Terminals' list */
	termList = NULL;			//init linked list - empty list
	termListTail = termList;	//head of linked list = tail
	globalTermCtr = 0; 			//init terminals counter to 0   

	/* Requests queue*/
	requestQueueList = NULL;	//init request queue linked list
	requestQueueTail = NULL;	//head of linked list = tail       


	return SUCCESS;
}

/* request queue methods*/
//adding new request to request queue
void addRequestToQueue(struct requestQueueItem * reqNodeToAdd)
{
  if(NULL == requestQueueTail)
  {//case queue is empty
    reqNodeToAdd->next = NULL;
    requestQueueList = reqNodeToAdd;
    requestQueueTail = requestQueueList;
  }
  else
  {
    reqNodeToAdd->next = NULL;
    requestQueueTail->next = reqNodeToAdd;
    requestQueueTail = requestQueueTail->next;
  }
}

//creaing new request node
struct requestQueueItem * createRequestQueueItem(char * jsonReq, const char * method, int termId, bool isResponseReady, struct MHD_Connection *connection)
{
	struct requestQueueItem * reqNode = (struct requestQueueItem *)malloc(sizeof(struct requestQueueItem));

  	strcpy(reqNode->json, jsonReq);		//request
  	strcpy(reqNode->method, method);	//method
  	reqNode->termId = termId;			//termId
  	reqNode->isResponseReady = isResponseReady; //bool to signal we handled response (sent it over "conncection")

  	return reqNode;
}

//add new request to request queue 
int queueReq(struct requestQueueItem * request)
{
	//lock request queue
	pthread_mutex_lock(&reqQueueMutex);
	//add new request to request queue
	addRequestToQueue(request);
	//unlock request queue
	pthread_mutex_unlock(&reqQueueMutex);

	return SUCCESS;
}


//check and get new request from request queue
struct requestQueueItem * checkQueueAndGetRequest()
{
	struct requestQueueItem * tempReqHolder = requestQueueList;

	//check queue
	if(NULL == requestQueueList)
	{
		return NULL;
	}
	if(requestQueueList == requestQueueTail)
		requestQueueList = requestQueueTail = requestQueueList->next;
	else
		requestQueueList = requestQueueList->next;

	//return first request in queue
	return tempReqHolder;
}

//shutdown
static void shutDown()
{
	//free allocated memory
	freeNodeList();

	printf("\nEquipment thread finished!\n\n");
	//signal to HTTP server that equipmentMgr is done
	sigEqMgrFlag = true;
}

//main routine Equipment manager thread
void * mgrRoutine(void * httpSrvInitMsg)
{
	printf("\n%s\n", (char*)httpSrvInitMsg);

	//init
	mgrInit();

	//polling request list to se if we have new request from HTTP server
	while(sigEqMgrFlag)
	{
		struct requestQueueItem * newRequest = NULL;	//new request holder
		
		/* Check for new request in requestQueue */
		//lock request queue
		pthread_mutex_lock(&reqQueueMutex);
		//check and get new request
		newRequest = checkQueueAndGetRequest();
		//unlock request queue
    	pthread_mutex_unlock(&reqQueueMutex);
		

    	if(NULL != newRequest)	
    	{//we fot new request
			//handling request
			if(0 == strcmp(newRequest->method, "GET"))
			{
				char errMsg[ERRORMSGSIZE];
				if(FAIL == readFromDb(newRequest->termId, newRequest->json, errMsg)) //read terminal/s info from DB
					strcpy(newRequest->json, errMsg);

			}
			else if(0 == strcmp(newRequest->method, "POST"))
			{
				char * response;

				response = writeToDb(newRequest->json, NULL);	//write new terminal to DB
			}

			//
			newRequest->isResponseReady = true;
			free(newRequest);
		}
	}

	//shutdown 
	shutDown();

	return 0;
}

