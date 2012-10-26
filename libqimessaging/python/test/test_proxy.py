##
## Copyright (C) 2012 Aldebaran Robotics
##

""" Test dynamic method creation

- Also  test other things
"""

from qimessaging.application import Application
from qimessaging.session import Session
from qimessaging.objectbuilder import ObjectBuilder
from qimessagingswig import servicedirectory

def _reply(string):
    """ Famous reply function
    Sig :: reply::s(s)
    """
    ret =  "%sbim" % string
    return ret

def test_proxy():
    """ Create a session and bind all previous methods
    """
    sd = servicedirectory()
    app_ = Application()

    #1 Get service directory listening url.
    sd_addr = sd.listen_url()

    #2 Connect a session on service directory.
    service = Session(sd_addr)
    print "so long ?"
    #3 bind all methods to object builder.
    builder = ObjectBuilder()
    builder.register_method("reply::s(s)", _reply)
    print "be ready"
    #4 Create instance (aka Object) of service
    obj = builder.object()
    assert obj is not None
    print "it's gonna be..."
    #5 Expose service.
    assert service.listen("tcp://0.0.0.0:0") is True
    print "wait for it..."
    idx = service.register_service("test_proxy", obj)
    print "LEGENDARY"
    #6 Get proxy on service.
    proxy = service.service("test_proxy")

    #7 Assert dynamic method creation works.
    proxy.reply("test")

    # Cleanup
    app_.stop()
    service.unregister_service(idx)
    service.close()
    # main : Done.

if __name__ == "__main__":
    test_proxy()
