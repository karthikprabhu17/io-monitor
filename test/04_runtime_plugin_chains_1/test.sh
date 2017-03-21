#!/bin/bash


echo Running test event 1

#prepare test
rm -f file input.txt output.txt
rm -f file mq1 listener_output.csv events_captured.csv
touch mq1

cat > input.txt <<EOF
load-plugin ../../plugins/filter_domains.so:df1 HTTP
load-plugin ../../plugins/filter_domains.so:df2 START_STOP
list-plugins
df2 print-mask
df1 print-mask
df1 update-mask FILE_OPEN_CLOSE,FILE_WRITE
df2 print-mask
df1 print-mask
quit
EOF

cat >pattern.txt <<EOF
START_STOP
HTTP
START_STOP
FILE_WRITE,FILE_OPEN_CLOSE
EOF
../../mq_listener/mq_listener -m mq1 -p ../../plugins/input_cli.so < input.txt  | grep domain_mask\: | cut -d ':' -f 2 > output.txt

diff pattern.txt output.txt

if [ 0 -ne $? ] ; then
    echo Test failed: result of plugin reconfiguration not preserved correctly
    exit 1
fi

echo "Test event passed"

exit 0
