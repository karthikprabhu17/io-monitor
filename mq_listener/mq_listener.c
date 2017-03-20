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
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>
#include "domains.h"
#include "ops.h"
#include "ops_names.h"
#include "domains_names.h"
#include "mq.h"
#include "plugin.h"
#include "plugin_chain.h"
#include "command_parser.h"

static const int MESSAGE_QUEUE_PROJECT_ID = 'm';

int c_mq_path(const char* name, const char** args);
int c_load_plugin(const char* name, const char** args);
int c_config(const char* name, const char** args);
int c_help(const char* name, const char** args);
int c_unload_plugin(const char* name, const char** args);
int c_reorder_plugins(const char* name, const char** args);
int c_quit(const char* name, const char** args);

struct command commands[] =
  {
    {"load-plugin", "p",
     "<plugin-library>[:alias] [plugin-options]",
     "Start program with particular plugin enabled;"
     " Plugin is loaded as so library. After path to library you may add alias"
     " (useful when you will have multiple instances of given plugin loaded, and"
     " may want to unload one of them)."
     " After that you may supply parameter string for your plugin.",
     c_load_plugin,0},
    {"unload-plugin", "u",
     "<plugin-library-path or alias>",
     "remove plugin from running image of mq_listener",
     c_unload_plugin,1},
    {"reorder-plugins", "r",
     "<comma separated list of plugins, referred to by paths or aliases>",
     "It is required to give exhaustive list of all plugins that are loaded; otherwise "
     "command will be rejected",
     c_reorder_plugins,1},
    {"quit", "q",
     "",
     "exit instance of mq_listener",
     c_quit, 1},
    {"mq-path", "m",
     "<path>",
     "Select message queue file. This parameter is mandatory unless config file is used",
    c_mq_path,0},
    {"config", "c",
     "<path>",
     "Start mq_listener with particular config file",
     c_config,0},
    {"help", "h",
     "",
     "Print help message",
     c_help,0},
    {"","","","",NULL,0}
  };


//*****************************************************************************
int input_loop();

int show_runtime_commands = 0;
int message_queue_key = -1;
int message_queue_id = -1;

int main(int argc, char** argv)
{
  set_commands_array(commands);
  int rc =  parse_args(argc, argv);
  if (rc) {
    return rc;
  } else {
    if (message_queue_id == -1) {
      fprintf(stderr, "You need to provide message queue either "
	      "via config file or via --mq-path/-m command line option\n");
      return 1;
    }
    show_runtime_commands = 1;
    return input_loop();
  }
  
  unload_all_plugins();
   
  return 0;
}

//*****************************************************************************

int input_loop()
{
   MONITOR_MESSAGE monitor_message;
   ssize_t message_size_received;
   while (1) {
      memset(&monitor_message, 0, sizeof(MONITOR_MESSAGE));
      message_size_received =
         msgrcv(message_queue_id,
                &monitor_message,  // void* ptr
                sizeof(struct monitor_record_t),  // size_t nbytes
                0,   // long type
                0);  // int flag
      if (message_size_received > 0) {
	execute_plugin_chain(&monitor_message.monitor_record);
      } else {
	fprintf(stderr, "rc = %zu\n", message_size_received);
	fprintf(stderr, "errno = %d\n", errno);
      }
   }
}

//*****************************************************************************

int c_mq_path(const char* name, const char** args)
{
  const char* message_queue_path;

  if (!args[0]) {
    fprintf(stderr, "error: Message queue path is required\n");
  } else {
    message_queue_path = args[0];
  }
  
  message_queue_key = ftok(message_queue_path, MESSAGE_QUEUE_PROJECT_ID);
  if (message_queue_key == -1) {
    fprintf(stderr, "error: unable to obtain key for message queue path '%s'\n",
	    message_queue_path);
    fprintf(stderr, "errno: %d\n", errno);
    exit(1);
  }
  
  message_queue_id = msgget(message_queue_key, (0664 | IPC_CREAT));
  if (message_queue_id == -1) {
    fprintf(stderr, "error: unable to obtain id for message queue path '%s'\n",
	    message_queue_path);
    fprintf(stderr, "errno: %d\n", errno);
    exit(1);
  }
  return 0;
}

//*****************************************************************************

int c_load_plugin(const char* name, const char** args)
{
  const char* plugin_library = 0;
  const char* plugin_options = 0;
  const char* plugin_alias = 0;
  /* TODO extract alias */
  plugin_library = args[0];
  plugin_options = args[1];
  printf("Attempting to load plugin %s\n", plugin_library);
  int res = load_plugin(plugin_library, plugin_options, NULL);
  if (res) {
    fprintf(stderr, "Filed to load plugin. Will now quit\n");
    exit(1);    
  } else {
    printf("Load successful\n");
    return 0;
  }
}

//*****************************************************************************

int c_config(const char* name, const char** args)
{
  if (!args[0]) {
    fprintf(stderr, "Path to cfg missing\n");
  }
  FILE* f = fopen(args[0],"r");
  char buf[PATH_MAX];
  while (fgets(buf, PATH_MAX, f)) {
    if (buf[0] != '#')
      parse_command(buf);
  }
  fclose(f);
  return 0;
}

//*****************************************************************************

int c_help(const char* name, const char** args)
{
  int i;
  puts("mq_listener: listening end of io_monitor.");
  puts("Example invocation:");
  puts("   ./mq_listener/mq_listener -m mq1 -p plugins/output_table.so");
  puts("   ./mq_listener/mq_listener -c mq_listener/listener.conf.example");
  for (i = 0; commands[i].command_function; ++i) {
    if (commands[i].interactive_only && !show_runtime_commands)
      continue;

    if (show_runtime_commands) {
      printf("%s (%s) %s\n", commands[i].name,
	     commands[i].short_name, commands[i].params_desc);
    } else {
      printf("--%s / -%s %s\n", commands[i].name,
	     commands[i].short_name, commands[i].params_desc);
    }
      printf("     %s\n", commands[i].help);
  }
  return 0;
}

//*****************************************************************************

int c_unload_plugin(const char* name, const char** args)
{
}

//*****************************************************************************

int c_reorder_plugins(const char* name, const char** args)
{

}

//*****************************************************************************

int c_quit(const char* name, const char** args)
{
  puts("quitting");
  unload_all_plugins();
  exit(1);
}
