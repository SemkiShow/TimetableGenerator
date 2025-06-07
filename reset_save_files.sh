#!/bin/bash

set -e

if [ ! -f settings.txt ] || [ "$1" == "" ]; then
    printf "vsync=true\nlast-timetable=\nmerged-font=false\ntimetable-autosave-interval=60\n" > settings.txt
fi
if [ "$1" == "" ]; then
    rm -r timetables/
fi
if [ "$1" == "" ]; then
    rm -r templates/
fi
if [ ! -d timetables ]; then
    mkdir timetables
fi
if [ ! -d templates ]; then
    mkdir templates
fi

