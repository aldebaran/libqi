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


def load_lib_qipyessaging():
    """ Load _qipyessaging.so and its dependencies.

    This makes _qipyessaging usable from a relocatable
    SDK without having to set LD_LIBRARY_PATH
    """
    import ctypes
    import os
    import sys
    deps = [
            "libboost_python.so",
            "libboost_system.so",
            "libboost_chrono.so",
            "libboost_program_options.so",
            "libboost_thread.so",
            "libboost_filesystem.so",
            "libboost_regex.so",
            "libboost_locale.so",
            "libboost_signals.so",
            "libqi.so",
            "libqitype.so",
            "libqimessaging.so",
            "libqipython.so",
    ]
    if sys.version_info[0] == 2:
        deps.append("libqipython.so")
    else:
        deps.append("libqipython3.so")
    this_dir = os.path.abspath(os.path.dirname(__file__))
    for dep in deps:
        full_path = os.path.join(this_dir, "..", dep)
        try:
            ctypes.cdll.LoadLibrary(full_path)
        except Exception:
            pass


def _on_import_module():
    import sys
    if sys.platform.startswith("linux"):
        load_lib_qipyessaging()


#######
_on_import_module()

from _qi import Application as _Application
from _qi import ApplicationSession as _ApplicationSession
from _qi import ( FutureState, FutureTimeout, Future, Promise,
                  Property, Session, Signal,
                  createObject, registerObjectFactory,
                  async, PeriodicTask)
from . import path
from ._type import ( Void, Bool,
                     Int8, UInt8,
                     Int16, UInt16,
                     Int32, UInt32,
                     Int64, UInt64,
                     Float, Double,
                     String, List,
                     Map, Struct,
                     Object, Dynamic,
                     Buffer, AnyArguments,
                     typeof, _isinstance)
from ._binder import bind, nobind
from .logging import fatal, error, warning, info, verbose, Logger
from .logging import getLogger, logFatal, logError, logWarning, logInfo, logVerbose, logDebug  #deprecated
from .translator import defaultTranslator, tr, Translator


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

#application is a singleton, it should live till the end of the program
#because it own eventloops
def Application(args=None, raw=False, autoExit=True, url=None):
    global _app
    if args is None:
        args = [ "" ]
    if url is None:
        url = "tcp://127.0.0.1:9559"
    if _app is None:
        if raw:
            _app = _Application(args)
        else:
            _app = _ApplicationSession(args, autoExit, url);
    else:
        raise Exception("Application was already initialized")
    return _app

ApplicationSession = Application

__all__ = ["FutureState",
           "FutureTimeout",
           "Future",
           "Promise",
           "Property",
           "Session",
           "Signal",
           "createObject",
           "registerObjectFactory",
           "async",
           "Void", "Bool", "Int8", "UInt8", "Int16", "UInt16", "Int32", "UInt32", "Int64", "UInt64",
           "Float", "Double", "String", "List", "Map", "Struct", "Object", "Dynamic", "Buffer", "AnyArguments",
           "typeof", "isinstance",
           "bind", "nobind",
           "fatal", "error", "warning", "info", "verbose",
           "getLogger", "logFatal", "logError", "logWarning", "logInfo", "logVerbose", "logDebug",  #deprecated
           "Logger", "defaultTranslator", "tr", "Translator"
]

import atexit
atexit.register(_stopApplication)
# Do not pollute namespace
del atexit
