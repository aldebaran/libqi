#!/usr/bin/env python2
##
## Author(s):
##  - Cedric GESTES <gestes@aldebaran-robotics.com>
##
## Copyright (C) 2013 Aldebaran Robotics

import qi
import time
import threading
import pytest

def setValue(p, v):
    time.sleep(0.2)
    p.setValue(v)

class FooService:
    def __init__(self):
        pass

    def simple(self):
        return 42

    def vargs(self, *args):
        return args

    def vargsdrop(self, titi, *args):
        return args

    @qi.nobind
    def hidden(self):
        pass

    @qi.bind(qi.Dynamic, qi.AnyArguments)
    def bind_vargs(self, *args):
        return args

    @qi.bind(qi.Dynamic, (), "renamed")
    def oldname(self):
        return 42

    @qi.bind(qi.Int64, (qi.Int32, qi.Int32))
    def add(self, a, b):
        return a + b

    @qi.bind(None, None)
    def reta(self, a):
        return a

    def retfutint(self):
        p = qi.Promise()
        t = threading.Thread(target=setValue, args=(p, 42, ))
        t.start()
        return p.future()

    @qi.bind(qi.Int32)
    def bind_retfutint(self):
        p = qi.Promise("(i)")
        t = threading.Thread(target=setValue, args=(p, 42, ))
        t.start()
        return p.future()

    def retfutmap(self):
        p = qi.Promise()
        t = threading.Thread(target=setValue, args=(p, { 'titi' : 'toto', "foo" : "bar" },))
        t.start()
        return p.future()

    @qi.bind(qi.Int32())
    def retc(self, name, index):
        return name[index]

def docalls(sserver, sclient):
    m = FooService()
    sserver.registerService("FooService", m)
    s = sclient.service("FooService")

    print("simple test")
    assert s.simple() == 42

    print("vargs")
    assert s.vargs(42) == (42,)
    assert s.vargs("titi", "toto") == ("titi", "toto",)

    print("vargs drop")
    assert s.vargsdrop(4, 42) == (42,)

    print("hidden")
    try:
        s.hidden()
        assert False
    except:
        pass

    print("bound methods")
    assert s.bind_vargs(42) == (42,)
    assert s.bind_vargs("titi", "toto") == ("titi", "toto",)

    print("renamed")
    assert s.renamed() == 42

    print("test types restrictions")
    assert s.add(40, 2) == 42
    try:
        s.add("40", "2")
        assert False
    except:
        pass

    print("test future")
    assert s.retfutint() == 42
    print("test bound future")
    assert s.bind_retfutint() == 42

    print("test future async")
    assert s.retfutint(_async=True).value() == 42
    print("test bound future async")
    assert s.bind_retfutint(_async=True).value() == 42

    print("test future async")
    assert qi.async(s.retfutint).value() == 42
    print("test bound future async")
    assert qi.async(s.bind_retfutint).value() == 42

    print("test future map")
    assert s.retfutmap() == { 'titi' : 'toto', "foo" : "bar" }

    print("test future map async")
    fut = s.retfutmap(_async=True)
    assert fut.hasValue() == True
    assert fut.value() == { 'titi' : 'toto', "foo" : "bar" }



def test_calldirect():
    ses = qi.Session()
    ses.listenStandalone("tcp://127.0.0.1:0")
    #MODE DIRECT
    print("## DIRECT MODE")
    try:
        docalls(ses, ses)
    finally:
        ses.close()

def test_callsd():
    sd = qi.Session()
    try:
        sd.listenStandalone("tcp://127.0.0.1:0")
        local = sd.endpoints()[0]

        #MODE NETWORK
        print("## NETWORK MODE")
        ses = qi.Session()
        ses2 = qi.Session()
        try:
            ses.connect(local)
            ses2.connect(local)
            docalls(ses, ses2)
        finally:
            ses.close()
            ses2.close()
    finally:
        sd.close()



class Invalid1:
    def titi():
        pass

def test_missingself():
    sd = qi.Session()
    try:
        sd.listenStandalone("tcp://127.0.0.1:0")
        local = sd.endpoints()[0]

        print("## TestInvalid (missing self)")
        ses = qi.Session()
        ses.connect(local)
        i = Invalid1()
        with pytest.raises(Exception):
            ses.registerService("Invalid1", i)
    finally:
        ses.close()
        sd.close()

class Invalid2:
    @qi.bind(42)
    def titi(self, a):
        pass

def test_badbind():
    sd = qi.Session()
    try:
        sd.listenStandalone("tcp://127.0.0.1:0")
        local = sd.endpoints()[0]

        print("## TestInvalid (bind: bad return value)")
        ses = qi.Session()
        ses.connect(local)
        i = Invalid2()
        with pytest.raises(Exception):
            ses.registerService("Invalid2", i)
    finally:
        ses.close()
        sd.close()

class Invalid3:
    @qi.bind(qi.Float, [42])
    def titi(self, a):
        pass

def test_badbind2():
    sd = qi.Session()
    try:
        sd.listenStandalone("tcp://127.0.0.1:0")
        local = sd.endpoints()[0]

        print("## TestInvalid (bind: bad params value)")
        ses = qi.Session()
        ses.connect(local)
        i = Invalid3()
        with pytest.raises(Exception):
            ses.registerService("Invalid3", i)
    finally:
        ses.close()
        sd.close()

def main():
    test_calldirect()
    test_callsd()
    test_missingself()
    test_badbind()
    test_badbind2()

if __name__ == "__main__":
    main()
