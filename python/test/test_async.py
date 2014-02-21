#! /usr/bin/python2

import qi

def add(a, b):
    return a + b

def fail():
    assert(False)

def err():
    raise RuntimeError("sdsd")


def test_async_fun():
    f = qi.async(add, 21, 21)
    assert(f.value() == 42)


def test_async_error():
    f = qi.async(err)
    assert(f.hasError() == True)
    assert(f.error().startswith("RuntimeError: sdsd"))

class Adder:
    def __init__(self):
        self.v = 0

    def add(self, a):
        self.v += a
        return self.v

    def val(self):
        return self.v

def test_async_meth():
    ad = Adder()
    f = qi.async(ad.add, 21)
    assert(f.value() == 21)
    f = qi.async(ad.add, 21)
    assert(f.value() == 42)
    f = qi.async(ad.val)
    assert(f.value() == 42)

def test_async_delay():
    f = qi.async(add, 21,  21, delay=1000)
    assert(f.value() == 42)

result = 0
import time
def test_periodic_task():
    t = qi.PeriodicTask()
    def add():
        global result
        result += 1
    t.setCallback(add)
    t.setUsPeriod(1000)
    t.start(True)
    time.sleep(1)
    t.stop()
    assert result > 5 #how to find 5: plouf plouf plouf
    cur = result
    time.sleep(1)
    assert cur == result

def test_async_cancel():
    f = qi.async(fail, delay=1000000)
    f.cancel()
    f.wait()
    assert(f.isFinished())
    assert(not f.hasError())
    assert(f.isCanceled())

def main():
    test_async_fun()
    test_async_error()
    test_async_meth()
    test_async_delay()
    test_periodic_task()
    test_async_cancel()

if __name__ == "__main__":
    main()
