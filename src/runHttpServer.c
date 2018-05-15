#include "runHttpServer.h"

/* callback to handle http request
 *
*/
static int
answer_to_connection (void *cls, struct MHD_Connection *connection,
                      const char *url, const char *method,
                      const char *version, const char *upload_data,
                      size_t *upload_data_size, void **con_cls)
{
  const char *json = "{ 'response': ['Hello-World'] }"; //json response to be returned
  struct MHD_Response *response;  //struct to wrap the json so we can send the response over HTTP
  int ret;
  
  //creating the response
  response =
    MHD_create_response_from_buffer (strlen (json), (void *) json, 
             MHD_RESPMEM_PERSISTENT);
  //queuing the response to be sent
  ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
  MHD_destroy_response (response);

  return ret;
}


int main ()
{
  struct MHD_Daemon *daemon;

  //starting deamon to listen to new inbound connections
  daemon = MHD_start_daemon (MHD_USE_SELECT_INTERNALLY, PORT, NULL, NULL,
                             &answer_to_connection, NULL, MHD_OPTION_END); //for future multi-threaded we will use MHD_USE_THREAD_PER_CONNECTION
  if (NULL == daemon)
    return 1;

  printf("\n\nHTTP server is running...\n");
  printf("(press return key to stop the server)\n\n");

  //
  (void) getchar ();

  MHD_stop_daemon (daemon);

  return 0;
}
