#include "CUnit.h"
#include "Basic.h"

#include <stdio.h>    // for printf
#include <curl/curl.h>  // curl

#define PORT 8888
#define BUFSIZE 2048  //sufficient for 9 terminals

//global vars
static CURL *c;
static CURLcode errornum;
char buf[BUFSIZE];
const char *expceted_postWithData_json    = "{ 'response': ['This is a POST request with data!'] }";      //json response to be returned
const char *expceted_postWithNoData_json  = "{ 'response': ['This is a POST request with NO data!'] }";   //json response to be returned
const char *basic_key_value_json          = "{ \"key1\": \"value1\" }";

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
int init_suite(void) { return 0; }
int clean_suite(void) { return 0; }

//performing curl according to desired url
void execute_curl(const char* url, const char* postData)
{
  struct curl_slist *hs=NULL; //to include content-type in curl

  cbc.buf = buf;
  cbc.size = BUFSIZE;
  cbc.pos = 0;

  c = curl_easy_init ();
  curl_easy_setopt (c, CURLOPT_URL, url);
  curl_easy_setopt (c, CURLOPT_PORT, (long)PORT);
  curl_easy_setopt (c, CURLOPT_POSTFIELDS, postData);
  hs = curl_slist_append(hs, "Content-Type: application/json");
  curl_easy_setopt (c, CURLOPT_HTTPHEADER, hs);
  curl_easy_setopt (c, CURLOPT_WRITEFUNCTION, &copyBuffer);
  curl_easy_setopt (c, CURLOPT_WRITEDATA, &cbc);
  curl_easy_setopt (c, CURLOPT_VERBOSE, 1);
  curl_easy_setopt (c, CURLOPT_TIMEOUT, 150L);

  errornum = curl_easy_perform (c); //performing: curl http://127.0.0.1:8888 GET request
}

//check and see if we got 200 OK response for the POST request
void simple_post_request_test(void) 
{ 
  CU_ASSERT_EQUAL(CURLE_OK, errornum);
  if(CURLE_OK != errornum)
    CU_FAIL(HTTP Server probably down!); //suspecting HTTP server down
}
//check if the json reponse is as expected for POST with no data (i.e no -d in curl)
void post_with_data_request_test(void) 
{ 
  int contentSize;
  const char* url = "http://127.0.0.1/api/terminals/";
  const char* postData = basic_key_value_json;

  execute_curl(url, postData);
  contentSize = strlen(cbc.buf);
  curl_easy_cleanup (c);
  
  CU_ASSERT_STRING_EQUAL( expceted_postWithData_json, cbc.buf );
}
//check if the json reponse is as expected for POST with data (i.e -d in curl)
void post_with_no_data_request_test(void)
{
  int contentSize;
  const char* url = "http://127.0.0.1/api/terminals/";
  const char* postData = "";

  execute_curl(url, postData);
  contentSize = strlen(cbc.buf);
  curl_easy_cleanup (c);

  printf("\n#### expceted_postWithData_json is: %s\n", expceted_postWithNoData_json);
  printf("\n#### cbc.buf is: %s\n", cbc.buf);

  CU_ASSERT_STRING_EQUAL( expceted_postWithNoData_json, cbc.buf );
}


 int main()
 {
   CU_pSuite pSuite = NULL;

   /* initialize the CUnit test registry */
   if ( CUE_SUCCESS != CU_initialize_registry() )
      return CU_get_error();

   /* add a suite to the registry */
   pSuite = CU_add_suite( "post_request_test_suite", init_suite, clean_suite );
   if ( NULL == pSuite ) {
      CU_cleanup_registry();
      return CU_get_error();
   }

   /* add the tests to the suite */
   if ( (NULL == CU_add_test(pSuite, "simple_post_request_test", simple_post_request_test))             || 
        (NULL == CU_add_test(pSuite, "post_with_data_request_test", post_with_data_request_test))       ||
        (NULL == CU_add_test(pSuite, "post_with_no_data_request_test", post_with_no_data_request_test))
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