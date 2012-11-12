#!/bin/sh
# ${1} stands for test name.
# ${2} stands for path to test directory.
# ${3} stands for path to qimessaging build dir.
#
# This run only on unix because :
# - Call to mono not needed on windows (native binary)
# - LD_PRELOAD load libqimessaging.so and not qimessaging.dll

# Call .NET binary with mono JIT compiler
LD_PRELOAD=${3}/lib/libqimessaging.so mono ${2}/${1}/bin/Debug/${1}.exe
