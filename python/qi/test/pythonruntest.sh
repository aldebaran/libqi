#!/bin/sh

# Set path to libnaoqi build dir and call python script
echo "Running test with PYTHONPATH=${1}/lib:${2}"
PYTHONPATH="${1}/lib:${2}" python2 ${3}
