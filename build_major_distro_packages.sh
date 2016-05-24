#!/usr/bin/env bash

majorDistros="debian gentoo slackware redhat arch"

for eachDistro in $majorDistros
do
  if [ "$eachDistro" == "gentoo" -o "$eachDistro" == "slackware" -o "$eachDistro" == "redhat" -o "$eachDistro" == "arch" ]
  then
    echo "$eachDistro" NOT CURRENTLY SUPPORTED
  elif [ "$eachDistro" == "debian" ]
  then
    echo "$eachDistro" PACKAGING IS CURRENTLY EXPERIMENTAL
  else
    echo "$eachDistro" DISTRO IS NOT KNOWN BY THIS SCRIPT
  fi
done

