#!/usr/bin/env python
##
## Copyright (C) 2010, 2011, 2012 Aldebaran Robotics
##

import sys

from qimessaging import Session
from qimessaging import Message
from qimessaging import Application
from qimessaging import Object


def init_session(sd_addr):

    session = Session()
    session.connect(sd_addr)

    if not session.wait_for_connected():
        print "Cannot connect to service directory"
        return None

    return session

def get_service(session, service_name):
    obj = session.service(service_name)

    if obj is None:
        print "Oops, cannot get service serviceTest"
        return None

    return obj


def call_reply(obj):
    print "\n\nSynchronous call : ('reply', 2)"

    value = obj.call("reply", 2)

    if value and obj.get_last_error() is not None:
        print "Oops, call failed : %s" % obj.get_last_error()
        pass

    print 'Reply : %s' % value

def a_call_reply(obj):
    print "Asynchronous call :"

    message = Message()
    message.write_string("plaf")
    future = obj.async_call("reply::(s)", message)

    future.wait()

    if future.is_error():
        print "Oops, an error occured : %s" % future.get_error()
        pass

    if future.is_ready() and not future.is_error():
        answer = future.get_value()
        value = answer.read_string();
        print 'Reply : %s' % value


def get_servicedirectory_address(argv):
    if len(argv) != 2:
        print 'Usage : python2 qi-client.py directory-address'
        print 'Assuming service directory address is tcp://127.0.0.1:5555'
        return "tcp://127.0.0.1:5555"

    return argv[1]

def  main(argv):
    sd_addr = get_servicedirectory_address(argv)

    session = init_session(sd_addr)
    if session is None:
        sys.exit()

    obj = get_service(session, "serviceTest")
    if obj is None:
        sys.exit()

    a_call_reply(obj)
    call_reply(obj)

    print "\n"
    session.disconnect()
    sys.exit()

if __name__ == "__main__":
    app = Application(sys.argv)
    main(sys.argv)
