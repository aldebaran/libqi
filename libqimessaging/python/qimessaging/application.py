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
import _qi

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
        self._app = _qi.py_application_create(args)

    def run(self):
        """ Waits until on of those condition become true:
             - stop() is called.
             - TERM or QUIT signal is received.
             - The Application instance is destroyed, which means main() is exiting.
        """
        _qi.qi_application_run(self._app)

    def stop(self):
        """ Tear down application

        .. note::
            Calling this function will make run() return.
        """
        _qi.qi_application_stop(self._app)

    def __del__(self):
        """ Release allocated ressources
        """
        _qi.qi_application_destroy(self._app)
