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

#ifndef __OPS_H
#define __OPS_H


// define the operations that we're tracking. please note that in some
// cases several C library calls all get identified the same (e.g., open,
// open64, fopen are all recorded as OPEN).
typedef enum {
   OPEN,           //  (FILE_OPEN_CLOSE)
   CLOSE,          //  (FILE_OPEN_CLOSE)
   WRITE,          //  (FILE_WRITE)
   READ,           //  (FILE_READ)
   SYNC,           //  (SYNCS)
   SEEK,           //  (SEEKS)
   RENAME,         //  (MISC)
   LINK,           //  (LINKS)
   UNLINK,         //  (LINKS)
   FCNTL,          //  (FILE_DESCRIPTORS)
   DUP,            //  (FILE_DESCRIPTORS)
   STAT,           //  (FILE_METADATA)
   ACCESS,         //  (FILE_METADATA)
   CHMOD,          //  (FILE_METADATA)
   CHOWN,          //  (FILE_METADATA)
   FLOCK,          //  (MISC)
   READLINK,       //  (LINKS)
   UTIME,          //  (FILE_METADATA)
   GETXATTR,       //  (XATTRS)
   SETXATTR,       //  (XATTRS)
   LISTXATTR,      //  (XATTRS)
   REMOVEXATTR,    //  (XATTRS)
   MOUNT,          //  (FILE_SYSTEMS)
   UMOUNT,         //  (FILE_SYSTEMS)
   FORK,           //  (PROCESSES)
   MKNOD,          //  (MISC)
   MKDIR,          //  (DIRS)
   RMDIR,          //  (DIRS)
   CHDIR,          //  (DIRS)
   CHROOT,         //  (MISC)
   SOCKET,         //  (SOCKETS)
   FLUSH,          //  (SYNCS)
   ALLOCATE,       //  (FILE_SPACE)
   TRUNCATE,       //  (FILE_SPACE)
   OPENDIR,        //  (DIR_METADATA)
   CLOSEDIR,       //  (DIR_METADATA)
   READDIR,        //  (DIR_METADATA)
   SEEKDIR,        //  (DIR_METADATA)
   TELLDIR,        //  (DIR_METADATA)
   REWINDDIR,      //  (DIR_METADATA)
   SCANDIR,        //  (DIR_METADATA)
   CONNECT,        //  (SOCKETS)
   ACCEPT,         //  (SOCKETS)
   LISTEN,         //  (SOCKETS)
   BIND,           //  (SOCKETS)
   EXEC,           //  (PROCESSES)
   KILL,           //  (PROCESSES)
   
   // operations listed below are NOT directly associated with
   // C functions
   START,          // Start execution of program
   STOP,           // Stop execution of program
   HTTP_REQ_SEND,  // Send an HTTP request. s1 will contain verb and URL
   HTTP_REQ_RECV,  // Get an HTTP request.  s1 will contain verb and URL
   HTTP_RESP_SEND, // Send an HTTP response. error code field will contain response code
   HTTP_RESP_RECV, // Receive HTTP response
   HTTP_RESP_FINI_SEND, // Sent final byte of HTTP response
   HTTP_RESP_FINI_RECV, // Receive final byte of HTTP response

   MUTEX_LOCK,
   MUTEX_UNLOCK,
   MUTEX_INIT,
   MUTEX_DESTROY,
   THREAD_CREATE,
   COND_WAIT,
   COND_SIGNAL,
   COND_BROADCAST,
   
   END_OPS         // keep this one as last
} OP_TYPE;

#endif
