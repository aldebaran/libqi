##
## Author(s):
##  - Pierre Roullon <proullon@aldebaran-robotics.com>
##
## Copyright (C) 2010, 2011, 2012 Aldebaran Robotics
##

""" Python wrapper around QiMessaging future.

Allow easy asynchronous calls handling and calls synchronisation.

.. note:: This module is unused
"""

import _qipy

class Promise:
    """ Promise
    """

    def __init__(self):
        self._prom = _qipy.qi_promise_create()

    def future(self):
        return Future(_qipy.qi_promise_get_future(self._prom))

    def set_error(self, err):
        _qipy.qi_promise_set_error(self._prom, str(err))

    def set_value(self, val):
        print "prom setval"
        _qipy.qipy_promise_set_value(self._prom, val)

    def __del__(self):
        """ Destructor of Future class

        Call C instance destructor
        """
        _qipy.qi_promise_destroy(self._prom)

class Future:
    """ Future can be used to keep track of a long asynchronous call
    """
    def __init__(self, future):
        """ Future constructor

        .. note::
           Mandatory : Constructor need a valid C future.

        .. args::
           future : Valid C future wrapper.
        """
        print "calling Future.__init__"
        self._fut = future

    def wait(self):
        """ Wait untill future value or error is set.
        """
        _qipy.qi_future_wait(self._fut)

    def has_error(self):
        """ Check whether future encountered an error.
        """
        return _qipy.qi_future_has_error(self._fut)

    def is_ready(self):
        """ Check whether future value is set.
        """
        return _qipy.qi_future_is_finished(self._fut)

    def error(self):
        """ Getter on future error.
        """
        #todo: may leak
        return _qipy.qi_future_get_error(self._fut)

    def value(self):
        """ Getter on future error.
        """
        return _qipy.qipy_future_get_value(self._fut)

    def add_callback(self, func):
        """ Add a callback to the future
        """
        return _qipy.qipy_future_add_callback(self._fut, self, func)

    def __del__(self):
        """ Destructor of Future class

        Call C instance destructor
        """
        print "calling Future.__del__"
        #import pdb
        #pdb.set_trace()
        _qipy.qi_future_destroy(self._fut)
