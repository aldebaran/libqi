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

import _qi

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

    def wait(self):
        """ Wait untill future value or error is set.
        """
        _qi.qi_future_wait(self._fut)

    def is_error(self):
        """ Check whether future encountered an error.
        """
        return _qi.qi_future_is_error(self._fut)

    def is_ready(self):
        """ Check whether future value is set.
        """
        return _qi.qi_future_is_ready(self._fut)

    def get_error(self):
        """ Getter on future error.
        """
        return _qi.qi_future_get_error(self._fut)

    def __del__(self):
        """ Destructor of Future class

        Call C instance destructor
        """
        _qi.qi_future_destroy(self._fut)
