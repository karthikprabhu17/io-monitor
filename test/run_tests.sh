#!/bin/bash

TEST_LOG_DIR=test_log_`date +%s`
mkdir $TEST_LOG_DIR

FAILURE=""

for dir in `ls` ; do
    if [ -x $dir/test.sh ] ; then
	echo Executing test in $dir | tee -a $TEST_LOG_DIR/out
	cd $dir
	./test.sh 2>&1 > ../$TEST_LOG_DIR/out_$dir
	if [ $? -eq 0 ] ; then
	    echo Test successful | tee -a ../$TEST_LOG_DIR/out
	else
	    echo Test failed  | tee -a ../$TEST_LOG_DIR/out
	    FAILURE="One or more tests failed"
	fi
	cd ..
    fi
done

if [ -z "$FAILURE" ] ; then
    echo All tests successful
    exit 0
else
    echo $FAILURE
    exit 1
fi
