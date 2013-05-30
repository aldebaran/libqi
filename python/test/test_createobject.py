#!/usr/bin/env python
##
## Author(s):
##  - Cedric GESTES <gestes@aldebaran-robotics.com>
##
## Copyright (C) 2013 Aldebaran Robotics

import qi
import os
import sys
import ctypes

def main():

    if len(sys.argv) < 3:
        sys.exit('Usage: %s --sdk-dir path/to/sdk/' % sys.argv[0])

    if sys.platform.startswith("linux"):
        full_path = "%s/lib/libtestregisterobject.so"% sys.argv[2]


    if sys.platform.startswith("darwin"):
       full_path = "%s/lib/libtestregisterobject.dylib"% sys.argv[2]


    if sys.platform.startswith("win"):
       full_path = "%s/lib/libtestregisterobject.dll"% sys.argv[2]

    try:
       ctypes.cdll.LoadLibrary(full_path)
    except Exception as e:
       print e
       sys.exit(1)

    try:
       testObject = qi.createObject("NoObject")
       assert False
    except:
       assert True

    testObject = qi.createObject("TestObject")

    testObject.call("setv", 42)
    fut = testObject.call("getv")
    assert (fut == 42)

if __name__ == "__main__":
    main()
