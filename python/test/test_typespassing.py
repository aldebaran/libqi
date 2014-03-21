#!/usr/bin/env python
##
## Author(s):
##  - Vincent Barbaresi <vbarbaresi@aldebaran-robotics.com>
##
## Copyright (C) 2013 Aldebaran Robotics

import time
import qi
import sys
import pytest

class TestService:
    def display(self, t):
        return t


def test_throwing_callback():
    def raising(f):
        raise Exception("woops")

    local = "tcp://127.0.0.1:0"
    sd = qi.Session()
    sd.listenStandalone(local)

    s = qi.Session()
    s.connect(sd.endpoints()[0])
    f = s.service("ServiceDirectory", _async=True)

    f.addCallback(raising)
    time.sleep(0.01)
    s.close()


def test_unicode_strings():
    local = "tcp://127.0.0.1:0"
    sd = qi.Session()
    sd.listenStandalone(local)

    s = qi.Session()
    s.connect(sd.endpoints()[0])

    m = TestService()
    s.registerService("TestService", m)
    service = s.service("TestService")
    # ASCII range
    unicode_string = ''.join([chr(i) for i in range(1, 128)])
    mystring = service.display(unicode_string)
    print("mystr:", mystring)
    print("uystr:", unicode_string)
    assert type(mystring) == str
    assert mystring.encode("ascii") == unicode_string.encode("ascii")

    # Wide unicode
    wide_string = "\\U00010000" * 39 + "\\uffff" * 4096
    mystring = service.display(wide_string)
    assert mystring == wide_string

    # String with many unicode chars
    if sys.version_info[0] == 2:
        unicode_string = ''.join([unichr(i) for i in range(1, 50000)])
    else:
        unicode_string = ''.join([chr(i) for i in range(1, 50000)])
    service.display(unicode_string)
    time.sleep(0.01)
    s.close()


def test_builtin_types():
    local = "tcp://127.0.0.1:0"
    sd = qi.Session()
    sd.listenStandalone(local)

    s = qi.Session()
    s.connect(sd.endpoints()[0])

    m = TestService()
    s.registerService("TestService", m)
    service = s.service("TestService")

    # None
    assert service.display(None) is None
    # bool
    t, f = service.display(True), service.display(False)
    assert t == 1  # is True ?
    assert f == 0  # is False ?

    # int
    import sys
    if sys.version < '3':
        integer_types = (int, long,)
    else:
        integer_types = (int,)
    assert isinstance(service.display(42), integer_types)
    assert service.display(42) == 42

    # float
    assert service.display(0.1337) == 0.1337

    # long (32b)
    assert service.display(2 ** 31 - 1) == 2147483647

    # list
    assert service.display([]) == []
    assert service.display([1]) == [1]
    assert service.display(["bla", "bli"]) == ["bla", "bli"]

    # sets
    assert service.display(set([1, 2])) == (1, 2)
    assert service.display(frozenset([1, 2])) == (1, 2)
    ret = service.display(frozenset([frozenset("a"), frozenset("b")]))
    assert ret == (("b",), ("a",)) or ret == (("a",), ("b",))

    # tuple
    assert service.display(()) == ()
    assert service.display((1)) == (1)
    assert service.display((1, 2)) == (1, 2)

    # dict
    assert service.display({}) == {}
    assert service.display({1: "bla", 3: []}) == {1: "bla", 3: []}

    # bytearray
    assert service.display(bytearray("lol", encoding="ascii")) == "lol"

    # buffer (not implemented)
    try:
        import sys
        if sys.version >= '3':
            service.display(memoryview("lol".encode()))
        else:
            service.display(buffer("lol"))
    except RuntimeError:
        pass  # OK

    time.sleep(0.01)
    s.close()


def test_object_types():
    local = "tcp://127.0.0.1:0"
    sd = qi.Session()
    sd.listenStandalone(local)

    s = qi.Session()
    s.connect(sd.endpoints()[0])

    m = TestService()
    s.registerService("TestService", m)
    service = s.service("TestService")

    # new style
    class A(object):
        pass
    obj = A()

    service.display(A)
    service.display(obj)

    # old style
    class Aold:
        pass
    objold = Aold()

    try:
        service.display(Aold)
    except RuntimeError:
        pass

    service.display(objold)


def test_qi_object_instance():
    local = "tcp://127.0.0.1:0"
    sd = qi.Session()
    sd.listenStandalone(local)

    s = qi.Session()
    s.connect(sd.endpoints()[0])

    m = s.service("ServiceDirectory")
    assert qi.typeof(m) == qi.Object
    assert qi.typeof(m) == qi.Object()
    assert qi.isinstance(m, qi.Object)
    assert qi.isinstance(m, qi.Object())


def test_type():
    assert qi.Int8 == qi.Int8
    assert qi.Int8() == qi.Int8()
    assert qi.Int8 == qi.Int8()
    assert qi.Int8() == qi.Int8
    with pytest.raises(Exception):
        assert qi.Map != qi.Int8
    with pytest.raises(Exception):
        assert qi.List != qi.List(qi.Int8)
    assert qi.List(qi.Int8) == qi.List(qi.Int8)
    assert qi.List(qi.Int8()) == qi.List(qi.Int8())
    assert qi.List(qi.Int8()) == qi.List(qi.Int8)
    assert qi.List(qi.Int8) == qi.List(qi.Int8())
    assert qi.Object != qi.Int8
    assert qi.Object != qi.Int8()
    assert qi.Object != qi.Int32()
    assert qi.Int8() != qi.UInt8()
    assert (qi.Int8() != qi.Int8()) == False
    assert (qi.Int8 != qi.Int8) == False
    assert (qi.Int8 != qi.Int8()) == False
    assert (qi.Int8() != qi.Int8) == False


def main():
    test_throwing_callback()
    test_unicode_strings()
    test_builtin_types()
    test_object_types()
    test_qi_object_instance()
    test_type()

if __name__ == "__main__":
    main()
