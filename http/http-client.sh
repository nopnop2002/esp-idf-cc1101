#!/bin/bash
SERVER="http://esp32-server.local:8080/post"

while :
do
	payload=`date "+%Y/%m/%d %H:%M:%S"`
	echo ${payload}
	#curl -d "2023/08/03 22:14:35" ${SERVER}
	curl -d "${payload}" ${SERVER}
	echo ""
	sleep 1
done
