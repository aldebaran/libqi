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

from _qi import Application, Future, FutureState, FutureTimeout, Promise, Session, Signal, Property

def _shrinker(self, *args, **kwargs):
    self.trigger(args, kwargs)

Signal.__call__ = _shrinker

__all__ = [ "Application", "Session", "Promise", "Future", "FutureState", "FutureTimeout", "Signal", "Property" ]
