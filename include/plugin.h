#ifndef PLUGIN_H
#define PLUGIN_H

#define PLUGIN_OPEN_SUCCESS  0
#define PLUGIN_OPEN_FAIL 1

/*
 * default answer for PFN_PROCESS_DATA (data was written/passed/otherwise
 * accepted 
 */
#define PLUGIN_ACCEPT_DATA  1

/*
 * plugin did not process data i.e. because it's a plugin dumping data
 * to remote database and database is busy/offline. Doesn't cause data
 * to be dropped from other plugins
 */
#define PLUGIN_REFUSE_DATA  0

/*
 * drop data - response issued typically by filter type plugins. causes
 * data not to be passed to plugins loaded after this one.
 */
#define PLUGIN_DROP_DATA   -1

#include "monitor_record.h"
/* structure passed to open function - passes parameters
 * containing state of mq_listener program, including
 * handles allowing interaction with it */
struct listener {
  int (*command_function)(const char* buf);
};
/* plugin needs to expose at least following four functions */

/* each plugin _must_ provide following four entry points */
/* each of these functions have "state" parameter which can be ignored or used.
 * if it is used, it can be used to hold state between invocations of plugin (instead of usage of
 * global variables which won't work if there are multiple instances of plugin loaded */

/* function open_plugin adhering to prototype below:
 * will be called immediately on plugin open */
typedef int (*PFN_OPEN_PLUGIN)(const char*, struct listener *, void** state);

/* function close_plugin adhering to prototype below:
 * will be called immediately on plugin open */
typedef void (*PFN_CLOSE_PLUGIN)(void* state);

/* function ok_to_accept_data adhering to prototype below:
 * will be called periodically after plugin responded
 * PLUGIN_REFUSE_DATA to process_data */
typedef int (*PFN_OK_TO_ACCEPT_DATA)(void* state);

/* function process_data adhering to prototype below:
 * will be called once per every incoming datapoint */
typedef int (*PFN_PROCESS_DATA)(struct monitor_record_t*, void* state);

/* plugin may (this is however optional) expose following function
 * in addition to functions mentioned above */

/* function plugin_command adhering to prototype below: 
 * commands may be used to alter behavior of plugin. I.e. if it is filter plugin, 
 * filtering scope can be changed and if it is an output plugin, format can be adjusted */
typedef int (*PFN_PLUGIN_COMMAND)(const char* name, const char** args, void* state);

/* function returning list of commands supported by plugin */
typedef char** (*PFN_LIST_COMMANDS)(void* state);

/* note, it is advisable that if plugin supports commands,
 * one of commands supported is help, giving brief description of plugin and
 * its available commands. Even if plugin doesn't expose any commands that
 * alter its behavior, it is good to expose help command */

/*
  for convenience of plugin author, correctly named prototypes
  are provided below.

  int open_plugin(const char* plugin_config);
  void close_plugin();
  int ok_to_accept_data();
  int process_data(struct monitor_record_t* data);
  char **list_commands();
  int plugin_command(const char* name, const char** args);
*/

#endif

