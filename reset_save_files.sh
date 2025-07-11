#!/bin/bash

set -e

if [ "$1" == "" ]; then
    rm settings.txt
fi
if [ ! -d timetables ]; then
    mkdir timetables
fi
if [ ! -d templates ]; then
    mkdir templates
fi

