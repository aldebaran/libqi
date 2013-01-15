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
import ctypes

def load_libqimessaging():
    """ Load qimessaging.so and its dependencies.

    This makes qimessaging usable from a relocatable
    SDK without having to set LD_LIBRARY_PATH
    """
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
        load_libqimessaging()
    except Exception, e:
        print e
