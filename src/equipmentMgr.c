/********************************************************/
/*														*/
/*	This file contains equipmentMgr.h implementaions 	*/
/*														*/
/********************************************************/

#include "equipmentMgr.h"

/* Linked List manipulations: add, getById, getAll */
//add node to linked list
int addNodeToList(struct terminals* nodeToAdd)
{
	return SUCCESS;
}
//get node by id from list
struct terminals* getTerminalById(int id)
{

	return NULL;
}
//returning the head of the terminals' list
struct terminals* getAllTerminals()
{
	return termList;
}

/* DB manipulations: write, read */
//write to database
int writeToDb(const void * node, int * error)
{
	return SUCCESS;
}

//read from database
const char * readFromDb(int termId, int * error)
{
	*error = FAIL;
	struct terminals* terms = NULL;

	if(termId == ALL)
		terms = getAllTerminals();
	else
		terms = getTerminalById(termId);

	return createJsonFromLinkedList(terms);
}

//creating json response from node or list 
const char * createJsonFromLinkedList()
{
  char * json = NULL;

  while( NULL != termList )
  {

  }

  return json;
}



void mgrRoutine()
{
	//while HTTP server alive, keep waiting for read/write DB requests
	// {
	//		checking in shared linked list if we have another R/W request
	// }

}

int mgrInit()
{
	//create thread

	//call main loop function 
	mgrRoutine();

	return 1;
}

