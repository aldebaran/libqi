##
## Copyright (C) 2012, 2013 Aldebaran Robotics
##

""" Python client implementation of famous QiMessaging hello world : serviceTest
"""

import time
import sys
import qi

def get_service(session, service_name):
    """ Get service given by service_name.

    .. note:: Exit on failure.
    """

    return obj

def call_reply(obj):
    """ Synchronous call to serviceTest.reply::s(s)
    """

def get_servicedirectory_address(argv):
    """ Parse command line arguments

    Print usage is service directory address is not set.
    """
    if len(argv) != 2:
        print 'Usage : python2 qi-client.py directory-address'
        print 'Assuming service directory address is tcp://127.0.0.1:5555'
        return "tcp://127.0.0.1:5555"

    return argv[1]

def callmoilababy(f):
    print "ici.com"
    ret = f.value().reply("coco")
    print "rep coco:", ret

def callmoilababy2(f):
    print "ici2.com"

def onPlaf(f):
    print "result:", f.value()

def toto(session):

    session.connect("tcp://127.0.0.1:5555", async=True)
    #3 Get service serviceTest
    fut = session.service("serviceTest", async=True)

    print "plouf1"
    fut.add_callback(callmoilababy)
    print "plouf2"
    fut.add_callback(callmoilababy2)
    print "plouf3"

    obj = fut.value()
    print "obj:", obj
    print "dir:", dir(obj)
    print "o f:", obj.reply
    print "dir f:", dir(obj.reply)
    #4 Call foreign method reply
    value = obj.reply("plaf", _overload="reply::(s)", _async=False)
    print 'Reply : ', dir(value), value

    f = obj.reply("plaf", async=True)
    f.add_callback(onPlaf)

    i = 0
    while i < 3:
        print "ploof"
        time.sleep(1)
        i = i + 1
    #5 Cleanup
    session.close()

def  main():
    """ Entry point of qiservice
    """
    #0 Declare app
    _application = qi.Application()

    #1 Check if user give us service directory address.
    sd_addr = get_servicedirectory_address(sys.argv)

    #2 Open a session onto service directory.
    session = qi.Session()

    toto(session)

    _application.stop()

if __name__ == "__main__":
    main()
