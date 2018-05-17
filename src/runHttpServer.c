/********************************************************/
/*                                                      */
/*  This file contains runHttpServer.h implementaions   */
/*                                                      */
/********************************************************/

#include "runHttpServer.h"

/* 
 * Sending json response to the requeest initiator
*/
static int
sendResponse (struct MHD_Connection *connection, const char *json)
{
  struct MHD_Response *response;
  int ret;


  //creating the response
  response = MHD_create_response_from_buffer (strlen(json), 
    (void *) json, MHD_RESPMEM_MUST_COPY); //using MHD_RESPMEM_MUST_COPY because our json buffer is the stack (not allocated)
  if (!response)
    return MHD_NO;
  MHD_add_response_header(response, "Content-Type", "application/json");

  //queuing the response to be sent
  ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
  MHD_destroy_response (response);

  return ret;
}

/* 
 * A callback to handle HTTP request
*/
static int
answerToConnection (void *cls, struct MHD_Connection *connection,
                      const char *url, const char *method,
                      const char *version, const char *upload_data,
                      size_t *upload_data_size, void **con_cls)
{
  char * jsonResponse;
  char errMsg[ERRMSGSIZE];
  struct postStatus *post = NULL;
  post = (struct postStatus*)*con_cls;

  //check if resource path is valid
  if( strstr(url, TERMINALSRESOURCE) == NULL)
  {
    return sendResponse (connection, jsonError);
  }

  /* Received GET request */
  if (0 == strcmp (method, "GET"))
  {
    int termId = ALL;
    char jResponse[MAXRESPONSESIZE];

    //extracting terminal id from url
    termId = getTerminalId(url);    //could be ALL or specific ID

    //get correponding json from DB
    readFromDb(termId, jResponse, errMsg);

    //sending appropriate response
    return sendResponse(connection, jResponse);
  }

  /* Received POST request */
  if (0 == strcmp (method, "POST"))
  {
    char endPoint[MAXENDPOINTSIZE];

    //extracting endpoint from url (we expect NO endpoint)
    sscanf(url, "/api/terminals/%s", endPoint);
    
    //checkig if we got wrong resource path
    if( endPoint[0] != '\0' )
    {
        return sendResponse (connection, jsonError);
    }

    //first time the callback has been called (upload_data still not available)
    if(post == NULL) {
      post = malloc(sizeof(struct postStatus));
      post->status = false;
      *con_cls = post;
    }

    if(!post->status) 
    {
      post->status = true;
      return MHD_YES;
    } 
    //the second time invoking this callback makes the upload_data available
    else if(*upload_data_size != 0)
    {//check if we have data in post request

      postWithDataFlag = true; 
      post->buff = malloc(*upload_data_size + 1);
      snprintf(post->buff, *upload_data_size+1,"%s", upload_data);
      *upload_data_size = 0;
      jsonResponse = (char*) jsonPostWithData;
      return MHD_YES;
    }

    //freeing allocated memory
    if(post != NULL)
    {
      if(postWithDataFlag)
      {
        free(post->buff);
        postWithDataFlag = false;
      }
      else
        jsonResponse = (char*) jsonPostWithNoData;
      free(post);
    }

    //NOTE: for testing only - remove when done!
    //writeToDb(jsonPostWithData, NULL);

    //sending appropriate response
    return sendResponse (connection, jsonResponse);
  }

  //sending error message as we didn't get proper GET or POST request
  return sendResponse (connection, jsonError);
}

//extracting terminal id from url
int getTerminalId(const char* url)
{
    int termId = ALL;
    char endPoint[MAXENDPOINTSIZE];

    //extracting terminal id as string from url
    sscanf(url, "/api/terminals/%s", endPoint);
    
    //checkig if we got terminal id
    if( endPoint[0] != '\0' )
    {
      termId = atoi(endPoint); 
    }

    //returning terminal id or ALL
    return termId;
}

int main()
{
  struct MHD_Daemon *daemon;

  //starting deamon to listen to new inbound connections
  daemon = MHD_start_daemon (MHD_USE_SELECT_INTERNALLY, PORT, NULL, NULL,
                             &answerToConnection, NULL, MHD_OPTION_END); //for future multi-threaded we will use 
                                                    //MHD_USE_THREAD_PER_CONNECTION for the first argument

  if (NULL == daemon)
    return 1;

  printf("\n\nHTTP server is running...\n");
  printf("(press return key to stop the server)\n\n");

  //
  (void) getchar ();

  MHD_stop_daemon (daemon);

  return 0;
}
