#!/usr/bin/env bash

while IFS= read archiveLine
do
  projectName=$(echo "$archiveLine" | cut -f 1 -d ",")
  projectVersion=$(echo "$archiveLine" | cut -f 2 -d ",")
  archiveLocation=$(echo "$archiveLine" | cut -f 3 -d ",")
	echo PROCESSING "$archiveLine" VERSION "$projectVersion"
	wget "$archiveLocation"
done < archives_to_process

