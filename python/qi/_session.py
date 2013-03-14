##
## Author(s):
##  - Cedric GESTES <gestes@aldebaran-robotics.com>
##
## Copyright (C) 2013 Aldebaran Robotics
##

import _qipy
from qi.genericobject import GenericObject
from qi.future import Future, Promise

__all__ = ( 'Session', )

##TODO: should support with Sesssion()
class Session:
    """Package all function needed to create and connect to _qipyessage services.

    It can be used as a dictionnary of services, with keys representing the names
    of the services available and the values said services.
    """

    def __init__(self, address=None):
        """Session constructor, if address is set, try to connect."""
        self._session = _qipy.qi_session_create()
        if address is not None:
            self.connect(address)

    def __del__(self):
        """Session destructor, also destroy C++ session."""
        _qipy.qi_session_destroy(self._session)

    def connect(self, address, async=False):
        """ Connect to service directory.

        :raises: ConnectionError
        """

        fut = Future(_qipy.qi_session_connect(self._session, address))
        if async:
            return fut
        if fut.has_error():
            raise ConnectionError('Cannot connect to ' + address + ': ' + fut.error())

    def listen(self, address, async=False):
        """Listen on given address.

        :param address: A valid IPv4 address we can bind to.
        """
        fut = Future(_qipy.qi_session_listen(self._session, address))
        if async:
            return fut
        if fut.has_error():
            #todo: another error?
            raise ConnectionError('Cannot listen on ' + address + ': ' + fut.error())


    def register_object(self, name, obj, async=False):
        """Register given Python class instance.

        :param name: Name of the object.
        :param obj: Instance of this object.
        """
        #TODO
        functionsList = binder.buildFunctionListFromObject(obj)
        return _qipy.py_session_register_object(self._session, name, obj, functionsList)


    def register_service(self, name, obj, async=False):
        """Register given service and expose it to the world.

        :param name: Name of the service to register.
        :param obj: The actual service to bind.
        :returns: The id of the service.
        """
        #TODO
        return _qipy.qi_session_register_service(self._session, name, obj._obj)

    def unregister_service(self, idx, async=False):
        """Unregister service, it is not visible anymore.

        :param idx: id of the service given on register.
        """
        #TODO
        _qipy.qi_session_unregister_service(self._session, idx)

    def service(self, name, default=None, async=False):
        """Fetches a new instance of GenericObject binding to a service. If the
        service is unknown, returns *default*.

        :param name: Name of the service.
        :param default: Default value if service is unknown.
        """
        if not isinstance(name, str):
            raise TypeError('keys must be strings.')

        # Get C object.
        fut_c = _qipy.qi_session_get_service(self._session, name)
        if not async:
            #this clear fut_c. do not use fut_c after that.
            obj_c = _qipy.qi_future_get_object(fut_c)
            # One failure, return None.
            if not obj_c:
                return default

            # Create Python object from C object.
            return GenericObject(qi_object=obj_c, qi_name=name)
        return Future(fut_c)

    def services(self, async=False):
        """Retrieves the list of the names of all the services available on the
        service directory."""
        fut = Future(_qipy.qi_session_get_services(self._session))
        if async:
            return fut
        return fut.value()

    def close(self):
        """Closes connection with the service directory."""
        _qipy.qi_session_close(self._session)
