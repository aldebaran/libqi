##
## Author(s):
##  - Pierre Roullon <proullon@aldebaran-robotics.com>
##
## Copyright (C) 2010, 2011, 2012 Aldebaran Robotics
##

""" QiMessaging session system

Sessions are basis of QiMessaging application.
There is two different manners to use them :

- Service style :

 With objects bound on it, listening for call of other applications.

- Client style :

Only connected to service directory, get services from other applications.
"""

import _qimessagingswig
import qimessaging.binder as binder
from qimessaging.genericobject import GenericObject

class ConnectionError(Exception):
    """ Raised by Session constructor and Session.connect
    """
    def __init__(self, value):
        """ ConnectionError constructor
        Args:
        value : Error message.
        """
        self._value = value

    def __str__(self):
        """ Error message getter, Python style.
        """
        return str(self._value)


class RegisterError(Exception):
    """Raised by Session when it can't register a service."""

    def __init__(self, value):
        self._value = value

    def __str__(self):
        return str(self._value)


class Session:
    """ Package all function needed to create and connect to QiMessage services.
    """
    def __init__(self, address = None):
        """ Session constructor, if address is set, try to connect.
        """
        self._session = _qimessagingswig.qi_session_create()
        if address:
            self.connect(address)

    def connect(self, address):
        """ Connect to service directory.

        .. Raises::
            ConnectionError exception.
        """
        if not _qimessagingswig.qi_session_connect(self._session, address):
            raise ConnectionError('Cannot connect to ' + address)

    def listen(self, address):
        """ Listen on given address.

        Uppon connection, return service asked.
        """
        if _qimessagingswig.qi_session_listen(self._session, address):
            return True
        return False

    def register_object(self, name, obj):
        """ Register given Python class instance
        """
        functionsList = binder.buildFunctionListFromObject(obj)
        return _qimessagingswig.py_session_register_object(self._session, name, obj, functionsList)


    def register_service(self, name, obj):
        """ Register given service and expose it to the world.
        """
        return _qimessagingswig.qi_session_register_service(self._session, name, obj._obj)

    def unregister_service(self, idx):
        """ Unregister service, it is not visible anymore.
        """
        _qimessagingswig.qi_session_unregister_service(self._session, idx)


    def _addfunc(self, name, signature, obj, sigreturn):
        """ Dynamicaly add function to object
        """
        def innerfunc(*args):
            """ Function called when alias is used.
            """
            return obj._call(signature, *args)
        innerfunc.__doc__ = "Docstring of %s" % name
        innerfunc.__name__ = name
        innerfunc.__signature__ = signature
        innerfunc.__sigreturn__ = sigreturn
        setattr(obj, innerfunc.__name__, innerfunc)

    def service(self, name):
        """ Ask to service directory for a service.

        On success, dynamicaly add remote methods to Python object.
        Generated method are a wrapper around Object.call method.
        """
        # Get C object.
        obj_c = _qimessagingswig.qi_session_get_service(self._session, name)

        # One failure, return None.
        if not obj_c:
            return None

        # Create Python object from C object.
        return GenericObject(obj_c)

    def close(self):
        """ Disconnect from service directory.
        """
        _qimessagingswig.qi_session_close(self._session)

    def __del__(self):
        """ Session destructor, also destroy C++ session
        """
        _qimessagingswig.qi_session_destroy(self._session)
