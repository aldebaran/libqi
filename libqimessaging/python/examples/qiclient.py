##
## Copyright (C) 2012 Aldebaran Robotics
##

""" Python client implementation of famous QiMessaging hello world : serviceTest
"""

import sys
import qimessaging

from qimessaging import Application
from qimessaging import Session

def get_service(session, service_name):
    """ Get service given by service_name.

    .. note:: Exit on failure.
    """
    obj = session.service(service_name)

    if obj is None:
        print "Oops, cannot get service %s" % service_name
        sys.exit()

    return obj


def call_reply(obj):
    """ Synchronous call to serviceTest.reply::s(s)
    """
    value = obj.reply("plaf");

    print 'Reply : %s' % value

def get_servicedirectory_address(argv):
    """ Parse command line arguments

    Print usage is service directory address is not set.
    """
    if len(argv) != 2:
        print 'Usage : python2 qi-client.py directory-address'
        print 'Assuming service directory address is tcp://127.0.0.1:5555'
        return "tcp://127.0.0.1:5555"

    return argv[1]

def  main():
    """ Entry point of qiservice
    """
    #0 Declare app
    _application = Application()

    #1 Check if user give us service directory address.
    sd_addr = get_servicedirectory_address(sys.argv)

    #2 Open a session onto service directory.
    try:
        session = Session(sd_addr)
    except qimessaging.ConnectionError as session_error:
        print 'Connection error : %s' % session_error
        sys.exit()

    #3 Get service serviceTest
    obj = get_service(session, "serviceTest")

    #4 Call foreign method reply
    call_reply(obj)

    #5 Cleanup
    session.close()
    _application.stop()

if __name__ == "__main__":
    main()
