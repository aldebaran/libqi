.. _guide-py-service:

How to write a qimessaging service in Python
********************************************

Introduction
============

This guide will teach you how to write a qimessaging service in Python, and advertise
it to others.

Prerequisites
-------------

- An installed python NAOqi SDK for your operating system.

Creating a service
------------------

We are going to create simple service that raise a signal when it's method bang is called.

Let's declare a simple class to do that.

.. code-block:: python

  import qi

  class MyFooService:
    def __init__(self, *args, **kwargs):
      #define a signal 'onBang'
      self.onBang = qi.Signal()

    #define a bang method that will trigger the onBang signal
    def bang(self):
      #trigger the signal with 42 as value
      self.onBang(42)


Creating a session
------------------

To make the service available to others we need a :py:class:`qi.Session`.

Let's create a :py:class:`qi.Session` and connect it to a ServiceDirectory, then let's register our MyFooService created just above under the name 'foo'.

.. code-block:: python

  import qi

  #create an application
  app = qi.Application()

  #create an instance of MyFooService
  myfoo = MyFooService()

  s = qi.Session()
  s.connect("tcp://127.0.0.1:9559")
  #let's register our service with the name "foo"
  id = s.registerService("foo", myfoo)

  #let the application run
  app.run()

Test the service
----------------

We are going to create a simple client to call the service and react to it.

.. code-block:: python

  import qi

  def onBangCb(i):
    print "bang:", i

  s = qi.Session()
  s.connect("tcp://127.0.0.1:9559")
  foo = s.service("foo")
  #register a callback on 'onBang'
  foo.onBang.connect(onBangCb)
  #call bang
  foo.bang()
