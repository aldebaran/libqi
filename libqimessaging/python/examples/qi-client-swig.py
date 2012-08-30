#!/usr/bin/env python
##
## Author(s):
##  - Pierre Roullon <proullon@aldebaran-robotics.com>
##
## Copyright (C) 2010, 2011, 2012 Aldebaran Robotics
##

import _qi
from ctypes import *

def callReply(sdAddr):
    print("Connecting to serviceTest")

    session = _qi.qi_session_create()
    _qi.qi_session_connect(session, sdAddr)

    if _qi.qi_session_wait_for_connected(session, 1000) == False:
        print("Cannot connect to service directory")
        return 1

    obj = _qi.qi_session_get_service(session, "serviceTest")
    if obj == 0:
        print("Oops, cannot get service serviceTest")
        return 1

    message = _qi.qi_message_create()
    _qi.qi_message_write_string(message, "plaf")
    future = _qi.qi_object_call(obj, "reply::s(s)", message)

    _qi.qi_future_wait(future);

    if _qi.qi_future_is_error(future) == True:
        print("An error occured :", _qi.qi_future_get_error())
        return 1

    if _qi.qi_future_is_ready(future) == True and _qi.qi_future_is_error(future) == False:
        answer = _qi.qi_future_get_value(future)
        value = _qi.qi_message_read_string(answer);
        print("Reply :", value)

    _qi.qi_session_disconnect(session)
    return 0

def main(argc, argv):
    print("qi-client.py")
    # Todo : Parse argument
    print("Assuming master-address is tcp://127.0.0.1:5555")
    sdAddr = "tcp://127.0.0.1:5555"
    return callReply(sdAddr)

if __name__ == "__main__":
    main(0, 0)
