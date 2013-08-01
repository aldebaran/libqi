#!/usr/bin/python2
##
## Copyright (C) 2012, 2013 Aldebaran Robotics
##

""" Python client implementation of famous QiMessaging hello world : serviceTest
"""

import time
import sys
import qi

def get_servicedirectory_address(argv):
    """ Parse command line arguments

    Print usage is service directory address is not set.
    """
    if len(argv) != 2:
        print 'Usage : python2 qi-client.py directory-address'
        print 'Assuming service directory address is tcp://127.0.0.1:9559'
        return "tcp://127.0.0.1:9559"

    return argv[1]

def onReply(fut):
    print "async repl:", fut.value()

def onServiceAvailable(fut):
    print "onServiceAvailable"

def onTestEvent(v):
    print "Event:", v

def onTestEventGeneric(*args):
    print "EventGeneric:", args

def  main():
    """ Entry point of qiservice
    """
    session = qi.Session()
    f = session.connect("tcp://127.0.0.1:9559", _async=True)
    print "connected?", not f.hasError()

    #3 Get service serviceTest
    fut = session.service("serviceTest", _async=True)
    fut.addCallback(onServiceAvailable)

    obj = fut.value()

    #obj.testEvent.connect(onTestEvent)
    obj.testEventGeneric.connect(onTestEventGeneric)

    print "repl:", obj.call("reply", "plouf")
    f = obj.reply("plaf", _async=True)
    f.addCallback(onReply)

    i = 0
    while i < 2:
        print "waiting..."
        time.sleep(1)
        i = i + 1
    session.close()

if __name__ == "__main__":
    main()
