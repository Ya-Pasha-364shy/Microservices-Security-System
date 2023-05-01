#!/bin/bash

counter=0

while IFS= read -r line; do
	INR=`echo $line | grep "service" | awk '{print $9}'`
	if [ "$INR" == "backup_service" ] || [ "$INR" == "service_manager" ]; then
		counter=$(($counter + 1));
	fi
done<./test.txt
rm -f test.txt

if [ "$counter" -eq 2 ]; then
	echo "OK"
else
	echo "Error !"
fi