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
        print 'Assuming service directory address is tcp://127.0.0.1:9559'
        return "tcp://127.0.0.1:9559"

    return argv[1]

def callmoilababy(f):
    print "ici.com"
    #ret = f.value().reply("coco")
    #print "rep coco:", ret

def callmoilababy2(f):
    print "ici2.com"

def onPlaf(f):
    print "the result:", f.value()

def toto(session):

    f = session.connect("tcp://127.0.0.1:9559", _async=True)
    print "connected?", not f.has_error()

    #3 Get service serviceTest
    fut = session.service("serviceTest", _async=True)

    print "plouf1", fut.value()
    fut.add_callback(callmoilababy)
    print "plouf2"
    fut.add_callback(callmoilababy2)
    print "plouf3"

    obj = fut.value()
    print "obj:", obj
    print "dir:", dir(obj)
    print "o f:", obj.call
    print "dir f:", dir(obj.call)

    print "list:", obj.call("replyVector::()")
    print "map:", obj.call("replyMap::()")
    print "map:", obj.call("replyMap2::()")

    print "list:", obj.replyVector()
    print "map:", obj.replyMap()
    print "map:", obj.replyMap2()

    print "metaobj:", obj.call("metaObject::(I)", (1,))

    #print "props:", obj.call("properties", tuple(), None)
    print "props:", obj.call("properties::()", tuple())
    print "repl:", obj.call("reply::(s)", ("plouf",))
    print "repl:", obj.call("reply::(m)", ("plouf",))

    #4 Call foreign method reply
    #value = obj.reply("plaf", _overload="reply::(s)", _async=False)
    #print 'Reply : ', value

    f = obj.reply("plaf", _overload="reply::(s)", _async=True)
    #print "jen suis la, mais ca crack avant je pense"
    f.add_callback(onPlaf)

    i = 0
    while i < 3:
        print "ploof"
        time.sleep(1)
        i = i + 1
    #5 Cleanup
    session.close()

import sys

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
