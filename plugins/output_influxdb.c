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
static const char hostname[] = "hostname";
static const char device[] = "device";
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

void copy_escaped_tag_value(const char* src, char* dest, int dest_len)
{
   int src_index = 0;
   int dest_index = 0;
   int len_src = strlen(src);
   char ch;
   char last_ch;
   memset(dest, 0, dest_len);

   for (; src_index < len_src; src_index++) {
      ch = src[src_index];

      if (ch == ' ' && last_ch == ' ') {
         continue;
      } else if (ch == ',' && last_ch == ',') {
         continue;
      } else if (ch == '=' && last_ch == '=') {
         continue;
      }

      if (ch == '\n') {
         return;
      }

      if (ch == '\\') {
         last_ch = ch;
         continue;
      }

      if ((ch == ' ') || (ch == ',') || (ch == '=')) {
         if (dest_index < (dest_len-2)) {
            dest[dest_index++] = '\\';
         } else {
            return;
         }
      }

      if (dest_index < (dest_len-1)) {
         dest[dest_index++] = ch;
      } else {
         return;
      }
      last_ch = ch;
   }
}

//*****************************************************************************

int is_all_whitespace(const char* s)
{
   int i;
   int s_len = strlen(s);
   char ch;

   for (i = 0; i < s_len; i++) {
      ch = s[i];
      if (ch != ' ' || ch != '\n' || ch != '\r' || ch != '\t') {
         return 0;
      }
   }
   return 1;
}

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
   char s1_data[PATH_MAX]="";
   char s2_data[STR_LEN]="";
   int have_s1;
   int have_s2;
   
   CURL* curl_handle = curl_easy_init();
   if (curl_handle) {
      have_s1 = (data->s1 != NULL) && (strlen(data->s1) > 0) && !is_all_whitespace(data->s1);
      have_s2 = (data->s2 != NULL) && (strlen(data->s2) > 0) && !is_all_whitespace(data->s2);

      if (have_s1) {
         copy_escaped_tag_value(data->s1, s1_data, PATH_MAX);
      }

      if (have_s2) {
         copy_escaped_tag_value(data->s2, s2_data, STR_LEN);
      }

          snprintf(postdata,
                   1024,
                   "%s,%s=%s,%s=%s,%s=%s %s=\"%s\",%s=\"%s\",%s=\"%s\",%s=\"%s\",%s=\"%s\",%s=\"%s\",%s=%d,%s=%f,%s=%d,%s=%d,%s=%d,%s=%zu",
                   measure,
                   facility, data->facility,
                   "dom_tag", domains_names[data->dom_type],
                   "op_tag", ops_names[data->op_type],

		   // tags
                   device, data->device[0]?data->device:"none", 
                   dom_type, domains_names[data->dom_type],
                   hostname, data->hostname,
                   op_type, ops_names[data->op_type],
                   s1,s1_data[0]?s1_data:"NULL",
                   s2, s2_data[0]?s2_data:"NULL",

                   timestamp, data->timestamp,
                   elapsed_time, data->elapsed_time,
                   pid, data->pid,
                   error_code, data->error_code,
                   fd, data->fd,
                   bytes_transferred, data->bytes_transferred);

      puts(postdata);

      // our url is passed in as argument of open_plugin
      curl_easy_setopt(curl_handle, CURLOPT_URL, url);
      curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDS, postdata);
      curl_status = curl_easy_perform(curl_handle);

      if (curl_status == CURLE_OK) {
         rc = PLUGIN_ACCEPT_DATA;
      } else {
         fprintf(stderr, "curl_easy_perform() FAILED: %s\n",
                 curl_easy_strerror(curl_status));
         rc = PLUGIN_REFUSE_DATA;
      }
      curl_easy_cleanup(curl_handle);
   }

   return rc;
}

//*****************************************************************************


