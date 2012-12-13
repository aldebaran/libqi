##
## Author(s):
##  - Pierre Roullon <proullon@aldebaran-robotics.com>
##
## Copyright (C) 2010, 2011, 2012 Aldebaran Robotics
##

""" QiMessaging object class
"""

import _qimessagingswig

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


class GenericObject:
    """ Main class of QiMessaging
    """
    def __init__(self, qi_object = None):
        """ Object constructor.

        If qi_object is not set, create empty object.
        """
        if qi_object is None:
            self._obj = _qimessagingswig.qi_object_create("obj")
        else:
            self._obj = qi_object

    def _call(self, name, *args):
        """ Call remote method with given arguments.

        .. Args::
            name : Function name.
            args : Tuple containing giver arguments.
        """
        return _qimessagingswig.qi_generic_call(self._obj, name, args)

    def __del__(self):
        """ Object destructor, also destroy C++ object.
        """
        if self._obj:
            _qimessagingswig.qi_object_destroy(self._obj)
