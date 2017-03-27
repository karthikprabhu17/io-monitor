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

#include "plugin.h"
#include "monitor_record.h"
#include "domains_names.h"
#include "ops_names.h"


//*****************************************************************************

int open_plugin(const char* plugin_config)
{
   return PLUGIN_OPEN_SUCCESS;
}

//*****************************************************************************

void close_plugin()
{
}

//*****************************************************************************

int ok_to_accept_data()
{
   return PLUGIN_ACCEPT_DATA;
}

//*****************************************************************************

int process_data(struct monitor_record_t* data)
{
   printf("%s,%s,%d,%f,%d,%s,%s,%d,%d,%zu,%s,%s\n",
          data->facility,
          data->device,
          data->timestamp,
          data->elapsed_time,
          data->pid,
          domains_names[data->dom_type],
          ops_names[data->op_type], data->error_code, data->fd,
          data->bytes_transferred, data->s1, data->s2);
   fflush(stdout);
   return PLUGIN_ACCEPT_DATA;
}

//*****************************************************************************

