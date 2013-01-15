##
## Author(s):
##  - Pierre Roullon <proullon@aldebaran-robotics.com>
##
## Copyright (C) 2010, 2011, 2012 Aldebaran Robotics
##

""" Ease startup and teardown of an application.

.. module:: qimessaging

"""

import sys
import _qimessagingswig

class Application:
    """ Python wrapper around qi::Application
    """
    def __init__(self, args=None):
        """ Application constructor, initialize application

        .. note::
           Application has to be created before anything else.

        .. args::
           args : Command line arguments.
        """
        if not args:
            args = sys.argv
        self._app = _qimessagingswig.py_application_create(args)

    def run(self):
        """ Waits until on of those condition become true:
             - stop() is called.
             - TERM or QUIT signal is received.
             - The Application instance is destroyed, which means main() is exiting.
        """
        _qimessagingswig.qi_application_run(self._app)

    def stop(self):
        """ Tear down application

        .. note::
            Calling this function will make run() return.
        """
        _qimessagingswig.qi_application_stop(self._app)

    def __del__(self):
        """ Do nothing.
        Application must be released after everything else anyway.
        """

    @staticmethod
    def initialized():
        return _qimessagingswig.qi_application_initialized()


class MissingApplicationError(Exception):
    def __init__(self, feature=None):
        self._feature = feature

    def __str__(self):
        msg = []
        if self._feature is not None:
            msg.append('%s feature needs an Application to work.' % self._feature)
        msg.append('You must create an Application.')
        return ' '.join(msg)
