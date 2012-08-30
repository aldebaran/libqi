#!/usr/bin/env python
##
## Author(s):
##  - Pierre Roullon <proullon@aldebaran-robotics.com>
##
## Copyright (C) 2010, 2011, 2012 Aldebaran Robotics
##

import _qi

class Session:
    def __init__(self):
        self.session = _qi.qi_session_create()

    def connect(self, address):
        return _qi.qi_session_connect(self.session, address)

    def listen(self, address):
        return _qi.qi_session_listen(self.session, address)

    def registerService(self, name, obj):
        return _qi.qi_session_register_service(self.session, name, obj)

    def unregistedService(self, idx):
        _qi.qi_session_unregister_service(self.session, idx)

    def waitForConnected(self, msecs):
        _qi.qi_session_wait_for_connected(self.session, msecs)

    def service(self, name):
        return _qi.qi_session_get_service(self.session, name)

    def disconnect(self):
        _qi.qi_session_disconnect(self.session)

    def waitForDisconnected(self, msecs):
        _qi.qi_session_wait_for_disconnected(self.session, msecs)

    def __del__(self):
        _qi.qi_session_destroy(self.session)


# Goal :

#def main():
#    ses = qi.Session();
#    ses.connect("tcp://blabla:9559");
#    obj = ses.service("Memory")
#    obj.call("toto", 2, 4)
