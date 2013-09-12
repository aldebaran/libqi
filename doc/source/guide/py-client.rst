.. _guide-py-client:

How to write a qimessaging client in Python
===========================================

Introduction
-------------

This guide will teach you how to write a qimessaging client in Python, to
interact with the various services offered by the robot.

Prerequisites
-------------

- An installed python NAOqi SDK for your operating system.

Creating a session
------------------

The first step to interact with qimessaging services is to connect a
:py:class:`qi.Session` to the _`Service Directory` of the robot.
Then to get a proxy on the service you are interested in.

Let's assume we want to get the ALTextToSpeech service.

.. code-block:: python

  import qi

  s = qi.Session()
  s.connect("tcp://127.0.0.1:9559")
  tts = s.service("ALTextToSpeech")


Calling a method on a service
-----------------------------

Let us make the robot speak by using the *say* method of the *ALTextToSpeech* service.

.. code-block:: python

  import qi

  s = qi.Session()
  s.connect("tcp://127.0.0.1:9559")
  tts = s.service("ALTextToSpeech")
  tts.say("Hello Word")

Making asynchronous calls using qi::async
-----------------------------------------

Most of the API are designed to block until the operation started by the call
completes. For example with this code inside our main::

  tts = session.service("ALTextToSpeech");
  motion = session.service("ALMotion");
  tts->say("This is a very very very very long sentence.");
  motion->moveTo(1, 0, 0); // go forward one meter

The robot will only start moving when he finishes speaking.

To perform both actions simultaneously, the API provides :py:func:`qi.async` that
performs a call in an asynchronous manner, and notifies you when the call
finishes using a _`qi.Future`::

  tts = session.service("ALTextToSpeech");
  motion = session.service("ALMotion");
  sayOp = tts->async("say", "This is a very very very very long sentence.");
  moveOp = motion->async("moveTo", 1, 0, 0);
  // Wait for both operations to terminate.
  sayOp.wait();
  moveOp.wait();

Look at the :py:class:`qi.Future` for more complete documentation, but here is what you
most definitely need to know:

- If the method throws an exception, it is stored in the *Future*, and can be
  accessed using *error()*.
- Use *wait()* to wait for the future to complete. It can accept a timeout duration
  as argument, and will return the state of the future.
- Use *value()* and *error()* to get the stored value or error.
- You can regiter a callback to be notified when the future finishes with
  *qi::Future::connect()*.


Using signal
------------

Using properties
---

passing an object as argument
-----------------------------


Generic api
-----------

If you wish to use a service for which no interface is available, but for
which you know the API, you can use the _`qi::AnyObject` generic API made
available throug *qi::Object<Empty>*::

  qi::Object<Empty> obj = session.service("ALTextToSpeech");
  obj.call("say", "Hello once more.");

Methods are also provided to emit and connect signals, read/write properties,
and access the service API.
