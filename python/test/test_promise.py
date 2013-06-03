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
from qi import Future


def wait(promise, t=0.1):
    time.sleep(t)
    promise.set_value("mjolk")


def test_empty_future_value():
    f = Future()
    try:
        f.value()
    except RuntimeError:
        pass


def test_many_futures_create():
    def wait(p):
        time.sleep(1)
    p = Promise(wait)
    fs = [p.future() for _ in range(100)]
    p.set_value(1337)
    for f in fs:
        assert f.has_value()
        assert f.value() == 1337


def test_future_wait():
    def wait(promise):
        time.sleep(0.1)
        promise.set_value("lol")

    p = Promise()
    f = p.future()
    threading.Thread(target=wait, args=[p]).start()

    assert f.is_finished() is False
    assert f.value() == "lol"
    assert f.is_finished() is True


def test_many_futures_wait_cancel():

    def cancel(p):
        p.set_value("Kappa")

    ps = [Promise(cancel) for _ in range(50)]
    fs = [p.future() for p in ps]
    for p in ps:
        threading.Thread(target=wait, args=[p]).start()
    # Cancel only one future
    fs[25].cancel()

    for i, f in enumerate(fs):
        if i == 25:
            assert f.value() == "Kappa"
        else:
            assert f.value() == "mjolk"


def test_many_promises_wait_cancel():

    def cancel(p):
        p.set_value("Kappa")

    ps = [Promise(cancel) for _ in range(50)]
    fs = [p.future() for p in ps]
    for p in ps:
        threading.Thread(target=wait, args=[p]).start()
    # Cancel only one promise
    ps[25].set_canceled()

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
    threading.Thread(target=wait, args=[p, 0.01]).start()
    # 10ms + 3ms
    assert f.value(timeout=12) == "mjolk"
    assert f.has_error() is False


def test_future_timeout_immediate():
    p = Promise()
    f = p.future()
    threading.Thread(target=wait).start()
    try:
        f.value(timeout=0)
    except RuntimeError:
        pass


def test_future_timeout():
    p = Promise()
    f = p.future()
    threading.Thread(target=wait, args=[p, 0.01]).start()
    try:
        # 10ms - 3ms
        f.value(timeout=8)
    except RuntimeError:
        pass
    assert f.has_error() is False


def test_future_error():
    p = Promise()
    p.set_error("woops")
    f = p.future()
    assert f.has_error() is True
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

    def callback(f):
        global called
        assert f.is_running() is False
        assert f.value() == 1337
        assert called is False
        called = "aight"

    p = Promise()
    f = p.future()
    f.add_callback(callback)
    p.set_value(1337)
    assert called == "aight"
    assert not f.is_canceled()
    assert f.is_finished()


called1, called2 = "", ""
def test_future_two_callbacks():

    def callback1(f):
        global called1
        called1 = "1"

    def callback2(f):
        global called2
        called2 = "2"

    p = Promise()
    f = p.future()
    f.add_callback(callback1)
    f.add_callback(callback2)
    p.set_value(42)

    assert called1 == "1"
    assert called2 == "2"


def test_future_callback_noargs():
    def callback():
        pass
    p = Promise()
    f = p.future()
    f.add_callback(callback)
    p.set_value("segv?")
    assert not f.is_canceled()
    assert f.is_finished()


def test_promise_re_set():
    p = Promise()
    p.set_value(42)
    try:
        p.set_value(42)
    except RuntimeError:
        pass


def test_future_exception():
    p = Promise()
    f = p.future()

    def raising(f):
        raise Exception("oops")

    f.add_callback(raising)
    p.set_value(42)


def main():
    test_empty_future_value()

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

if __name__ == "__main__":
    main()
