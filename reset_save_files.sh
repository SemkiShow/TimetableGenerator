#!/bin/bash

set -e

if [ "$1" == "" ]; then
    rm settings.txt
fi
if [ ! -f version.txt ]; then
    echo "" > version.txt
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

