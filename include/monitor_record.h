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

#ifndef __MONITOR_RECORD_H
#define __MONITOR_RECORD_H

#ifdef __FreeBSD__
#include <sys/syslimits.h>
#else
#include <linux/limits.h>
#endif
#define STR_LEN 256
#define HOSTNAME_LEN 64

struct monitor_record_t {
  char facility[STR_LEN];
  char hostname[HOSTNAME_LEN];
  int timestamp;
  float elapsed_time;
  int pid;

  int dom_type;
  int op_type;

  int error_code;
  int fd;
  size_t bytes_transferred;
  char s1[PATH_MAX];
  char s2[STR_LEN];
};

#endif
