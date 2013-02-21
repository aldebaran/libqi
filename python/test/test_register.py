##
## Copyright (C) 2012 Aldebaran Robotics
##

""" Automatic class instance registering for qimessaging Python bindings
"""

from qimessaging.application import Application
from qimessaging.session import Session
from qimessagingswig import servicedirectory
from qimessaging.binder import bind, nobind

class  RandomClass:
    """ Sample class for Python autobind
    """

    # Bound with a custom signature
    @bind("s(i)")
    def foo(self, integer):
        """ Return Foo
        """
        return "Foo"

    # Bind this method with default parameters
    def bar(self):
        """ Return bar!
        """
        return "bar!"

    # Do not bind this method
    @nobind
    def myPrivateFunction(self):
      pass

def main():
    """ Instanciate qimessaging session and bind RandomClass instance
    """
    sd = servicedirectory()

    # Get qimessaging session
    sess = Session(sd.listen_url())

    # Get RandomClass instance
    test = RandomClass()

    # Try to bind simple int
    integer = 0
    assert sess.register_object("Integer", integer) == False

    # Bind RandomClass instance
    assert sess.register_object("RandomClass", test) > 0

    # Cleanup

if __name__ == "__main__":
    main()
