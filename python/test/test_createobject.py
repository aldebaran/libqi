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

    if len(sys.argv) < 4:
        sys.exit('Usage: %s --sdk-dir path/to/sdk/ LIB_SUFFIX' % (sys.argv[0]))

    # I think this can be avoided if we get the library fullpath by argument,
    # but we need cmake to pass it to us. The best cmake can do is:
    # get_property(liblocation TARGET registerobject PROPERTY LOCATION)
    # but this does not include the _d suffix on windows (tested on cmake 2.8.10.2)
    if sys.platform.startswith("linux"):
        full_path = "%s/lib/libtestregisterobject%s.so" % (sys.argv[2], pyver)


    if sys.platform.startswith("darwin"):
       full_path = "%s/lib/libtestregisterobject%s.dylib" % (sys.argv[2], pyver)


    if sys.platform.startswith("win"):
       full_path = "%s/bin/testregisterobject%s.dll" % (sys.argv[2], pyver)
       if not os.path.isfile(full_path):
          full_path = "%s/bin/testregisterobject%s_d.dll" % (sys.argv[2], pyver)
       if not os.path.isfile(full_path):
          print "registerobject library not found!"
	  assert False

    try:
       print "loading ", full_path
       ctypes.cdll.LoadLibrary(full_path)
    except Exception as e:
       import traceback
       traceback.print_exc()
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
