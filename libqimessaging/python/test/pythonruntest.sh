#!/bin/sh

# Set path to libnaoqi build dir and call python script
PYTHONPATH="${1}/lib:${2}" python2 ${3}
