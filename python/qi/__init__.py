##
## Author(s):
##  - Cedric GESTES <gestes@aldebaran-robotics.com>
##  - Pierre ROULLON <proullon@aldebaran-robotics.com>
##
## Copyright (C) 2010 - 2013 Aldebaran Robotics
##

""" QiMessaging Python bindings

Provided features are very close to C++, Python style.
"""

import os
import sys

def load_lib_qipyessaging():
    """ Load _qipyessaging.so and its dependencies.

    This makes _qipyessaging usable from a relocatable
    SDK without having to set LD_LIBRARY_PATH
    """
    import ctypes
    deps = [
            "libqi.so",
            "libqitype.so",
            "libqimessaging.so",
    ]
    this_dir = os.path.abspath(os.path.dirname(__file__))
    for dep in deps:
        full_path = os.path.join(this_dir, "..", dep)
        ctypes.cdll.LoadLibrary(full_path)


if sys.platform.startswith("linux"):
    try:
        load_lib_qipyessaging()
    except Exception:
        pass


#######

from _qi import Application as _Application
from _qi import FutureState, FutureTimeout, Future, \
                Promise, Property, Session, Signal, \
                createObject, registerObjectFactory

from _type import Void, Bool, Int8, UInt8, Int16, UInt16, Int32, UInt32, Int64, UInt64, Float, Double, String, List, Map, Struct, Object, Dynamic, Buffer, AnyArguments
from _type import typeof, _isinstance
from _binder import bind, nobind

#rename isinstance here. (isinstance should not be used in this file)
isinstance = _isinstance

_app = None


#we want to stop all thread before python start destroying
#module and the like. (this avoid callback calling python while
#it's destroying)
def _stopApplication():
    global _app
    if _app is not None:
        _app.stop()
        del _app
        _app = None

import atexit
atexit.register(_stopApplication)

#application is a singleton, it should live till the end of the program
#because it own eventloops
def Application():
    global _app
    if _app is None:
        _app = _Application()
    return _app

__all__ = ["FutureState",
           "FutureTimeout",
           "Future",
           "Promise",
           "Property",
           "Session",
           "Signal",
           "createObject",
           "registerObjectFactory",

           "Void", "Bool", "Int8", "UInt8", "Int16", "UInt16", "Int32", "UInt32", "Int64", "UInt64",
           "Float", "Double", "String", "List", "Map", "Struct", "Object", "Dynamic", "Buffer", "AnyArguments",
           "typeof", "isinstance"
           "bind", "nobind"

]
