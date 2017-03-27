//
// Copyright (c) 2017 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
// implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <string>
#include <map>

#include "resolver.h"
#include "monitor_record.h"


#define LEN_DEVICE_NAME 40
#define LEN_MAJ_MIN 10
#define LEN_DEVICE_SIZE 10
#define LEN_DEVICE_TYPE 40
#define LEN_MOUNT_POINT 256

typedef struct _device_config {
   char device_name[LEN_DEVICE_NAME];
   char maj_min[LEN_MAJ_MIN];
   char rm;
   char device_size[LEN_DEVICE_SIZE];
   char ro;
   char device_type[LEN_DEVICE_TYPE];
   char mount_point[LEN_MOUNT_POINT];
} device_config;


using namespace std;


// mapping of file system mount points to devices
static map<string,string> fs_mount_path_to_dev;
static string root_fs_dev;  // device for root file system

// mapping of file descriptors to devices
static map<int,string> fd_to_dev;


//*****************************************************************************

int parse_device_fields(const char* device_fields, device_config* dc)
{
   char* pos_equal;
   char* token;
   char* buf_copy;
   char* rest;
   char* pos_open_quote;
   char* pos_close_quote;
   char* pos_field_value;
   int j = 0;
   int num_field_chars;

   buf_copy = strdup(device_fields);
   rest = buf_copy;
   memset(dc, 0, sizeof(struct _device_config));

   //NAME="vda" MAJ:MIN="253:0" RM="0" SIZE="40G" RO="0" TYPE="disk" MOUNTPOINT="" 
   while (token = strtok_r(rest , " ", &rest)) {
      pos_equal = strchr(token, '=');
      if (NULL == pos_equal) {
         continue;
      }
      pos_open_quote = strchr(pos_equal+1, '"');
      if (NULL == pos_open_quote) {
         continue;
      }
      pos_field_value = pos_open_quote + 1;

      pos_close_quote = strchr(pos_field_value, '"');
      if (NULL == pos_close_quote) {
         continue;
      }

      num_field_chars = pos_close_quote - pos_open_quote - 1;

      switch (j) {
         case 0:
            memcpy(dc->device_name,
                   pos_field_value,
                   MIN(LEN_DEVICE_NAME, num_field_chars));
            break;
         case 1:
            memcpy(dc->maj_min,
                   pos_field_value,
                   MIN(LEN_MAJ_MIN, num_field_chars));
            break;
         case 2:
            dc->rm = pos_field_value[0];
            break;
         case 3:
            memcpy(dc->device_size,
                   pos_field_value,
                   MIN(LEN_DEVICE_SIZE, num_field_chars));
            break;
         case 4:
            dc->ro = pos_field_value[0];
            break;
         case 5:
            memcpy(dc->device_type,
                   pos_field_value,
                   MIN(LEN_DEVICE_TYPE, num_field_chars));
            break;
         case 6:
            memcpy(dc->mount_point,
                   pos_field_value,
                   MIN(LEN_MOUNT_POINT, num_field_chars));
            break;
         default:
            break;
      }
      j++;
   }

   free(buf_copy);
   return j;
}

//*****************************************************************************

void capture_device_info() {
   FILE* f;
   char path[256];
   device_config dc;
   int fields_parsed;

   f = popen("lsblk -P", "r");
   if (f == NULL) {
      return;
   }

   /* read output one line at a time */
   while (fgets(path, sizeof(path)-1, f) != NULL) {
      fields_parsed = parse_device_fields(path, &dc);
      if (fields_parsed == 7) {
         if (!strcmp(dc.mount_point, "/")) {
            root_fs_dev = string(dc.device_name);
         } else {
            fs_mount_path_to_dev[string(dc.mount_point)] = string(dc.device_name);
         }
      }
   }
}

//*****************************************************************************

const string& path_to_dev(const string& path) {
   const map<string,string>::const_iterator itEnd = fs_mount_path_to_dev.end();
   map<string,string>::const_iterator it = fs_mount_path_to_dev.begin();

   for (; it != itEnd; it++) {
      const string& mount_point = it->first;
      if (!path.compare(0, mount_point.size(), mount_point)) {
         return it->second;
      }
   }

   return root_fs_dev;
}

//*****************************************************************************

void register_file(struct monitor_record_t* rec) {
   if ((rec->fd != FD_NONE) && (rec->s1 != NULL) && (strlen(rec->s1) > 0)) {
      const string& dev_id = path_to_dev(string(rec->s1));
      fd_to_dev[rec->fd] = dev_id;
   }
}

//*****************************************************************************

void deregister_file(struct monitor_record_t* rec) {
   if (rec->fd != FD_NONE) {
      map<int,string>::iterator it = fd_to_dev.find(rec->fd);
      if (it != fd_to_dev.end()) {
         fd_to_dev.erase(it);
      }
   }
}

//*****************************************************************************

void resolve_file(struct monitor_record_t* rec) {
   if (rec->fd != FD_NONE) {
      const map<int,string>::const_iterator it = fd_to_dev.find(rec->fd);
      if (it != fd_to_dev.end()) {
         const string& dev_id = it->second;
         strncpy(rec->device, dev_id.c_str(), DEVICE_LEN); 
      }
   }
}

//*****************************************************************************

