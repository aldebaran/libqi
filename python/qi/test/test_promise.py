#!/usr/bin/env python
##
## Author(s):
##  - Cedric GESTES <gestes@aldebaran-robotics.com>
##  - Vincent Barbaresi <vbarbaresi@aldebaran-robotics.com>
##
## Copyright (C) 2013 Aldebaran Robotics

import time
import threading

from qi import Promise

def waiterSetValue(promise, waiter):
    #time.sleep(t)
    waiter.wait()
    try:
        promise.setValue("mjolk")
    except:
        pass

def waitSetValue(p, t=0.01):
    time.sleep(t)
    p.setValue("mjolk")

def test_many_futures_create():
    def wait(p):
        time.sleep(1)
    p = Promise(wait)
    fs = [p.future() for _ in range(100)]
    p.setValue(1337)
    for f in fs:
        assert f.hasValue()
        assert f.value() == 1337

def test_future_wait():
    p = Promise()
    f = p.future()
    threading.Thread(target=waitSetValue, args=[p, 0.1]).start()
    assert f.isFinished() is False
    assert f.value() == "mjolk"
    assert f.isFinished() is True


def test_many_futures_wait_cancel():
    def cancel(p):
        try:
            p.setValue("Kappa")
        except:
            pass #ok: cancel called many times

    waiter = Promise();
    ps = [Promise(cancel) for _ in range(50)]
    fs = [p.future() for p in ps]
    for p in ps:
        threading.Thread(target=waiterSetValue, args=[p, waiter.future()]).start()
    # Cancel only one future
    fs[25].cancel()
    waiter.setValue(None)

    for i, f in enumerate(fs):
        if i == 25:
            assert f.value() == "Kappa"
        else:
            assert f.value() == "mjolk"


def test_many_promises_wait_cancel():
    def cancel(p):
        try:
            p.setValue("Kappa")
        except:
            pass #ok: cancel called many times

    waiter = Promise();
    ps = [Promise(cancel) for _ in range(50)]
    fs = [p.future() for p in ps]
    for p in ps:
        threading.Thread(target=waiterSetValue, args=[p, waiter.future()]).start()
    # Cancel only one promise
    ps[25].setCanceled()
    waiter.setValue(None)
    for i, f in enumerate(fs):
        if i == 25:
            try:
                f.value()
            except RuntimeError:
                pass
        else:
            assert f.value() == "mjolk"


def test_future_no_timeout():
    p = Promise()
    f = p.future()
    threading.Thread(target=waitSetValue, args=[p]).start()
    # 1sec to be secure
    assert f.value(timeout=1000) == "mjolk"
    assert f.hasError() is False


def test_future_timeout_immediate():
    p = Promise()
    f = p.future()
    threading.Thread(target=waitSetValue, args=[p, 1]).start()
    try:
        f.value(timeout=0)
    except RuntimeError:
        pass


def test_future_timeout():
    p = Promise()
    f = p.future()
    threading.Thread(target=waitSetValue, args=[p, 1]).start()
    try:
        # 10ms - 3ms
        f.value(timeout=8)
    except RuntimeError:
        pass
    assert f.hasError() is False


def test_future_error():
    p = Promise()
    p.setError("woops")
    f = p.future()
    assert f.hasError() is True
    assert f.error() == "woops"
    try:
        f.value()
    except RuntimeError:
        pass


def test_future_cancel_exception():
    def throw(promise):
        time.sleep(0.01)
        raise Exception("plop")

    p = Promise(throw)
    f = p.future()
    f.cancel()


called = False


def test_future_callback():

    result = Promise()

    def callback(f):
        global called
        assert f.isRunning() is False
        assert f.value() == 1337
        assert called is False
        called = "aight"
        result.setValue("bim")

    p = Promise()
    f = p.future()
    f.addCallback(callback)
    p.setValue(1337)
    result.future().wait(1000)
    assert result.future().hasValue(0)
    assert called == "aight"
    assert not f.isCanceled()
    assert f.isFinished()


called1, called2 = "", ""
def test_future_two_callbacks():

    result1 = Promise()
    result2 = Promise()
    def callback1(f):
        global called1
        called1 = "1"
        result1.setValue("bim")

    def callback2(f):
        global called2
        called2 = "2"
        result2.setValue("bim")

    p = Promise()
    f = p.future()
    f.addCallback(callback1)
    f.addCallback(callback2)
    p.setValue(42)

    result1.future().wait(1000)
    result2.future().wait(1000)

    assert result1.future().hasValue(0)
    assert result2.future().hasValue(0)
    assert called1 == "1"
    assert called2 == "2"


def test_future_callback_noargs():
    def callback():
        pass
    p = Promise()
    f = p.future()
    f.addCallback(callback)
    p.setValue("segv?")
    assert not f.isCanceled()
    assert f.isFinished()


def test_promise_re_set():
    p = Promise()
    p.setValue(42)
    try:
        p.setValue(42)
    except RuntimeError:
        pass


def test_future_exception():
    p = Promise()
    f = p.future()

    def raising(f):
        raise Exception("oops")

    f.addCallback(raising)
    p.setValue(42)

# This test doesn't assert, it's only segfault check
# No segfault => no bug
def test_future_many_callback(nbr_fut = 10000):
    def callback(f):
        pass

    for _ in range(nbr_fut):
        p = Promise()
        f = p.future()
        f.addCallback(callback)
        p.setValue(0)

# This test is ok
import threading
def test_many_callback_threaded():
    nbr_threads = 100
    thr_list = list()
    for i in range(nbr_threads):
        thr = threading.Thread(target=test_future_many_callback, kwargs={"nbr_fut": 10})
        thr_list.append(thr)

    for i in range(nbr_threads):
        thr_list[i].start()

    for i in range(nbr_threads):
        thr_list[i].join()

    for i in range(nbr_threads):
        if thr_list[i].isAlive():
            print("IT IS ALIIIIVE: " + str(i))
    print("finish")


def main():
    test_many_futures_create()
    test_future_wait()
    test_many_futures_wait_cancel()
    test_many_promises_wait_cancel()
    test_future_no_timeout()
    test_future_timeout_immediate()
    test_future_timeout()
    test_future_error()
    test_future_cancel_exception()
    test_future_callback()
    test_promise_re_set()
    test_future_exception()
    test_future_two_callbacks()
    test_future_callback_noargs()
    test_future_many_callback()
    #test_many_callback_threaded()

if __name__ == "__main__":
    main()
