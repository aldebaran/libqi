##
## Copyright (C) 2012 Aldebaran Robotics
##

""" Test method binding.

- Create a session and expose different type of methods
"""

import sys
import qimessaging
import _qimessagingswig

from qimessaging.application import Application
from qimessaging.session import Session
from qimessaging.objectbuilder import ObjectBuilder
from qimessagingswig import servicedirectory

def _ping():
    """ Ping method, return 'pong' string.
    Sig : ping::s()
    """
    print "_ping : PONG !"
    return "pong"

def _pingv():
    """ Ping method, do nothing.
    Sig : ping::v()
    """
    print "_pingv : PONG !"
    pass

def _add(a, b):
    """ Simple addition method
    Sig : add::d(dd) / add::d(id)
    """
    print '%f + %f = %f' % (a, b, a + b)
    return float(a) + float(b)

def _addi(a, b):
    """ Simple addition method
    Sig : add::i(ii)
    """
    print '%d + %d = %d' % (a, b, a + b)
    return a + b

def _sub(a, b):
    """ Simple addition method
    Sig : sub::i(iis) / sub::i(ii)
    """
    print '%d - %d = %d' % (a, b, a - b)
    return a - b

def _sub_message(a, b, m):
    """ Simple addition method
    Sig : sub::i(iis) / sub::i(ii)
    """
    print 'Message : "%s". %d - %d = %d' % (m, a, b, a - b)
    return a - b

def _reply(string):
    """ Famous reply function
    Sig :: reply::s(s)
    """
    print "Received %s, replying %sbim" % (string, string)
    ret =  "%sbim" % string
    return ret

def _split(string, separator):
    """ Split string with given separator
    Sig : split::[s](ss)
    """
    print "Spliting '%s' with '%s'" % (string, separator)
    ret = string.rsplit(separator)
    print ret
    return ret

def _biggest_average(value1, value2):
    """ Compute average of 2 floating list and return 1 or 2.
    Sig : biggestAverage::i([f][f])
    """
    pass

def _print(robot):
    """ Elegant print information about a robot stored in a tuple
    Sig : print::b((ssi))
    """
    print "Robot %s, category %s, serial number %d" % (robot[0], robot[1], robot[2])

def _printList(robots):
    """ Elegant print information about a list of robots stored in a tuple
    Sig : print::v([(ssi)])
    """
    for robot in robots:
        _print(robot)

    return len(robots)

def _get_robots():
    """ Getter on a map of robots"
    Sig : robots::{s(ssi)}()
    """
    pass

def test_call():
    """ Create a session and bind all previous methods
    """
    sd = servicedirectory()
    app_ = Application()

    #1 Get service directory listening url.
    sd_addr = sd.listen_url()

    #2 Connect a session on service directory.
    service = Session(sd_addr)

    #3 bind all methods to object builder.
    builder = ObjectBuilder()
    builder.register_method("reply::s(s)", _reply)
    builder.register_method("ping::s()", _ping)
    builder.register_method("pingv::v()", _pingv)
    builder.register_method("add::i(ii)", _addi)
    builder.register_method("add::f(ff)", _add)
    builder.register_method("sub::i(ii)", _sub)
    builder.register_method("sub::i(iis)", _sub_message)
    builder.register_method("split::[s](ss)", _split)
    builder.register_method("biggestAverage::i([f][f])", _biggest_average)
    builder.register_method("print::v((ssi))", _print)
    builder.register_method("print::i([(ssi)])", _printList)
    builder.register_method("robots::{s(ssi)}()", _get_robots)

    #4 Create instance (aka Object) of service
    obj = builder.object()
    assert obj is not None

    #5 Expose service.
    assert service.listen("tcp://0.0.0.0:0") is True
    idx = service.register_service("test_call", obj)

    #6 Get proxy on service.
    proxy = service.service("test_call")
    assert proxy != None

    #7 Assert call with simple parameter and return value works.
    print "\nTest #1 : Calling reply('plaf')"
    ret = proxy.call("reply", "plaf")
    assert ret == "plafbim"

    #8 Assert call with no parameter and return value works.
    #print "\nTest #2 : Calling ping()"
    #ret = proxy.call("ping")
    #print ret
    #assert ret == "pong"

    #9 Assert call with no parameter and no return value works.
    #print "\nTest #3 : Calling void ping()"
    #roxy.call("pingv")

    #10 Assert call to ambigous signature works.
    print "\nTest #4.1 : Calling ambiguous add(a, b)"
    retAdd = proxy.call("add", 4.2, 38.22)
    assert (retAdd - 42.42) <= 0.01
    print "\nTest #4.2"
    retAdd2 = proxy.call("add", 2, 40)
    assert retAdd2 == 42

    #11 Assert call to overloaded method works.
    print "\nTest #5.1 : Calling overloaded sub(a, b) and sub(a, b, str)"
    retSub = proxy.call("sub", 3, 2)
    assert retSub == 1
    print "\nTest #5.2"
    retSub2 = proxy.call("sub", 5, 2, "yolo")
    assert retSub2 == 3

    #12 Assert complex return value works (List).
    print '\nTest #6 : Calling split("riri;fifi;loulou", ";"'
    pig_list = proxy.call("split", "riri;fifi;loulou", ";")
    assert pig_list != None
    assert pig_list[0] == "riri"
    assert pig_list[1] == "fifi"
    assert pig_list[2] == "loulou"

    #13 Assert complex parameter works (Tuple).
    print '\nTest #7 : Calling print(("Gibouna", "Nao", 2345223))'
    robot = ("Gibouna", "Nao", 23443234)
    proxy.call("print", robot)

    #14 Assert complex recursive parameters works (List of tuple).
    #print '\nTest #7 : Calling print(robot_list)'
    #robot_list = [("Gibouna", "Nao", 23443234), ("Billy West", "Nao", 123456), ("Wall-E", "Garbage Collector", 55555505)]
    #robot_number = proxy.call("print", robot_list)

    # Cleanup
    print "Cleanup..."
    service.unregister_service(idx)
    service.close()
    app_.stop()
    # main : Done.

if __name__ == "__main__":
    test_call()
