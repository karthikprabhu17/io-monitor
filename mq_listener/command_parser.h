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

#ifndef __COMMAND_PARSER_H
#define __COMMAND_PARSER_H

typedef int (*command_fun)(const char* name, const char** args);

/* structure for each command */
struct command
{
  char name[256];     /* full name of command */
  char short_name[16]; /* typically one character or abbreviation */

  char params_desc[512]; /* for short help line */
  char help[4096]; /* broader description */

  command_fun command_function; /* function invoked when command
				 * is detected by interpreter */
  
  int interactive_only; /* skip this in help message with -h/--help 
			 * invocation */
  
  struct command * next_command; /* used internally (as command_parser
				  * internally uses linked lists); 
				  * in input parameters to set_c* functions,
				  * this field is ignored */
};

/* add command to comand list */
void set_command(struct command* _command);

/* add array of commands to comand list */
void set_commands_array(struct command* _commands);

/* parse command supplied on CLI input or from conf file */
int parse_command(const char* buf);

/* parse command line input arguments */
int parse_args(int argc, char* argv[]);

/* remove command from interpreter buffers */
void free_command(const char* name);

/* clear all commands buffers */
void free_commands();

#endif
