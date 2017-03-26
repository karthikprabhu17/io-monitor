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

#ifndef __DOMAINS_H
#define __DOMAINS_H

// set up some categories to group metrics
typedef enum {
// domains below are associated with function calls
   LINKS,             // 0  (link, unlink)
   XATTRS,            // 1  (getxattr, setxattr, listxattr)
   DIRS,              // 2  (mkdir, rmdir, chdir)
   FILE_SYSTEMS,      // 3  (mount, umount)
   FILE_DESCRIPTORS,  // 4  (dup, fcntl)
   SYNCS,             // 5  (sync, flush)
   SOCKETS,           // 6  (socket, accept, bind, connect, send, recv)
   SEEKS,             // 7  (fseek, lseek)
   FILE_SPACE,        // 8  (fallocate, ftruncate)
   PROCESSES,         // 9  (fork, exec, kill, exit)
   FILE_METADATA,     // 10  (stat, access, chmod, chown)
   FILE_WRITE,        // 11  (write, send etc.)
   FILE_READ,         // 12  (read, recv etc.)
   FILE_OPEN_CLOSE,   // 13  (open, close)
   MISC,              // 14  (miscellaneous; rename, flock, mknod, chroot, etc.)
   DIR_METADATA,      // 15  (opendir, readdir, seekdir, etc.)
// domains below are associated with system events not tied directly to function calls
   START_STOP,        // 17  (associated with starting and exiting an app)
   HTTP,              // 18  (HTTP verb events)
   THREADS,           // 19
   END_DOMAINS        // keep this one as last
} DOMAIN_TYPE;

#endif

