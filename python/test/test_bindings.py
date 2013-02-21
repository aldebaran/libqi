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
from qimessaging.binder import bind
from qimessaging.objectbuilder import ObjectBuilder
from qimessagingswig import servicedirectory

@bind("s(s)")
def reply(string):
    """ Simple 'for sure !' string concatener.
    """
    ret = "%s, for sure !" % string
    return ret

def main():
    """ Test both client and service side of Python QiMessaging bindings.
    """
    application_ = Application()
    sd = servicedirectory()

    # Get service directory listening url.
    sd_addr = sd.listen_url()

    # Connect a first session to service directory (Client use).
    client_session = Session(sd_addr)

    # Connect a second session (Service use).
    service_session = Session(sd_addr)

    # Create an object builder and register method on it.
    builder = ObjectBuilder()
    builder.register_method(reply)

    # Create instance (aka Object) of service
    obj = builder.object()
    assert obj is not None

    # Initialise service.
    assert service_session.listen("tcp://0.0.0.0:0") is True
    idx = service_session.register_service("test_python_bindings_service", obj)

    # Get service with client session.
    service_test = client_session.service("test_python_bindings_service")
    assert service_test is not None

    # Call reply bound method and check return value
    ret = service_test.reply("testing")
    assert ret == "testing, for sure !"

    # Clean up
    application_.stop()
    client_session.close()
    service_session.unregister_service(idx)
    service_session.close()
    # main : Done.

if __name__ == "__main__":
    main()
