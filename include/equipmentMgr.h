/**************************************************************/
/*                                                            */
/*  This file invoke a thread that will manage the terminals  */ 
/*  equipment by performing read/write oprations to the       */
/*  terminals DB.                                             */
/*                                                            */
/**************************************************************/

#include <stdio.h>

//outcome message of DB oporation
#define FAIL    0
#define SUCCESS 1
//when requesting all terminals from DB
#define ALL    -1

struct terminals
{
  int id;
  char* data;
  struct terminals * next;
};

struct terminals * termList;

//This method will initiatilze and create the Equipment thread
int mgrInit();

//main Equipment thread routine
void mgrRoutine();

//write to database
int writeToDb(const void * node, int * error);

//add node to linked list
int addNodeToList(struct terminals* nodeToAdd);

//read from database
const char * readFromDb(int termId, int * error);

struct terminals* getTerminalById(int id);
struct terminals* getAllTerminals();

//creating json response from node or list 
const char * createJsonFromLinkedList();


