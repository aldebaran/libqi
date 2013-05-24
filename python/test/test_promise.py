## Copyright (C) 2013 Aldebaran Robotics

import pytest

import time
import threading

from qi import Promise
from qi import Future


def wait(promise, t=0.1):
    time.sleep(t)
    promise.set_value("mjolk")


@pytest.mark.skipif(reason="crash")
def test_empty_future_value():
    f = Future()
    f.value()


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
            with pytest.raises(RuntimeError):
                f.value()
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
    with pytest.raises(RuntimeError):
        f.value(timeout=0)


def test_future_timeout():
    p = Promise()
    f = p.future()
    threading.Thread(target=wait, args=[p, 0.01]).start()
    with pytest.raises(RuntimeError):
        # 10ms - 3ms
        f.value(timeout=8)
    assert f.has_error() is False


def test_future_error():
    p = Promise()
    p.set_error("woops")
    f = p.future()
    assert f.has_error() is True
    assert f.error() == "woops"


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
        assert f.value() == 1337
        assert called is False
        called = not called

    p = Promise()
    f = p.future()
    f.add_callback(callback)
    p.set_value(1337)
    assert called is True


def test_promise_re_set():
    p = Promise()
    p.set_value(42)
    with pytest.raises(RuntimeError):
        p.set_value(42)


def test_future_exception():
    p = Promise()
    f = p.future()

    def raising(f):
        raise Exception("oops")

    f.add_callback(raising)
    p.set_value(42)


if __name__ == "__main__":
    #test_empty_future_value()

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
