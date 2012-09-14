##
## Copyright (C) 2012 Aldebaran Robotics
##

""" General test for QiMessaging Python bindings.

- Create a session and expose an service.
- Get service with another session.
- Try a call.
"""

import sys
import qimessaging

from qimessaging import Application
from qimessaging import Session

from qimessaging import ObjectBuilder

def service_reply(string):
    """ Simple 'bim' string concatener.
    """
    print "recv: %s" % string
    ret = "%s, for sure !" % string
    return ret

def get_servicedirectory_address(argv):
    """ Parse command line arguments
    """
    if len(argv) != 2:
        return "tcp://127.0.0.1:5555"

    return argv[1]

def test_integration():
    """ Test both client and service side of Python QiMessaging bindings.
    """
    application_ = Application()

    #1 Check if user gave us service directory address.
    sd_addr = get_servicedirectory_address(args)

    #2 Get a first session on it (Client use).
    try:
        client_session = Session(sd_addr)
    except qimessaging.ConnectionError:
        assert False

    #3 Get a second session (Service use).
    try:
        service_session = Session(sd_addr)
    except qimessaging.ConnectionError:
        assert False

    #4 Create an object builder and register method on it.
    builder = ObjectBuilder()
    builder.register_method("reply::s(s)", service_reply)

    #5 Create instance (aka Object) of service
    obj = builder.object()
    assert obj is not None

    #6 Initialise service.
    assert service_session.listen("tcp://0.0.0.0:0") is True
    idx = service_session.register_service("test_python_bindings_service", obj)

    #7 Get service with client session.
    service_test = client_session.service("test_python_bindings_service")
    assert service_test is not None

    #8 Call reply bound method and check return value
    ret = service_test.call("reply::(s)", "testing")
    print ret
    assert ret == "testing, for sure !"

    #9 Clean up
    client_session.close()
    service_session.unregister_service(idx)
    # main : Done.
