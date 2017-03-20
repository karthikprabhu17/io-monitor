#!/bin/bash

#prepare test
rm -f a.out file sample.csv 
gcc main.c

#parameters:
# $1: number of test event
# $2: monitor_domains
# $3: sample_output (path to file)
# $4: plugins applied to mq listener (string)
function run_case
{
    echo "running test event $1 (with parameters $2 $3 $4)"

    #prepare test
    rm -f file mq1 listener_output.csv events_captured.csv
    touch mq1


    #run listener for test
    (../../mq_listener/mq_listener -m mq1 $4 > listener_output.csv ) &

    #run test program
    LD_PRELOAD=`pwd`/../../io_monitor/io_monitor.so MESSAGE_QUEUE_PATH=`pwd`/mq1 MONITOR_DOMAINS=$2 ./a.out

    #kill listener
    sleep 1
    kill -9 `pgrep mq_listener` 
    
    #verify side effects of functions
    cat listener_output.csv | grep 'u,' | cut -d , -f 5,6 > events_captured.csv

    diff events_captured.csv $3
    if [ 0 -ne $? ] ; then
	echo Test failed: not all expected event were successfully captured.
	exit 1
    fi

    RWX=`ls -l file | cut -d ' ' -f 1`

    if [ "-rwx------" != $RWX ] ; then
	echo Test failed: File permissions not preserved.
	exit 1
    fi

    CONTENT=`cat file`
    if [ "data" != $CONTENT ] ; then
	echo Test failed: File content not preserved.
	exit 1
    fi

    echo "Test event passed"
}

cat > sample.csv <<EOF
START_STOP,START
DIRS,CHDIR
FILE_OPEN_CLOSE,OPEN
FILE_WRITE,WRITE
FILE_OPEN_CLOSE,CLOSE
START_STOP,STOP
EOF

run_case 1 ALL sample.csv "-p ../../plugins/output_csv.so"

run_case 2 ALL sample.csv "-p ../../plugins/output_csv.so -p ../../plugins/filter_domains.so FILE_OPEN_CLOSE,FILE_READ"

cat > sample.csv <<EOF
FILE_OPEN_CLOSE,OPEN
FILE_WRITE,WRITE
FILE_OPEN_CLOSE,CLOSE
EOF

run_case 3 ALL sample.csv " -p ../../plugins/filter_domains.so FILE_OPEN_CLOSE,FILE_READ,FILE_WRITE -p ../../plugins/output_csv.so"

run_case 3 FILE_OPEN_CLOSE,FILE_READ,FILE_WRITE sample.csv " -p ../../plugins/output_csv.so"

exit 0
