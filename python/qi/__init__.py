##
## Author(s):
##  - Cedric GESTES <gestes@aldebaran-robotics.com>
##  - Pierre ROULLON <proullon@aldebaran-robotics.com>
##
## Copyright (C) 2010, 2011 Aldebaran Robotics
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
            "lib_qipyessaging.so",
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


import _qipy
from qi.genericobject import GenericObject
from qi.application   import Application
from qi._session      import Session
from qi.future        import Future, Promise

__all__ = [ "Application", "Session", "Promise", "Future", "CallError", "SerializationError", "ConnectionError", "SessionNotConnectedError", "RegisterError", ]

class CallError(Exception):
    """ CallError exception raised by Object.call() method
    """
    def __init__(self, value):
        """ Constructor of CallError exception
        """
        self._value = value

    def __str__(self):
        """ Getter on error value, Python style.
        """
        return str(self._value)

class SerializationError(Exception):
    def __init__(self, value):
        self._value = value

    def __str__(self):
        return str(self._value)

class ConnectionError(Exception):
    """Raised by Session constructor and Session.connect."""

    def __init__(self, value):
        """ ConnectionError constructor

        :param value: Error message.
        """
        self._value = value

    def __str__(self):
        """Error message getter, Python style."""
        return str(self._value)

class SessionNotConnectedError(Exception):
    def __str__(self):
        return 'Session is not connected: can\'t fetch services list.'

class RegisterError(Exception):
    """Raised by Session when it can't register a service."""

    def __init__(self, value):
        self._value = value

    def __str__(self):
        return str(self._value)
