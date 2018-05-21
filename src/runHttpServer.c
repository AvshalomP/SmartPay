/********************************************************/
/*                                                      */
/*  This file contains runHttpServer.h implementaions   */
/*                                                      */
/********************************************************/

#include "runHttpServer.h"

/* 
 * Sending json response to the request initiator
*/
int sendResponse (struct MHD_Connection *connection, const char *json)
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
  char errMsg[ERRMSGSIZE];
  struct postStatus *post = NULL;
  post = (struct postStatus*)*con_cls;
  struct requestQueueItem * request;

  //Validation - check if resource path is valid
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

    //create new request
    request = createRequestQueueItem(jResponse, method, termId, false, connection);
    //add new request to request queue
    queueReq(request);
    //holding connection to be completed
    while(!request->isResponseReady);

    //extracting response from handled request
    strcpy(jResponse, request->json);

    //sending appropriate response
    return sendResponse(connection, jResponse);
  }

  /* Received POST request */
  if (0 == strcmp (method, "POST"))
  {
    char endPoint[MAXENDPOINTSIZE];
    char jsonReqResp[MAXRESPONSESIZE];  //using as request and response for post

    //extracting endpoint from url (we expect NO endpoint)
    sscanf(url, "/api/terminals/%s", endPoint);
    
    //validation: checkig if we got wrong resource path
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
      snprintf(post->buff, *upload_data_size+1,"%s", upload_data);  //extracting upload data from request
      *upload_data_size = 0;
      return MHD_YES;
    }

    if(post != NULL)
    {
      if(postWithDataFlag)
      {
        sprintf(jsonReqResp, "%s", post->buff); //preparing request to pass to the DB
        free(post->buff);
        postWithDataFlag = false;

        //create new request
        request = createRequestQueueItem(jsonReqResp, method, -1, false, connection);

        //add new request to request queue
        queueReq(request);

        //holding connection to be completed
        while(!request->isResponseReady);
        strcpy(jsonReqResp, request->json);
      }
      else
        sprintf(jsonReqResp, "%s", jsonPostWithNoData);

      free(post);    //freeing allocated memory
    }

    //sending appropriate response
    return sendResponse(connection, jsonReqResp);;
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
  pthread_t pth_equipmentMgr; //thread ptr equipmentMgr


  //starting deamon to listen to new inbound connections
  daemon = MHD_start_daemon (MHD_USE_SELECT_INTERNALLY, PORT, NULL, NULL,
                             &answerToConnection, NULL, MHD_OPTION_END); //for future multi-threaded we will use 
                                                    //MHD_USE_THREAD_PER_CONNECTION for the first argument
  if (NULL == daemon)
    return 1;

  printf("\n\nHTTP server is running...\n");
  printf("(press return key to stop the server)\n\n");

  //creating the equipmentMgr thread
  pthread_create(&pth_equipmentMgr, NULL, mgrRoutine, "Equipment Manager started...");

  (void) getchar();

  //signal equipmentMgr thread to finish
  sigEqMgrFlag = false;

  while(!sigEqMgrFlag); //waiting for equipmentMgr thread to finish
  pthread_join(pth_equipmentMgr, NULL);

  MHD_stop_daemon (daemon);

  return 0;
}
