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

//creating node from json  
static struct terminals * createTerminalFromJson(const char * json, int * error)
{
	struct terminals * newNode = NULL;
	char * data;
	int newTerminalId = -1;

	//create new terminal (assigning id + transactions data)
	newNode = createNode(data);

	//extracting relevant data from json
   	//memset(newNode->data, '\0', sizeof(newNode->data));	//clear the memory location
   	//strncpy(newNode->data, json+14, (strlen(json)-16));	//+14 means getting what's after the response key
	//snprintf(newNode->data, strlen(json), "%[^:]: %s", json);
	//->> sscanf(json, "%99[^{}]{ %99[^}]}", data, newNode->transactions);

   
	return newNode;
}

//creating json response from node or list 
static int createJsonFromList(struct terminals* terms, const int termId, char * respJson)
{
  char tempTransHolder[DATASIZE];		//temporary buffer to the transactions portion
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
	struct terminals* head = termList;	//holds the head of the node list to free

	//traversing the list and freeing all nodes
	while(termList != NULL)
	{
		termList = termList->next;
		free(head);
		head = termList;
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
int writeToDb(const char * json, char* errMsg)
{
	struct terminals * newNodePtr = NULL;

	mgrInit(); //NOTE: remove when DB thread is implemented


	//create new terminal node from json
	newNodePtr = createTerminalFromJson(json, NULL);

	//add new terminal to linked list
	addNodeToList(newNodePtr);

	return SUCCESS;
}


//NOTE: create dummy list - need to remove!
static void createDummyList()
{
	//NOTE: for test - remove when done!
	struct terminals * dummy1 = NULL;
	struct terminals * dummy2 = NULL;
	const char transData[] = "{ \"cardType\": \"Visa\", \"TransactionType\": \"Credit\"}, { \"cardType\": \"EFTPOS\", \"TransactionType\": \"Savings\" }";
	const char transData2[] = "{ \"cardType\": \"MasterCard\", \"TransactionType\": \"Debit\"}, { \"cardType\": \"EFTPOS\", \"TransactionType\": \"Check\" }";


	mgrInit(); //NOTE: remove when DB thread is implemented

	//NOTE: adding dummy nodes for test - remove when done
	dummy1 = createNode(transData);
	dummy2 = createNode(transData2);
	addNodeToList(dummy1); 
	addNodeToList(dummy2);
}
//read from database
const int readFromDb(int termId, char * respJson, char* errMsg)
{
	struct terminals * terms = NULL;

	//NOTE: create dummy list - need to remove!
	createDummyList();

	//get terminal/terminal list
	if(ALL == termId)
		terms = getAllTerminals();
	else
		terms = getTerminalById(termId);

	if(NULL == terms)
	{
		sprintf(errMsg, "%s", "\nNo terminals in DB!\n"); 
		return FAIL;
	}

	createJsonFromList(terms, termId, respJson);
	
	return SUCCESS;
}

static void shutDown()
{
	//free allocated memory
	freeNodeList();

	//kill thread

}
//main routine Equipment manager thread
void mgrRoutine(const char* json, const char* method)
{

	//while HTTP server alive, keep waiting for read/write DB requests
	// {
	//		checking in shared memory if we have another R/W request
	// }


	//main logic
	if(0 == strcmp(method, "GET"))
	{

	}
	else if(0 == strcmp(method, "POST"))
	{

	}

	//shutdown - traversing through the list and freeing allocated memory
	//shutDown();
}

//This method will initiatilze and create the Equipment thread
int mgrInit()
{
	//init all globals
	termList = NULL;			//init linked list - empty list
	termListTail = termList;	//head of linked list = tail
	globalTermCtr = 0; 			//init terminals counter to 0            

	//create thread

	//call routine function 
	//mgrRoutine();

	return SUCCESS;
}

