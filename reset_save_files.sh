#!/bin/bash

set -e

if [ ! -f settings.txt ] || [ "$1" == "" ]; then
    printf "vsync=true\n" > settings.txt
fi
if [ ! -d timetables ] && [ "$1" == "" ]; then
    mkdir timetables
    mkdir timetables/templates
fi

