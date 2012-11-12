##
## Copyright (C) 2012 Aldebaran Robotics
##

""" Integration test for QiMessaging Python bindings.

- Create a session and expose an service.
- Get service with another session.
- Try a call.
"""

from qimessaging.application import Application
from qimessaging.session import Session
from qimessaging.objectbuilder import ObjectBuilder
from qimessagingswig import servicedirectory

def service_reply(string):
    """ Simple 'for sure !' string concatener.
    """
    ret = "%s, for sure !" % string
    return ret

def test_integration():
    """ Test both client and service side of Python QiMessaging bindings.
    """
    sd = servicedirectory()
    application_ = Application()

    #1 Get service directory listening url.
    sd_addr = sd.listen_url()

    #2 Connect a first session to service directory (Client use).
    client_session = Session(sd_addr)

    #3 Connect a second session (Service use).
    service_session = Session(sd_addr)

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
    ret = service_test.reply("testing")
    assert ret == "testing, for sure !"

    #9 Clean up
    application_.stop()
    client_session.close()
    service_session.unregister_service(idx)
    service_session.close()
    # main : Done.

if __name__ == "__main__":
    test_integration()
