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

# NOTE: moved to collections module in python 3.
import UserDict

import _qimessagingswig as _qim
import qimessaging.binder as binder

from qimessaging.genericobject import GenericObject

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


class Session(UserDict.DictMixin):
    """Package all function needed to create and connect to qimessage services.

    It can be used as a dictionnary of services, with keys representing the names
    of the services available and the values said services.
    """

    def __init__(self, address=None):
        """Session constructor, if address is set, try to connect."""
        self._session = _qim.qi_session_create()
        if address is not None:
            self.connect(address)

    def __del__(self):
        """Session destructor, also destroy C++ session."""
        _qim.qi_session_destroy(self._session)

    def connect(self, address):
        """ Connect to service directory.

        :raises: ConnectionError
        """
        if not _qim.qi_session_connect(self._session, address):
            raise ConnectionError('Cannot connect to ' + address)

    def listen(self, address):
        """Listen on given address.

        :param address: A valid IPv4 address we can bind to.
        """
        if _qim.qi_session_listen(self._session, address):
            return True
        return False

    def register_object(self, name, obj):
        """Register given Python class instance.

        :param name: Name of the object.
        :param obj: Instance of this object.
        """
        functionsList = binder.buildFunctionListFromObject(obj)
        return _qim.py_session_register_object(self._session, name, obj, functionsList)


    def register_service(self, name, obj):
        """Register given service and expose it to the world.

        :param name: Name of the service to register.
        :param obj: The actual service to bind.
        :returns: The id of the service.
        """
        return _qim.qi_session_register_service(self._session, name, obj._obj)

    def unregister_service(self, idx):
        """Unregister service, it is not visible anymore.

        :param idx: id of the service given on register.
        """
        _qim.qi_session_unregister_service(self._session, idx)

    def service(self, name, default=None):
        """Fetches a new instance of GenericObject binding to a service. If the
        service is unknown, returns *default*.

        :param name: Name of the service.
        :param default: Default value if service is unknown.
        """
        try:
            return self.__getitem__(name)
        except KeyError:
            return default

    def services(self):
        """Retrieves the list of the names of all the services available on the
        service directory."""
        services = _qim.qi_session_get_services(self._session)
        if services is None:
            raise SessionNotConnectedError()
        return services

    def close(self):
        """Closes connection with the service directory."""
        _qim.qi_session_close(self._session)

    # Iteration over services functions.
    def __getitem__(self, name):
        """Ask to service directory for a service.

        :param name: The name of the service.
        :raises: TypeError, KeyError
        """
        if not isinstance(name, str):
            raise TypeError('keys must be strings.')

        # Get C object.
        obj_c = _qim.qi_session_get_service(self._session, name)

        # One failure, return None.
        if not obj_c:
            raise KeyError("unknow service '%s'" % name)

        # Create Python object from C object.
        return GenericObject(obj_c)

    def keys(self):
        """Alias for :func:`services`."""
        return self.services()
