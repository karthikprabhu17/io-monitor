#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <map>
#include <list>

#include "io_monitor.h"

struct http_traffic{
 string firstline;
 map<string,string> header_fields;
}

list<http_traffic> fd_http;

map<int,fd_http> fd_map;

void check_for_http(int dom, int fd, const char* buf, size_t count,
                    struct timeval *s, struct timeval *e)
{
  char buffer1[PATH_MAX];
  char buffer2[STR_LEN];

  int line = 0;
  int i;
  int linelen[2] = {0,0};
  char *tgt;

  for (i = 0; i!= count;  i++) {
    if (buf[i]==0)
      return; // not a HTTP header!

    if (buf[i]=='\r') {
      if (i<count && buf[i+1] == '\n') {
        i++;
        line++;
        if (line > 1)
          break;
      } else {
        return; // not a HTTP!
      }
    }
    if (line) {
      buffer2[linelen[1]]=buf[i];
      linelen[1]++;
      if (linelen[1]>=STR_LEN)
        return;
      buffer2[linelen[1]]=0;
    } else {
      buffer1[linelen[0]]=buf[i];
      linelen[0]++;
      if (linelen[0]>=PATH_MAX)
        return;
      buffer1[linelen[0]]=0;
    }
  }
  if (!strstr(buffer1, "HTTP")) {
    return; // Not a HTTP event!
  }

  if ((!strncmp("GET ", buffer1, 4))
      || (!strncmp("PUT ", buffer1, 4))
      || (!strncmp("HEAD ", buffer1, 5))
      || (!strncmp("POST ", buffer1, 5))
      || (!strncmp("DELETE ", buffer1, 7))) {
    if (dom == FILE_WRITE) {
      record(HTTP, HTTP_REQ_SEND, fd, buffer1, NULL,
             s, e, 0, 0);
    } else {
      record(HTTP, HTTP_REQ_RECV, fd, buffer1, NULL,
             s, e, 0, 0);
    }
  } else if ((!strncmp("HTTP/1", buffer1, 6))) {
    int resp_code;
    char resp_proto[linelen[0]+1];
    char resp_desc[linelen[0]+1];
    sscanf(buffer1,"%s %d %s",resp_proto, &resp_code, resp_desc);
    if (resp_code >=100 && resp_code <1000) {
      if (dom == FILE_WRITE) {
        record(HTTP, HTTP_RESP_SEND, fd, buffer1, NULL,
               s, e, 0, 0);
      } else {
        record(HTTP, HTTP_RESP_RECV, fd, buffer1, NULL,
               s, e, 0, 0);
      }
    }
  }
}
