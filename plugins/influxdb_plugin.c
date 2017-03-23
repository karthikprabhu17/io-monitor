#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <curl/easy.h>

#include "plugin.h"
#include "monitor_record.h"
#include "domains_names.h"
#include "ops_names.h"

static const char facility[] = "facility";//char type
static const char timestamp[] = "timestamp";//int
static const char elapsed_time[] = "elapsed_time";//float
static const char pid[] = "pid";//int
static const char dom_type[] = "dom_type";//int
static const char op_type[] = "op_type";//int
static const char error_code[] = "error_code";//int
static const char fd[] = "fd";//int
static const char bytes_transferred[] = "bytes_transferred";//size_t
static const char s1[] = "s1";//char
static const char s2[] = "s2";//char
static char* url = "http://localhost:8086/write?db=testDB";
static char* measure = "iometrics";

//*****************************************************************************

int open_plugin(const char* plugin_config)
{
   curl_global_init(CURL_GLOBAL_ALL);
   //url = strdup(plugin_config);
   return PLUGIN_OPEN_SUCCESS;
}

//*****************************************************************************

void close_plugin()
{
   curl_global_cleanup();
   if (url != NULL) {
      free(url);
   }
}

//*****************************************************************************

int ok_to_accept_data()
{
   return PLUGIN_ACCEPT_DATA;
}

//*****************************************************************************

int process_data(struct monitor_record_t* data)
{
   int rc = PLUGIN_REFUSE_DATA;
   CURLcode curl_status;
   char postdata[1024];
   CURL* curl_handle = curl_easy_init();
   if (curl_handle) {
      snprintf(postdata,
               1024,
               "%s,%s=%s,%s=%s,%s=%s,%s=%s,%s=%s %s=%d,%s=%f,%s=%d,%s=%d,%s=%d,%s=%zu",
               measure,
               facility,data->facility,
               s1,data->s1,
               s2,data->s2,
               dom_type,domains_names[data->dom_type],
               op_type,ops_names[data->op_type],
               timestamp,data->timestamp,
               elapsed_time,data->elapsed_time,
               pid,data->pid,
               error_code,data->error_code,
               fd,data->fd,
               bytes_transferred,data->bytes_transferred);
      puts(postdata);

      // our url is passed in as argument of open_plugin
      curl_easy_setopt(curl_handle, CURLOPT_URL, url);
      curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDS, postdata);
      curl_status = curl_easy_perform(curl_handle);

      if (curl_status == CURLE_OK) {
         rc = PLUGIN_ACCEPT_DATA;
      }
      else {
       fprintf(stderr, "curl_easy_perform() FAILED: %s\n",
               curl_easy_strerror(curl_status));
      }
      curl_easy_cleanup(curl_handle);
   }

   return rc;
}

//*****************************************************************************


