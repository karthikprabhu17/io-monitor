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

//*****************************************************************************
static struct command * s_commands;

void set_commands_array(struct command* _commands)
{
  s_commands = _commands;
}

/*
 * \param name name of command to be executed
 * \param args nullptr terminated array of command line arguments
 */
int run_command(const char* name, const char** args)
{
  for (int i = 0; s_commands[i].command_function; ++i) {
    if (!strcmp(s_commands[i].name, name) ||
	!strcmp(s_commands[i].short_name, name)) {
      return s_commands[i].command_function(name, args);
    }
  }
  fprintf(stderr, "Not found: %s. Use -h/--help to see possible commands.\n",
	  name);
  return 1;
}

//*****************************************************************************

int parse_command(const char* buf)
{
  char* command_tokenized[128];
  char* token;
  char* buf_copy = strdup(buf);
  char* rest = buf_copy;
  int i;
  int j=0;
    
  while (token = strtok_r(rest , " ", &rest)) {
    command_tokenized[j++] = strdup(token);
  }
  command_tokenized[j] = NULL;
  
  run_command(command_tokenized[0], (const char**)&(command_tokenized[1]));
  j=0;
  while (command_tokenized[j]) {
    free(command_tokenized[j++]);
  }
  free(buf_copy);
}

//*****************************************************************************

int free_command(char** name, char** args)
{
  char ** arg = args;
  free(*name);
  *name = 0;
  while (*arg) {
    free(*arg);
    *arg=0;
    arg++;
  }

}
//*****************************************************************************

/* parse command line arguments */
int parse_args(int argc, char* argv[])
{
  char* command = 0;
  char* command_parameters[128];
  
  int apn = -1; /* argument parameter number */
  int i;
  for (int i = 1; i!= argc ; ++i) {
    if (argv[i][0]=='-') {
      if (command) {
	command_parameters[apn] = NULL;
	int rc = run_command(command, (const char**)command_parameters);
	free_command(&command, command_parameters);
	if (rc) {
	  fprintf(stderr, "Execution error. Quitting\n");
	  return -1;
	}
	apn = -1;
	command = 0;
      }
      if (argv[i][1]=='-') {
	if (argv[i][2]==0) {
	  fprintf(stderr, "Syntax error near argument %d - command expected\n", i);
	  return -1;
	} else {
	  command = strdup(argv[i]+2);
	  apn = 0;
	}
      } else {
	if (argv[i][1]==0) {
	  fprintf(stderr, "Syntax error near argument %d - command expected (short)\n", i);
	  return -1;
	} else {
	  command = strdup(argv[i]+1);
	  apn = 0;
	}
      } 
    } else {
      /* a parameter to argument */
      if (apn < 0) {
	fprintf(stderr, "Syntax error near argument %d: - expected.\n", i);
	return -1;
      } else {
	command_parameters[apn++] = strdup(argv[i]);
      }
    }
  }

  if (command) {
    command_parameters[apn] = NULL;
    int rc = run_command(command, (const char**)command_parameters);
    free_command(&command, command_parameters);
    if (rc) {
      fprintf(stderr, "Execution error. Quitting\n");
      return -1;
    } else {
      return 0;
    }
  } else {
    fprintf(stderr, "Syntax error - argument missing.\n");
    return -1;
  }
}
