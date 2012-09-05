#!/usr/bin/env python
##
## Copyright (C) 2010, 2011, 2012 Aldebaran Robotics
##

import sys

import _qi
from qimessaging import Application

def call_reply(sd_addr):
    session = _qi.qi_session_create()
    _qi.qi_session_connect(session, sd_addr)

    if _qi.qi_session_wait_for_connected(session, 1000) == False:
        print "Cannot connect to service directory"
        sys.exit()

    obj = _qi.qi_session_get_service(session, "serviceTest")
    if obj is None:
        print "Oops, cannot get service serviceTest"
        sys.exit()

    message = _qi.qi_message_create()
    _qi.qi_message_write_string(message, "king")
    future = _qi.qi_object_call(obj, "reply::(s)", message)

    _qi.qi_future_wait(future);

    if _qi.qi_future_is_error(future):
        print "An error occured : %s" % _qi.qi_future_get_error(future)
        sys.exit()

    if _qi.qi_future_is_ready(future) and not _qi.qi_future_is_error(future):
        answer = _qi.qi_message_cast(_qi.qi_future_get_value(future))
        value = _qi.qi_message_read_string(answer);
        print "Reply : %s" % value

    _qi.qi_session_close(session)


def get_servicedirectory_address(argv):
    if len(argv) != 2:
        print('Usage : python2 qi-client.py directory-address')
        print('Assuming service directory address is tcp://127.0.0.1:5555')
        return "tcp://127.0.0.1:5555"

    return argv[1]

def main(argv):
    sd_addr = get_servicedirectory_address(argv)
    return call_reply(sd_addr)

if __name__ == "__main__":
    app = Application(sys.argv)
    main(sys.argv)
