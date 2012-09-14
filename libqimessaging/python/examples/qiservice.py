##
## Copyright (C) 2012 Aldebaran Robotics
##

""" Python service providing the famous 'reply::s(s)' function
"""

import sys
import qimessaging

from qimessaging import Application
from qimessaging import Session
from qimessaging import ObjectBuilder

def service_reply(string):
    """ Famous reply::s(s) function
    """
    print "recv: %s" % string
    ret = "%sbim" % string
    return ret

def get_servicedirectory_address(argv):
    """ Parse command line arguments

    Print usage is service directory address is not set.
    """
    if len(argv) != 2:
        print 'Usage : %s qi-service.py directory-address' % argv[0]
        print 'Assuming service directory address is tcp://127.0.0.1:5555'
        return "tcp://127.0.0.1:5555"

    return argv[1]


def main():
    """ Entry point of qiservice
    """
    #0 Declare app
    app = Application()

    #1 Check if user give us service directory address.
    sd_addr = get_servicedirectory_address(sys.argv)

    #2 Open a session onto service directory.
    try:
        session = Session(sd_addr)
    except qimessaging.ConnectionError as session_error:
        print 'Connection error : %s' % session_error
        sys.exit()

    #3 Create an object builder and register method on it.
    builder = ObjectBuilder()
    builder.register_method("reply::s(s)", service_reply)
    obj = builder.object()

    if not obj:
        sys.exit()

    #4 Initialise service.
    session.listen("tcp://0.0.0.0:0")
    idx = session.register_service("serviceTest", obj)

    #5 Call Application.run() to join event loop.
    app.run()

    #6 Clean
    session.unregister_service(idx)
# main : Done.

if __name__ == "__main__":
    main()
