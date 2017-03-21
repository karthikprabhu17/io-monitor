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

#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "plugin.h"
#include "monitor_record.h"
#include "domains.h"
#include "ops.h"
#include "domains_names.h"
#include "ops_names.h"
#include "utility_routines.h"

//*****************************************************************************
struct plugin_state {
  unsigned int domain_bit_flags;
};

int open_plugin(const char* plugin_config, struct listener * listener, void *param)
{
  struct plugin_state ** ps = param;
  *ps = malloc(sizeof (struct plugin_state));
  (*ps)->domain_bit_flags=domain_list_to_bit_mask(plugin_config);
  return PLUGIN_OPEN_SUCCESS;
}

//*****************************************************************************

void close_plugin(void *param)
{
}

//*****************************************************************************

int ok_to_accept_data()
{
   return PLUGIN_ACCEPT_DATA;
}

//*****************************************************************************

int process_data(struct monitor_record_t* data, void *param)
{
  struct plugin_state * ps = param;
  const unsigned int domain_bit_flag = 1 << data->dom_type;
  if (0 == (ps->domain_bit_flags & domain_bit_flag)) {
    return PLUGIN_DROP_DATA;
  } else { 
    return PLUGIN_ACCEPT_DATA;
  }
}

//*****************************************************************************

char **list_commands()
{
  static const char* command_list[] =
    {"update-mask", "print-mask", "help", 0};
  return (char**)command_list;
}

//*****************************************************************************

int plugin_command(const char* name, const char** args, void *param)
{
  struct plugin_state * ps = param;

  if (args[0] && !strcmp(args[0], "update-mask") && args[1]) {
    ps->domain_bit_flags=domain_list_to_bit_mask(args[1]);
  } else if (args[0] && !strcmp(args[0], "print-mask")) {
    printf("domain_mask:");
    /* print all the enabled bit fields */
    int j = 0;
    unsigned int m = ps->domain_bit_flags;
    for (j = 0 ; m ; j++) {
      unsigned int i = 1 << j;
      if (i&m) {
	printf("%s", domains_names[j]);
	m&= ~i;
	if(m)
	  putchar(',');
      }
    }
    putchar('\n');
  }
}

//*****************************************************************************

