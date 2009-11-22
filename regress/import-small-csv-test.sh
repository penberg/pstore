#!/bin/sh

PSTORE=small.pstore
EXPECTED=small.output
ACTUAL=tmp

../pstore import small.csv $PSTORE
../pstore cat $PSTORE > $ACTUAL

diff -u $EXPECTED $ACTUAL
result=$?

rm $PSTORE
rm $ACTUAL

if test $result != 0
then
	echo "`basename $0`: FAILED"
	exit 1
fi
