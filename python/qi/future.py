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

FutureTimeout_Infinite = int(0x7ffffff)
FutureTimeout_None     = 0

class Promise:
    """ Promise
    """

    def __init__(self, cancel_callback=None):
        if cancel_callback is None:
            self._prom = _qipy.qi_promise_create()
        else:
            self._prom = _qipy.qipy_promise_cancelable_create(self, cancel_callback)

    def future(self):
        return Future(_qipy.qi_promise_get_future(self._prom))

    def set_error(self, err):
        _qipy.qi_promise_set_error(self._prom, str(err))

    def set_value(self, val):
        _qipy.qipy_promise_set_value(self._prom, val)

    def set_canceled(self):
        _qipy.qi_promise_set_canceled(self._prom)

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
        self._fut = future

    def wait(self, timeout = FutureTimeout_Infinite):
        """ Wait until future value or error is set.
        """
        _qipy.qi_future_wait(self._fut)

    def cancel(self):
        """ Try to cancel the future operation """
        _qipy.qi_future_cancel(self._fut)

    def has_error(self, timeout = FutureTimeout_Infinite):
        """ Check whether future encountered an error.
        """
        return _qipy.qi_future_has_error(self._fut, timeout)

    def has_value(self, timeout = FutureTimeout_Infinite):
        """ Check whether future encountered an error.
        """
        return _qipy.qi_future_has_value(self._fut, timeout)

    def is_finished(self):
        """ Check whether future value is set.
        """
        return _qipy.qi_future_is_finished(self._fut)

    def is_running(self):
        """ Check whether future value is set.
        """
        return _qipy.qi_future_is_running(self._fut)

    def is_canceled(self):
        """ Check whether future value is set.
        """
        return _qipy.qi_future_is_canceled(self._fut)

    def error(self, timeout = FutureTimeout_Infinite):
        """ Getter on future error.
        """
        #todo: may leak
        return _qipy.qi_future_get_error(self._fut)

    def value(self, timeout = FutureTimeout_Infinite):
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
        _qipy.qi_future_destroy(self._fut)
