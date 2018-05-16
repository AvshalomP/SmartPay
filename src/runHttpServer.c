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
  response =
    MHD_create_response_from_buffer (strlen (json), (void *) json, MHD_RESPMEM_PERSISTENT);
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
  struct postStatus *post = NULL;
  post = (struct postStatus*)*con_cls;

  //check if resource path is valid
  if( strstr(url, TERMINALSRESOURCE) == NULL)
  {
    return sendResponse (connection, jsonError);
  }

  if (0 == strcmp (method, "GET"))
  {
    int termId = ALL;

    //extracting terminal id from url
    termId = getTerminalId(url);    //could be ALL or specific ID

    //get correponding jason from DB
    //jsonResponse = (char*) readFromDb(termId, NULL); //read from db

    if( ALL == termId )
      return sendResponse(connection, jsonGetAll);
    else
      return sendResponse (connection, jsonGetById);
  }

  if (0 == strcmp (method, "POST"))
  {
    char endPoint[MAXENDPOINTSIZE];

    //extracting endpoint from url (we expect NO endpoint)
    sscanf(url, "/api/terminals/%s", endPoint);
    
    //checkig if we got terminal id
    if( endPoint[0] != '\0' )
    {
        return sendResponse (connection, jsonError);
    }
    //first time the callback has been called 
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
    else if(*upload_data_size != 0 && NULL != post->buff) 
    {//check if we have data in post request

        //if we get an empty string for the post data
        if(NULL == post->buff)
        {
          *upload_data_size = 0;
          return MHD_YES;
        }
        post->buff = malloc(*upload_data_size + 1);
        snprintf(post->buff, *upload_data_size+1,"%s", upload_data);
        *upload_data_size = 0;
        jsonResponse = (char*) jsonPostWithData;
        return MHD_YES;
    }
    else 
    {//if we don't have data in post
      if(post->buff == NULL || strcmp(post->buff, "") == 0)
        jsonResponse = (char*) jsonPostWithNoData;
      else
        free(post->buff);
    }

    //freeing allocated memory
    if(post != NULL)
    {
      free(post);
    }
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
