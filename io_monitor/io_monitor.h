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

#ifndef __IO_MONITOR_H
#define __IO_MONITOR_H


#define DECL_VARS() \
struct timeval start_time, end_time; \
int error_code = 0;

#define GET_START_TIME() \
gettimeofday(&start_time, NULL);

#define GET_END_TIME() \
gettimeofday(&end_time, NULL);

#define TIME_BEFORE() \
&start_time

#define TIME_AFTER() \
&end_time

// a debugging aid that we can easily turn off/on
#ifdef NDEBUG
#define PUTS(s)
#else
#define PUTS(s) \
puts(s);
#endif

// environment variables that we respond to
static const char* ENV_FACILITY_ID = "FACILITY_ID";
static const char* ENV_MESSAGE_QUEUE_PATH = "MESSAGE_QUEUE_PATH";
static const char* ENV_START_ON_OPEN = "START_ON_OPEN";
static const char* ENV_MONITOR_DOMAINS = "MONITOR_DOMAINS";
static const char* ENV_START_ON_ELAPSED = "START_ON_ELAPSED";
static const char* ENV_COUNT_SAMPLE_FREQUENCY = "COUNT_SAMPLE_FREQUENCY";
static const char* ENV_TIME_SAMPLE_FREQUENCY = "TIME_SAMPLE_FREQUENCY";
static const char* ENV_TIME_SAMPLE_DURATION = "TIME_SAMPLE_DURATION";

#define CHECK_LOADED_FNS() \
if (NULL == orig_open) initialize_monitor();

static const int SOCKET_PORT = 8001;
//static const int DOMAIN_UNSPECIFIED = -1;
//static const int FD_NONE = -1;
static const int MQ_KEY_NONE = -1;


static const ssize_t ZERO_BYTES = 0L;
static const char* message_queue_path = NULL;
static const int message_project_id = 'm';



// set up bit flags for each domain
static unsigned int BIT_LINKS = (1 << LINKS);
static unsigned int BIT_XATTRS = (1 << XATTRS);
static unsigned int BIT_DIRS = (1 << DIRS);
static unsigned int BIT_FILE_SYSTEMS = (1 << FILE_SYSTEMS);
static unsigned int BIT_FILE_DESCRIPTORS = (1 << FILE_DESCRIPTORS);
static unsigned int BIT_SYNCS = (1 << SYNCS);
static unsigned int BIT_SOCKETS = (1 << SOCKETS);
static unsigned int BIT_SEEKS = (1 << SEEKS);
static unsigned int BIT_FILE_SPACE = (1 << FILE_SPACE);
static unsigned int BIT_PROCESSES = (1 << PROCESSES);
static unsigned int BIT_FILE_METADATA = (1 << FILE_METADATA);
static unsigned int BIT_FILE_WRITE = (1 << FILE_WRITE);
static unsigned int BIT_FILE_READ = (1 << FILE_READ);
static unsigned int BIT_FILE_OPEN_CLOSE = (1 << FILE_OPEN_CLOSE);
static unsigned int BIT_MISC = (1 << MISC);
static unsigned int BIT_DIR_METADATA = (1 << DIR_METADATA);
static unsigned int BIT_START_STOP = (1 << START_STOP);
static unsigned int BIT_HTTP = (1 << HTTP);

#endif
