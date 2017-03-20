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

// This plugin demonstrates simple interactive user interface for mq_listener
// It is mainly developed to test components of plugin architecture which will later
// be essential in implementation of remote (HTTP based) management interface


#include <stdio.h>
#include <pthread.h>

#include "plugin.h"
#include "monitor_record.h"
#include "domains_names.h"
#include "ops_names.h"

static int kill_thread = 0;
static pthread_t input_thread;

void *thread_loop(void* param)
{
  struct listener * listener = param;
  char buf[PATH_MAX];
 
  while (!kill_thread) {
    printf("mq_listener> ");
    fflush(stdout);
    fgets(buf, PATH_MAX, stdin);
    listener->command_function(buf);

  }
}

//*****************************************************************************

int open_plugin(const char* plugin_config, struct listener * listener)
{
  pthread_create(&input_thread, NULL, thread_loop, listener);
  return PLUGIN_OPEN_SUCCESS;
}

//*****************************************************************************

void close_plugin()
{
  kill_thread = 1;
  pthread_join(input_thread, NULL);
}

//*****************************************************************************

int ok_to_accept_data()
{
   return PLUGIN_ACCEPT_DATA;
}

//*****************************************************************************

int process_data(struct monitor_record_t* data)
{
   return PLUGIN_ACCEPT_DATA;
}

//*****************************************************************************

