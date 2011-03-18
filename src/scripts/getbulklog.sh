#!/bin/bash

dir=/bulk/temp/yaca-log
outfile=bulk.log

echo Changing to $dir

cd $dir || exit 1
[ "$1" != "-q" ] && echo Syncing logfiles...
[ "$1" != "-q" ] && rsync -r root@nas:/var/log/yaca .

echo Searching for largest logfile number...

for i in {1..10000}; do
	ls yaca/bulk.log.$i.gz 2>/dev/null 1>&2
	if [ $? -ne 0 ]; then
		break
	fi
done

i=`expr $i - 1`
numfiles=$i

echo Found $numfiles logfiles, concatenating...

rm -f $outfile
for i in {$numfiles..1}; do
	gzip -c -d yaca/bulk.log.$i.gz >> $outfile
done
cat yaca/bulk.log >> $outfile

echo Done.
