##
## Author(s):
##  - Pierre Roullon <proullon@aldebaran-robotics.com>
##
## Copyright (C) 2010, 2011, 2012 Aldebaran Robotics
##

import _qi
from .object import Object

class ConnectionError(Exception):
    def __init__(self, value):
        self.value = value

    def __str__(self):
        return repr(self.value)

class Session:
    def __init__(self):
        self.session = _qi.qi_session_create()

    def connect(self, address):
        if _qi.qi_session_connect(self.session, address) == 1:
            raise ConnectionError('Cannot connect to ' + address)

    def listen(self, address):
        if _qi.qi_session_listen(self.session, address) == 1:
            return True
        return False

    def register_service(self, name, obj):
        return _qi.qi_session_register_service(self.session, name, obj)

    def unregister_service(self, idx):
        _qi.qi_session_unregister_service(self.session, idx)

    def service(self, name):
        obj = _qi.qi_session_get_service(self.session, name)

        if not obj:
            return None

    def close(self):
        _qi.qi_session_close(self.session)

    def __del__(self):
        _qi.qi_session_destroy(self.session)
