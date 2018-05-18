/**************************************************************/
/*                                                            */
/*  This file invoke a thread that will manage the terminals  */ 
/*  equipment by performing read/write oprations to the       */
/*  terminals DB.                                             */
/*                                                            */
/**************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//outcome message of DB oporation
#define FAIL    0
#define SUCCESS 1
//when requesting all terminals from DB
#define ALL    -1

#define DATASIZE 1024   

//terminal node
struct terminals
{
  int id;
  char transactions[DATASIZE];
  struct terminals * next;
};

//This method will initiatilze and create the Equipment thread
int mgrInit();

/* DB manipulations: write, read */
//write to database - returns SUCCESS or FAIL
int writeToDb(char * json, char* errMsg);
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

//main routine Equipment manager thread
void mgrRoutine(const char* json, const char* method);



