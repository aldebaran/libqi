#!/usr/bin/env python
##
## Author(s):
##  - Cedric GESTES <gestes@aldebaran-robotics.com>
##
## Copyright (C) 2013 Aldebaran Robotics

from qi import Future, Promise

def onError(promise):
  promise.set_error("paffff")

def onCancel(promise):
  promise.set_canceled()

def onValue(promise):
  promise.set_value("piffff")

def test_future():
  print "check simple future"
  prom = Promise()
  fut = prom.future()
  prom.set_value("titi")
  assert(fut.value() == "titi")
  assert(fut.is_finished())
  assert(not fut.is_canceled())

def test_future_cancel():
  #check error
  print "check cancel error"
  prom = Promise(onError)
  fut = prom.future()
  fut.cancel()
  assert(fut.is_finished())
  assert(not fut.is_canceled())
  assert(fut.has_error())
  assert(fut.error() == "paffff")

  print "check cancel value"
  #check value
  prom = Promise(onValue)
  fut = prom.future()
  fut.cancel()
  assert(fut.is_finished())
  assert(not fut.is_canceled())
  assert(not fut.has_error())
  assert(fut.has_value())
  assert(fut.value() == "piffff")

  print "check cancel canceled"
  #check cancel
  prom = Promise(onCancel)
  fut = prom.future()
  fut.cancel()
  assert(fut.is_finished())
  assert(fut.is_canceled())
  assert(not fut.has_error())

def main():
  test_future()
  test_future_cancel()

if __name__ == "__main__":
    main()
