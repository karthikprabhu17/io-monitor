#ifndef __PLUGIN_CHAIN_H
#define __PLUGIN_CHAIN_H

struct plugin_chain {
  const char* plugin_library;
  const char* plugin_options;
  const char* plugin_alias;   /* alternative method to access plugin in management routines */
  PFN_OPEN_PLUGIN pfn_open_plugin;
  PFN_CLOSE_PLUGIN pfn_close_plugin;
  PFN_OK_TO_ACCEPT_DATA pfn_ok_to_accept_data;
  PFN_PROCESS_DATA pfn_process_data;
  int plugin_paused;
  void* plugin_handle;
  struct plugin_chain* next_plugin;
};

int execute_plugin_chain(struct monitor_record_t *rec);

void unload_all_plugins();

int load_plugin(const char* library, const char* options, const char* alias);

int unload_plugin_by_name(const char* name);

int reorder_plugins(const char** names);

/* result is malloc-allocated; it is responsibility of caller to free it */
const char** list_plugins();
#endif
