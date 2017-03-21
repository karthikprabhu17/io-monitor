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
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <dlfcn.h>
#include <errno.h>
#include <pthread.h>

#include "domains.h"
#include "ops.h"
#include "ops_names.h"
#include "domains_names.h"
#include "mq.h"
#include "plugin.h"
#include "plugin_chain.h"
#include "command_parser.h"

/* global mutex for plugin items, to prevent plugin chain alteration from happening
 * during plugin execution */
pthread_mutex_t plugin_mutex = PTHREAD_MUTEX_INITIALIZER;

#define plugins_lock() pthread_mutex_lock(&plugin_mutex)
#define plugins_unlock() pthread_mutex_unlock(&plugin_mutex)

struct plugin_chain* plugins = NULL;

struct listener listener = 
{
  parse_command
};

int execute_plugin_chain(struct monitor_record_t *rec)
{
  struct plugin_chain* p = plugins;
  int rc_plugin;
  plugins_lock();
  while (p) {
    if (p->plugin_paused) {
      rc_plugin = p->pfn_ok_to_accept_data(p->state);
      if (rc_plugin == PLUGIN_ACCEPT_DATA) {
	p->plugin_paused = 0;
      }
    }

    if (!p->plugin_paused) {
      rc_plugin = p->pfn_process_data(rec,p->state);
      if (rc_plugin == PLUGIN_REFUSE_DATA) {
	p->plugin_paused = 1;
      }
      if (rc_plugin == PLUGIN_DROP_DATA) {
	plugins_unlock();
	return 0;
      }
    }
    p = p->next_plugin;
  }
  plugins_unlock();
  return 0;
}

/**
 * function unloads plugin so and all allocated resources but it 
 * does not remove plugin from the linked list. It is responsibility
 * of caller and it is requirement to do so 
 */
void unload_plugin(struct plugin_chain* p)
{
  p->pfn_close_plugin(p->state);
  free_command(p->plugin_library);
  printf("Closed plugin %s\n", p->plugin_library);
  dlclose(p->plugin_handle);
  free((char*)p->plugin_library);
  if (p->plugin_options)
    free((char*)p->plugin_options);
  if (p->plugin_alias)
    free((char*)p->plugin_alias);
}

void unload_all_plugins() {
  plugins_lock();
  while (plugins) {
    unload_plugin(plugins);
    plugins = plugins->next_plugin;
  }
  plugins = NULL;
  plugins_unlock();
}

/**
 * returns number of currently loaded plugins
 */
int count_plugins()
{
  struct plugin_chain* p = plugins;
  int num_plugins = 0;
  char** result;
  
  while (p) {
    num_plugins++;
    p=p->next_plugin;
  }
  return num_plugins;
}

char** list_plugins()
{
  struct plugin_chain* p = plugins;
  int num_plugins = count_plugins();
  char** result;
  
  result = calloc(sizeof(char*), num_plugins+1);
  if (!result)
    return 0;

  p = plugins;
  num_plugins = 0;
  while (p) {
    int s = strlen(p->plugin_library);
    if (p->plugin_alias) {
      s += 1 + strlen(p->plugin_alias);
    }

    result[num_plugins] = malloc(s);
    strcpy(result[num_plugins], (char*)p->plugin_library);
    /* append alias if applicable */
    if (p->plugin_alias) {
      sprintf(result[num_plugins] + strlen(p->plugin_library),
	      ":%s", p->plugin_alias);
    }
    num_plugins++;
    p=p->next_plugin;
  }
  return result;
}

/* Name of plugin can be either its alias or 
 * library path;
 */
struct plugin_chain* locate_plugin_by_name(const char* name)
{
  struct plugin_chain* p = plugins;
  char** result;
  
  while (p) {
    if (!strcmp(name, p->plugin_library)
	|| (p->plugin_alias && !strcmp(p->plugin_alias, name))) {
      return p;
    } else {
      p = p->next_plugin;
    }
  }
  return 0;  
}

int unload_plugin_by_name(const char* name)
{
  struct plugin_chain* p = locate_plugin_by_name(name);
  struct plugin_chain* i = plugins;
  plugins_lock();
  unload_plugin(p);
  /* unlink plugin from the list */
  if (plugins==p) {
    /* case when it's head (replace head) */
    plugins=p->next_plugin;
    free(p);
    plugins_unlock();
    return 0;
  } else {
    /* case when plugin in question is not head */
    while (i) {
      if (i->next_plugin == p) {
	i->next_plugin = p->next_plugin;
	free(p);
        plugins_unlock();
	return 0;
      }
      i = i->next_plugin;
    } /*while*/
    fprintf(stderr, "Failed to correctly unload plugin."
	    " This is likely a bug. mq_listener will now quit.");
    plugins_unlock();
    return 1;
  }
}


int load_plugin(const char* library, const char* options, const char* alias)
{
  struct plugin_chain* new_plugin = calloc(1, sizeof(struct plugin_chain));
  if (!new_plugin) {
    return 1;
  }
  new_plugin->plugin_library = strdup(library);
  new_plugin->plugin_options = options? strdup(options) : NULL;
  new_plugin->plugin_handle = dlopen(new_plugin->plugin_library, RTLD_NOW);

  if (NULL == new_plugin->plugin_handle) {
    printf("error: unable to open plugin library '%s'\n%s\n",
	   new_plugin->plugin_library,
	   dlerror());
    return 1;
  }

  /* copy alias so that plugin will be found under its alias */
  if (alias) {
    new_plugin->plugin_alias = strdup(alias);
  } else {
    new_plugin->plugin_alias = NULL;
  }
  
  new_plugin->pfn_open_plugin =
    (PFN_OPEN_PLUGIN) dlsym(new_plugin->plugin_handle, "open_plugin");
  new_plugin->pfn_close_plugin =
    (PFN_CLOSE_PLUGIN) dlsym(new_plugin->plugin_handle, "close_plugin");
  new_plugin->pfn_ok_to_accept_data =
    (PFN_OK_TO_ACCEPT_DATA) dlsym(new_plugin->plugin_handle, "ok_to_accept_data");
  new_plugin->pfn_process_data =
    (PFN_PROCESS_DATA) dlsym(new_plugin->plugin_handle, "process_data"); 

  new_plugin->pfn_plugin_command =
    (PFN_PLUGIN_COMMAND) dlsym(new_plugin->plugin_handle, "plugin_command"); 
  new_plugin->pfn_list_commands =
    (PFN_LIST_COMMANDS) dlsym(new_plugin->plugin_handle, "list_commands"); 

  if ((NULL == new_plugin->pfn_open_plugin) ||
      (NULL == new_plugin->pfn_close_plugin) ||
      (NULL == new_plugin->pfn_ok_to_accept_data) ||
      (NULL == new_plugin->pfn_process_data)) {
    dlclose(new_plugin->plugin_handle);
    printf("error: plugin missing 1 or more entry points\n");
    return 1;
  }

  int rc_plugin = (*new_plugin->pfn_open_plugin)
    (new_plugin->plugin_options,
     &listener, &new_plugin->state);
  if (rc_plugin == PLUGIN_OPEN_FAIL) {
    dlclose(new_plugin->plugin_handle);
    printf("error: unable to initialize plugin\n");
    return 1;
  }

  if (new_plugin->pfn_plugin_command) {
    struct command _command =
      { "", "",
	"", "",
	new_plugin->pfn_plugin_command,
	0, 0, new_plugin->state
      };
    sprintf(_command.name, "%s", library);
    sprintf(_command.short_name, "%s",alias? alias: library);
    set_command(&_command);
  }

  
  plugins_lock();
  if (plugins) {
    struct plugin_chain* tmp = plugins;
    while (tmp->next_plugin)
      tmp = tmp->next_plugin;
    tmp->next_plugin = new_plugin;
  } else {
    plugins=new_plugin;
  }
  plugins_unlock();
  
  return 0;
}
