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
#include <stdlib.h>

#include "plugin.h"
#include "monitor_record.h"
#include "domains_names.h"
#include "ops_names.h"

struct plugin_state {
  int kill_thread;
  pthread_t input_thread;
  struct listener * listener;
};
  
void *thread_loop(void* param)
{
  struct plugin_state * ps = param;
  struct listener * listener = ps->listener;

  char buf[PATH_MAX];
 
  while (!ps->kill_thread) {
    printf("mq_listener> ");
    fflush(stdout);
    fgets(buf, PATH_MAX, stdin);
    listener->command_function(buf);
  }
  puts("quitting mq_listener cli");
}

//*****************************************************************************

int open_plugin(const char* plugin_config, struct listener * listener, void *param)
{
  struct plugin_state ** ps = param;
  *ps = malloc(sizeof (struct plugin_state));
  (*ps)->kill_thread = 0;
  (*ps)->listener = listener;
  pthread_create(&(*ps)->input_thread, NULL, thread_loop, *ps);
  return PLUGIN_OPEN_SUCCESS;
}

//*****************************************************************************

void close_plugin(void *param)
{
  struct plugin_state * ps = param;
  puts("Close request received; joining thread");
  ps->kill_thread = 1;
  pthread_join(ps->input_thread, NULL);
  free(ps);
  puts("Joined");
}

//*****************************************************************************

int ok_to_accept_data(void *param)
{
   return PLUGIN_ACCEPT_DATA;
}

//*****************************************************************************

int process_data(struct monitor_record_t* data, void *param)
{
   return PLUGIN_ACCEPT_DATA;
}

//*****************************************************************************

