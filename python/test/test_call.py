##
## Copyright (C) 2012 Aldebaran Robotics
##

""" Test method binding.

- Create a session and expose different type of methods
"""

from qimessaging.application import Application
from qimessaging.session import Session
from qimessaging.objectbuilder import ObjectBuilder
from qimessaging.binder import bind
from qimessagingswig import servicedirectory

@bind("s()")
def ping():
    """ Ping method, return 'pong' string.
    Sig : ping::s()
    """
    print "_ping : PONG !"
    return "pong"

@bind("v()")
def pingv():
    """ Ping method, do nothing.
    Sig : ping::v()
    """
    print "_pingv : PONG !"
    pass

@bind("d(dd)")
def add(a, b):
    """ Simple addition method
    Sig : add::d(dd) / add::d(id)
    """
    print '%f + %f = %f' % (a, b, a + b)
    return float(a) + float(b)

@bind("i(ii)")
def addi(a, b):
    """ Simple addition method
    Sig : add::i(ii)
    """
    print '%d + %d = %d' % (a, b, a + b)
    return a + b

@bind("i(ii)")
def sub(a, b):
    """ Simple addition method
    Sig : sub::i(iis) / sub::i(ii)
    """
    print '%d - %d = %d' % (a, b, a - b)
    return a - b

@bind("i(iis)")
def sub_message(a, b, m):
    """ Simple addition method
    Sig : sub::i(iis) / sub::i(ii)
    """
    print 'Message : "%s". %d - %d = %d' % (m, a, b, a - b)
    return a - b

@bind("s(s)")
def reply(string):
    """ Famous reply function
    Sig :: reply::s(s)
    """
    print "Received %s, replying %sbim" % (string, string)
    ret =  "%sbim" % string
    return ret

@bind("[s](ss)")
def split(string, separator):
    """ Split string with given separator
    Sig : split::[s](ss)
    """
    print "Spliting '%s' with '%s'" % (string, separator)
    ret = string.rsplit(separator)
    print ret
    return ret

@bind("i([f][f])")
def biggest_average(value1, value2):
    """ Compute average of 2 floating list and return 1 or 2.
    Sig : biggestAverage::i([f][f])
    """
    pass

@bind("i((ssi))")
def myprint(r):
    """ Elegant print information about a robot stored in a tuple
    Sig : print::i((ssi))
    """
    print "Robot %s, category %s, serial number %d" % (r[0], r[1], r[2])
    return 0

@bind("v([(ssi)])")
def printList(robots):
    """ Elegant print information about a list of robots stored in a tuple
    Sig : print::v([(ssi)])
    """
    for robot in robots:
        print(robot)

    return len(robots)

@bind("{s(ssi)}()")
def get_robots():
    """ Getter on a map of robots"
    Sig : robots::{s(ssi)}()
    """
    pass

def test_call():
    """ Create a session and bind all previous methods
    """
    sd = servicedirectory()
    app_ = Application()

    # Get service directory listening url.
    sd_addr = sd.listen_url()

    # Connect a session on service directory.
    service = Session(sd_addr)

    # Bind all methods to object builder.
    builder = ObjectBuilder()
    builder.register_method(reply)
    builder.register_method(ping)
    builder.register_method(pingv)
    builder.register_method(addi)
    builder.register_method(add)
    builder.register_method(sub)
    builder.register_method(sub_message)
    builder.register_method(split)
    builder.register_method(biggest_average)
    builder.register_method(myprint)
    builder.register_method(printList)
    builder.register_method(get_robots)

    # Create instance (aka Object) of service
    obj = builder.object()
    assert obj is not None

    # Expose service.
    assert service.listen("tcp://0.0.0.0:0") is True
    idx = service.register_service("test_call", obj)

    # Get proxy on service.
    proxy = service.service("test_call")
    assert proxy != None

    # Assert call with simple parameter and return value works.
    print "\nTest #1 : Calling reply('plaf')"
    ret = proxy.call("reply", "plaf")
    assert ret == "plafbim"

    # Assert call with no parameter and return value works.
    print "\nTest #2 : Calling ping()"
    ret = proxy.call("ping")
    assert ret == "pong"

    # Assert call with no parameter and no return value works.
    print "\nTest #3 : Calling void ping()"
    proxy.call("pingv")

    # Assert call to ambigous signature works.
    print "\nTest #4.1 : Calling ambiguous add(a, b)"
    retAdd = proxy.call("add", 4.2, 38.22)
    assert (retAdd - 42.42) <= 0.01
    print "\nTest #4.2"
    retAdd2 = proxy.call("add", 2, 40)
    assert retAdd2 == 42

    # Assert call to overloaded method works.
    print "\nTest #5.1 : Calling overloaded sub(a, b) and sub(a, b, str)"
    retSub = proxy.call("sub", 3, 2)
    assert retSub == 1
    print "\nTest #5.2"
    retSub2 = proxy.call("sub_message", 5, 2, "yolo")
    assert retSub2 == 3

    # Assert complex return value works (List).
    print '\nTest #6 : Calling split("riri;fifi;loulou", ";"'
    pig_list = proxy.call("split", "riri;fifi;loulou", ";")
    assert pig_list != None
    assert pig_list[0] == "riri"
    assert pig_list[1] == "fifi"
    assert pig_list[2] == "loulou"

    # Assert complex parameter works (Tuple).
    print '\nTest #7 : Calling print(("Gibouna", "Nao", 2345223))'
    robot = ("Gibouna", "Nao", 23443234)
    proxy.call("myprint", robot)

    # Assert complex recursive parameters works (List of tuple).
    print '\nTest #8 : Calling print(robot_list)'
    robot_list = [("Gibouna", "Nao", 23443234), ("Billy West", "Nao", 123456), ("Wall-E", "Garbage Collector", 55555505)]
    robot_number = proxy.call("printList", robot_list)

    # Cleanup
    print "Cleanup..."
    service.unregister_service(idx)
    service.close()
    sd.close()
    app_.stop()
    # main : Done.

if __name__ == "__main__":
    test_call()
