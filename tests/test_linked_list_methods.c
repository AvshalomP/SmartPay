#include "CUnit.h"
#include "Basic.h"

#include <stdio.h>    // for printf
#include <stdlib.h>   //for malloc
#include <stdbool.h>  //for true or false
#include <string.h>   //for strcmp

#include "../include/equipmentMgr.h"

struct terminals * dummy1 = NULL;
struct terminals * dummy2 = NULL;
struct terminals * dummy3 = NULL;

const char transData[] = "{ \"cardType\": \"Visa\", \"TransactionType\": \"Credit\"}, { \"cardType\": \"EFTPOS\", \"TransactionType\": \"Savings\" }";
const char transData2[] = "{ \"cardType\": \"MasterCard\", \"TransactionType\": \"Debit\"}, { \"cardType\": \"EFTPOS\", \"TransactionType\": \"Check\" }";


bool compareNodes(struct terminals * node1, struct terminals * node2)
{
  if(node1->id == node2->id && (strcmp(node1->transactions, node2->transactions) == 0))
    return true;
  else
    return false;
}

/* Test Suite setup and cleanup functions: */
int init_suite(void) { return 0; }
int clean_suite(void) { return 0; }

/* Tests */
//test creation of new node 
void create_node_test(void) 
{ 
  int nodeId = 0;
  bool res = false;
  
  //create node
  dummy1 = createNode(transData);

  //adding node to list
  nodeId = addNodeToList(dummy1);

  //getting node by ID
  dummy1 = getTerminalById(nodeId);

  //create replica
  if (!(dummy2 = (struct terminals *)malloc(sizeof(struct terminals))))
  {
    printf("Memory overflow in insert.\n");
      exit(1);
  }
  dummy2->id = 1;                                 //filling id
  sprintf(dummy2->transactions, "%s", transData); //filling transaction data 


  res = compareNodes(dummy1, dummy2);

  CU_ASSERT(res);
}
//test getting a specific node from list
void get_node_by_id_test(void)
{
  int nodeId = 0;
  
  //create node
  dummy1 = createNode(transData);

  //adding node to list
  nodeId = addNodeToList(dummy1);

  //getting node by ID
  dummy1 = getTerminalById(nodeId);


  CU_ASSERT_STRING_EQUAL(dummy1->transactions, transData);
}
//test adding new node to list
void add_node_to_list_test(void) 
{ 
  int nodeId2 = 0;
  //creating 2 nodes
  dummy1 = createNode(transData);
  dummy2 = createNode(transData2);

  //adding 2 nodes to list
  addNodeToList(dummy1);
  nodeId2 = addNodeToList(dummy2);

  //get node 2
  dummy3 = getTerminalById(nodeId2);
  
  CU_ASSERT_EQUAL(dummy3->id, nodeId2);
  CU_ASSERT_STRING_EQUAL(dummy3->transactions, transData2);
}
//test freeing all allocated memory 
void free_list_test(void)
{
  //creating 2 nodes
  dummy1 = createNode(transData);
  dummy2 = createNode(transData2);

  //adding 2 nodes to list
  addNodeToList(dummy1);
  addNodeToList(dummy2);

  //free list
  freeNodeList();

  //head expected to be null
  CU_ASSERT_PTR_NULL(getTerminalById(1));
}


 int main()
 {
   CU_pSuite pSuite = NULL;

   /* initialize the CUnit test registry */
   if ( CUE_SUCCESS != CU_initialize_registry() )
      return CU_get_error();

   /* add a suite to the registry */
   pSuite = CU_add_suite( "linked_lest_methods_test_suite", init_suite, clean_suite );
   if ( NULL == pSuite ) {
      CU_cleanup_registry();
      return CU_get_error();
   }

   /* add the tests to the suite */
   if ( (NULL == CU_add_test(pSuite, "create_node_test",      create_node_test))        || 
        (NULL == CU_add_test(pSuite, "add_node_to_list_test", add_node_to_list_test))   ||
        (NULL == CU_add_test(pSuite, "get_node_by_id_test",   get_node_by_id_test))     ||
        (NULL == CU_add_test(pSuite, "free_list_test",        free_list_test))
      )
   {
      CU_cleanup_registry();
      return CU_get_error();
   }

   // Run all tests using the basic interface
   CU_basic_set_mode(CU_BRM_VERBOSE);
   CU_basic_run_tests();
   printf("\n");
   CU_basic_show_failures(CU_get_failure_list());
   printf("\n\n");


   /* Clean up registry and return */
   CU_cleanup_registry();
   return CU_get_error();

 }