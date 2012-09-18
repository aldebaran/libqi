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

import _qi
from .object import GenericObject

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
        return repr(self._value)

class Session:
    """ Package all function needed to create and connect to QiMessage services.
    """
    def __init__(self, address = None):
        """ Session constructor, if address is set, try to connect.
        """
        self._session = _qi.qi_session_create()
        if address:
            self.connect(address)

    def connect(self, address):
        """ Connect to service directory.

        .. Raises::
            ConnectionError exception.
        """
        if not _qi.qi_session_connect(self._session, address):
            raise ConnectionError('Cannot connect to ' + address)

    def listen(self, address):
        """ Listen on given address.

        Uppon connection, return service asked.
        """
        if _qi.qi_session_listen(self._session, address):
            return True
        return False

    def register_service(self, name, obj):
        """ Register given service and expose it to the world.
        """
        return _qi.qi_session_register_service(self._session, name, obj._obj)

    def unregister_service(self, idx):
        """ Unregister service, it is not visible anymore.
        """
        _qi.qi_session_unregister_service(self._session, idx)


    def _addfunc(self, name, signature, obj):
        def innerfunc(*args):
             return obj.call(signature, *args)
        innerfunc.__doc__ = "Docstring or %s" % name
        innerfunc.__name__ = name
        setattr(obj, innerfunc.__name__, innerfunc)

    def service(self, name):
        """ Ask to service directory for a service.

        On success, dynamicaly add remote methods to Python object.
        Generated method are a wrapper around Object.call method.
        """
        #1 Get C object.
        obj_c = _qi.qi_session_get_service(self._session, name)

        #1.1 One failure, return None.
        if not obj_c:
            return None

        #2 Create Python object from C object.
        obj = Object(obj_c)

        #3 Get all remote methods signature.
        methods = _qi.qi_object_methods_vector(obj_c)

        #4 Create Python methods
        for m in methods:
            # 4.1 Set method signature and name.
            signature = m
            name = m.rsplit("::", 2)[0]

            #4.2 Add method in object. NB: overloaded functions are merged, signature becomes function name.
            # Disambiguation is done at call.
            self._addfunc(name, signature, obj)

        return obj

    def close(self):
        """ Disconnect from service directory.
        """
        _qi.qi_session_close(self._session)

    def __del__(self):
        """ Session destructor, also destroy C++ session
        """
        _qi.qi_session_destroy(self._session)
