# io-monitor

## Overview

This is a thin shim intended to be used with **LD_PRELOAD** in order
to capture/intercept many C library calls for the purpose of
getting real-time metrics without having to change any existing
source code. The idea is that relevant metrics should be
captured very efficiently and then handed off to another process
to handle.

## IPC Mechanism

The preferred IPC mechanism is **Unix SysV message queues**. In order
to make use of the captured metrics, you must set the environment
variable **MESSAGE_QUEUE_PATH** to an existing file where user has
permissions for writing.

## Identifying Metrics

Each captured metric has an **operation type** to identify the kind
of operation that generated the metric. In some cases, the specified
operation is a family of functions grouped by functionality.
For example, the 'OPEN' operation on files can be one of the
following functions: open, open64, creat, creat64, fopen, fopen64.

Every operation type belongs to a **domain**. The domain is simply
a grouping mechanism to treat a certain logically related set
of operations as one (e.g., to enable/disable monitoring).

## Operations

| Operation     | Domain           | Functions |
| ---------     | ------           | --------- |
| CLOSEDIR      | DIR_METADATA     | closedir |
| DIRFD         | DIR_METADATA     | dirfd |
| OPENDIR       | DIR_METADATA     | fdopendir, opendir |
| READDIR       | DIR_METADATA     | readdir, readdir_r |
| REWINDDIR     | DIR_METADATA     | rewinddir |
| SCANDIR       | DIR_METADATA     | NOT-IMPLEMENTED |
| SEEKDIR       | DIR_METADATA     | seekdir |
| TELLDIR       | DIR_METADATA     | telldir |
| CHDIR         | DIRS             | chdir, fchdir |
| MKDIR         | DIRS             | mkdir, mkdirat |
| RMDIR         | DIRS             | rmdir |
| DUP           | FILE_DESCRIPTORS | dup, dup2, dup3 |
| ACCESS        | FILE_METADATA    | access, faccessat |
| CHMOD         | FILE_METADATA    | chmod, fchmod, fchmodat |
| CHOWN         | FILE_METADATA    | chown, fchown, fchownat, lchown |
| STAT          | FILE_METADATA    | fstat, lstat, stat |
| UTIME         | FILE_METADATA    | utime |
| CLOSE         | FILE_OPEN_CLOSE  | close, fclose |
| OPEN          | FILE_OPEN_CLOSE  | open, open64, creat, creat64, fopen, fopen64 |
| READ          | FILE_READ        | read, pread, readv, preadv, fread, fscanf, vfscanf |
| ALLOCATE      | FILE_SPACE       | posix_fallocate, fallocate |
| TRUNCATE      | FILE_SPACE       | truncate, ftruncate |
| MOUNT         | FILE_SYSTEMS     | mount |
| UMOUNT        | FILE_SYSTEMS     | umount, umount2 |
| WRITE         | FILE_WRITE       | write, pwrite, writev, pwritev, fprintf, vfprintf, fwrite |
| LINK          | LINKS            | link |
| READLINK      | LINKS            | readlink |
| UNLINK        | LINKS            | unlink |
| CHROOT        | MISC             | chroot |
| FLOCK         | MISC             | flock |
| MKNOD         | MISC             | mknod |
| RENAME        | MISC             | rename |
| EXEC          | PROCESSES        | exec* (all 6 variants) |
| FORK          | PROCESSES        | fork |
| KILL          | PROCESSES        | kill |
| SEEK          | SEEKS            | NOT-IMPLEMENTED |
| SOCKET        | SOCKETS          | socket |
| START         | START_STOP       | startup of a process (no corresponding function call) |
| STOP          | START_STOP       | end of a process (no corresponding function call) |
| FLUSH         | SYNCS            | fflush |
| SYNC          | SYNCS            | fsync, fdatasync, sync, syncfs |
| GETXATTR      | XATTRS           | getxattr, lgetxattr, fgetxattr |
| LISTXATTR     | XATTRS           | listxattr, llistxattr, flistxattr |
| REMOVEXATTR   | XATTRS           | removexattr, fremovexattr, lremovexattr |
| SETXATTR      | XATTRS           | setxattr, lsetxattr, fsetxattr |
| START_STOP    | START            | not a function; event is triggered if process is started |
| START_STOP    | STOP             | not a function; event is triggered if process is stopped. Keep in mind, that this event is not intercepted when exec call is successful |
| HTTP          | HTTP_REQ_SEND    | sent an HTTP request |
| HTTP          | HTTP_REQ_RECV    | received an HTTP request |
| HTTP          | HTTP_RESP_SEND   | sent an HTTP response |
| HTTP          | HTTP_RESP_RECV   | received an HTTP response |




## Domains

| Domain           | Description                      | Operations |
| ------           | -----------                      | ---------- |
| DIR_METADATA     | directory metadata operations    | CLOSEDIR, DIRFD, OPENDIR, READDIR, REWINDDIR, SCANDIR, SEEKDIR, TELLDIR |
| DIRS             | directory operations             | CHDIR, MKDIR, RMDIR |
| FILE_DESCRIPTORS | file descriptor manipulations    | DUP, FCNTL |
| FILE_METADATA    | file metadata operations         | ACCESS, CHMOD, CHOWN, STAT, UTIME |
| FILE_WRITE       | file write operations            | WRITE |
| FILE_READ        | file read operations             | READ |
| FILE_OPEN_CLOSE  | file open/close operations       | CLOSE, OPEN |
| FILE_SYSTEMS     | file system operations           | MOUNT, UMOUNT |
| FILE_SPACE       | file space adjustment operations | ALLOCATE, TRUNCATE |
| HTTP             | HTTP network operations          | TBD: http verb events |
| LINKS            | hard and soft link operations    | LINK, READLINK, UNLINK |
| MISC             | misc. operations                 | CHROOT, FLOCK, MKNOD, RENAME |
| PROCESSES        | process operations               | EXEC, FORK, KILL |
| SEEKS            | file seek operations             | SEEK |
| SOCKETS          | socket operations                | SOCKET, BIND, CONNECT |
| START_STOP       | begin and end of processes       | START, STOP |
| SYNCS            | file sync/flush operations       | FLUSH, SYNC |
| XATTRS           | extended attribute operations    | GETXATTR, LISTXATTR, REMOVEXATTR, SETXATTR |

## Environment Variables

| Variable              | Required? | Description |
| ------                | --------- | ----------- |
| FACILITY_ID           | N         | Identifies the component. defaults to 'u' |
| MESSAGE_QUEUE_PATH    | Y         | File path of existing file associated with SysV message queue |
| MONITOR_DOMAINS       | Y         | list of comma-separated domains to monitor or 'ALL' |
| START_ON_OPEN         | N         | starts paused, resumes on open of specified file |
| START_ON_ELAPSED      | N         | starts paused, resumes on elapsed time crossing specified threshold |
| TIME_SAMPLE_FREQUENCY | N         | specifies frequency for time-based sampling |
| TIME_SAMPLE_DURATION  | N         | specifies duration for time-based sampling |

## System requirements

This program should work on any Linux distribution. For build, in addition to standard
toolchain, you will need indent utility.

## START_ON_OPEN

START_ON_OPEN causes io monitor to start in paused mode. It will remain paused until a
file is opened whose path contains (substring) the value specified by this variable. 

In some cases, there may be a great deal of unwanted captures that occur when process
is first started. An example would be running a Python program. When the Python
interpreter starts it reads many files as part of its initialization.

The START_ON_OPEN feature can prevent such undesirable startup noise. If you set a
value for the environment variable 'START_ON_OPEN', the monitor will start in 'paused'
mode. It will remain paused until a file open operation occurs with the file path
containing the specified value. 

For example, you might set START_ON_OPEN as:

    export START_ON_OPEN="hello_world.txt"

for a Python program that begins by opening the file "hello_world.txt". This technique
would prevent the normal Python initialization traffic from being captured by the monitor.

## Metrics

| Metric            | Description |
| ------            | ----------- |
| facility          | identifier of component that generated the metrics |
| ts                | unix timestamp of when operation occurred |
| duration          | elapsed time of operation in milliseconds |
| pid               | process id where metrics were collected |
| domain            | domain grouping for the operation |
| op-type           | type of operation |
| error code        | integer error code. 0 = success; non-zero = errno in most cases |
| fd                | file descriptor associated with operation, or -1 if N/A |
| bytes transferred | number of bytes transferred for read/write operations |
| arg1              | context dependent |
| arg2              | context dependent |

## Running Listener

As io_monitor is a library collecting datapoints, default way to collect and display these datapoints is utility called mq_listener. To run mq_listener it is required to give it path to message queue file. It is also advisable to load at least one output plugin, as otherwise mq_listener won't tell you about events, it collects.

Assuming, you have your mq in root directory of this project, you can run mq_listener following way:

    ./mq_listener/mq_listener -m mq1 -p plugins/output_table.so

Alternatively you can use config file for following invocation:

    ./mq_listener/mq_listener -c mq_listener/listener.conf.example

There is possibility to load multiple plugins at once for following reasons:
- use plugins as UI
- use plugins as data filters (drop some events)
- use multiple forms of display or multiple forms of logging at once.

Consider following example:

    ./mq_listener/mq_listener -m mq1 -p plugins/filter_domain.so HTTP -p plugins/output_table.so

In this case only HTTP related events will be displayed even if MONITOR_DOMAINS variable is set to ALL. This is convenient way to change subset of monitored functions without restarting monitored application. Keep in mind that correct order of plugins is important.
