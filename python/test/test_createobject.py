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

    pyver = ""
    if sys.version_info[0] >= 3:
        pyver = "3"

    if len(sys.argv) < 3:
        sys.exit('Usage: %s --sdk-dir path/to/sdk/' % (sys.argv[0]))

    if sys.platform.startswith("linux"):
        full_path = "%s/lib/libtestregisterobject%s.so"% (sys.argv[2], pyver)


    if sys.platform.startswith("darwin"):
       full_path = "%s/lib/libtestregisterobject%s.dylib"% (sys.argv[2], pyver)


    if sys.platform.startswith("win"):
       full_path = "%s/lib/libtestregisterobject%s.dll"% (sys.argv[2], pyver)

    try:
       ctypes.cdll.LoadLibrary(full_path)
    except Exception as e:
       print(e)
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
