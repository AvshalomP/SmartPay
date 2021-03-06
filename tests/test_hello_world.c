#include "CUnit.h"
#include "Basic.h"

#include <stdio.h>  	// for printf
#include <curl/curl.h> 	// curl

#define PORT 8888
#define BUFSIZE 2048 	//sufficient for 9 terminals

//global vars
static CURL *c;
static CURLcode errornum;
char buf[BUFSIZE];
const char * expceted_json = "{ 'error': [Request was neither proper GET nor POST!] }"; //json response to be returned

//struct that will contain the response from the HTTP request
struct CBC
{
  char *buf;
  size_t pos;
  size_t size;
};

static struct CBC cbc;

//copy HTTP response to buufer  
static size_t
copyBuffer (void *ptr, size_t size, size_t nmemb, void *ctx)
{
  struct CBC *cbc = ctx;

  if (cbc->pos + size * nmemb > cbc->size)
    return 0;                   /* overflow */
  memcpy (&cbc->buf[cbc->pos], ptr, size * nmemb);
  cbc->pos += size * nmemb;
  return size * nmemb;
}

/* Test Suite setup and cleanup functions: */
int init_suite(void) 
{ 
  struct curl_slist *hs=NULL; //to include content-type in curl

	cbc.buf = buf;
	cbc.size = BUFSIZE;
	cbc.pos = 0;

	c = curl_easy_init ();
	curl_easy_setopt (c, CURLOPT_URL, "http://127.0.0.1/");
	curl_easy_setopt (c, CURLOPT_PORT, (long)PORT);
  hs = curl_slist_append(hs, "Content-Type: application/json");
  curl_easy_setopt (c, CURLOPT_HTTPHEADER, hs);
	curl_easy_setopt (c, CURLOPT_WRITEFUNCTION, &copyBuffer);
	curl_easy_setopt (c, CURLOPT_WRITEDATA, &cbc);
	curl_easy_setopt (c, CURLOPT_VERBOSE, 1);
	curl_easy_setopt (c, CURLOPT_TIMEOUT, 150L);

	errornum = curl_easy_perform (c); //performing: curl http://127.0.0.1:8888 GET request

	return 0; 
}

int clean_suite(void) 
{ 
	curl_easy_cleanup (c);

	return 0; 
}

//check and see if we got 200 OK response for the GET request
void simple_get_request_test(void) 
{	
	CU_ASSERT_EQUAL(CURLE_OK, errornum);
	if(CURLE_OK != errornum)
		CU_FAIL(HTTP Server probably down!); //suspecting HTTP server down
}
//check if the json reponse is as expected
void simple_get_response_test(void)
{
	int contentSize;

	contentSize = strlen(cbc.buf);

	CU_ASSERT_STRING_EQUAL( expceted_json, cbc.buf );
}


 int main()
 {
   CU_pSuite pSuite = NULL;

   /* initialize the CUnit test registry */
   if ( CUE_SUCCESS != CU_initialize_registry() )
      return CU_get_error();

   /* add a suite to the registry */
   pSuite = CU_add_suite( "simple_request_test_suite", init_suite, clean_suite );
   if ( NULL == pSuite ) {
      CU_cleanup_registry();
      return CU_get_error();
   }

   /* add the tests to the suite */
   if ( (NULL == CU_add_test(pSuite, "simple_get_request_test", simple_get_request_test)) 	||
   		(NULL == CU_add_test(pSuite, "simple_get_response_test", simple_get_response_test))
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