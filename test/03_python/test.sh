#!/bin/bash

echo Running test event 1

#prepare test
rm -f file sample.csv 
rm -f file mq1 listener_output.csv events_captured.csv
touch mq1


cat > sample.csv <<EOF
FILE_OPEN_CLOSE,OPEN
FILE_WRITE,WRITE
FILE_OPEN_CLOSE,CLOSE
START_STOP,STOP
EOF

#run listener for test
(../../mq_listener/mq_listener -m mq1 -p ../../plugins/output_csv.so > listener_output.csv ) &

#run test program
START_ON_OPEN=file LD_PRELOAD=`pwd`/../../io_monitor/io_monitor.so MESSAGE_QUEUE_PATH=`pwd`/mq1 MONITOR_DOMAINS=ALL ./main.py

#kill listener
sleep 1
kill -9 `pgrep mq_listener` 
    
#verify side effects of functions
cat listener_output.csv | grep 'u,' | cut -d , -f 5,6 > events_captured.csv

diff events_captured.csv sample.csv
if [ 0 -ne $? ] ; then
    echo Test failed: not all expected event were successfully captured.
    exit 1
fi


CONTENT=`cat file`
if [ "data" != $CONTENT ] ; then
    echo Test failed: File content not preserved.
    exit 1
fi

echo "Test event passed"

exit 0
