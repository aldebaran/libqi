#! /usr/bin/python2

import qi

def add(a, b):
    return a + b


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

def main():
    test_async_fun()
    test_async_error()
    test_async_meth()
    test_async_delay()

if __name__ == "__main__":
    main()
