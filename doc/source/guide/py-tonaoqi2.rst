.. _guide-py-tonaoqi2:

How to switch from NAOqi1 to NAOqi2
***********************************

Introduction
============

This guide will help you switch to NAOqi2.
It describes how to access the NAOqi2 world from NAOqi1 and how to take advantage of NAOqi2.

NAOqi2 main features
====================

The main new features of NAOqi2 are:

- support for map, list, object and structure
- support for signal and property
- it's no longer needed to be a module to subscribe to an event
- asynchronous call with result value

Getting a Session from a proxy
==============================

This section explains how to get a :py:class:`qi.Session` from naoqi1.

This can be used in NAOqi1 script or in Choregraphe behaviors.


.. code-block:: python

  import naoqi

  #imagine you have a NAOqi1 proxy on almemory
  mem = naoqi.ALProxy("ALMemory", "127.0.0.1", 9559)

  #get a NAOqi2 session
  ses = mem.session()

  #then you can use the session like you want


Subscribe to a memory event
===========================

When using NAOqi2 you do not have to be a module to subscribe to a memory event, you
just have to specify a callback.

.. warning::
  You have to keep the SignalSubscriber returned by ALMemory.Subscriber to keep the callback
  being called.

.. code-block:: python

  def mycallback(value):
    print "val:", value

  mem = session.service("ALMemory")
  sub = mem.subscriber("foo/bar")
  sub.signal.connect(mycallback)


If you want to have the key in your callback you can use functools.partial.

.. code-block:: python

  import functools

  def mycallback2(key, value):
    print "key:", key
    print "val:", value

  mem = session.service("ALMemory")
  sub = mem.subscriber("foo/bar")
  sub.signal.connect(functools.partial(mycallback2, "foo/bar"))



PCall vs qi.async
=================

NAOqi2 modules do not support pCall but they do support a more generic asynchronous mechanism :py:func:`qi.async`. If you want your call to be asynchronous
you can use :py:func:`qi.async` which returns a :py:class:`qi.Future`. That :py:class:`qi.Future` allow you to object the resulting value or error.

However some NAOqi1 modules really depend on pCall. (most notably tts.say and motion.moveTo). In this case you can use the pCall method provided by ALModule. It will behave the same as the old way and return a pcallId usable with the stop and the wait method of ALModule.

.. warning::
  the pCall method is only available on ALModule, not on NAOqi2 services. So it's availability depends on the service you are calling.


Let's see an example with pCall:

.. code-block:: python

  tts = session.service("ALTextToSpeech")
  pCallId = tts.pCall("say", "I love rock'n'pCall")
  tts.stop(pcallId) #oops that was not what I meant
  #let's try again
  pCallId = tts.pCall("say", "I love rock'n'roll")


Now let's imagine you want to call a function and do something when the function finishes:

.. code-block:: python

   tts = session.service("ALTextToSpeech")
   fut = qi.async(tts.say, "I Love rock'n'sync")
   #do some work here

   #either wait and get the value
   print "value:", fut.value()

   #or add a callback that will be called when the function returns
   def mycb(fut):
     print "value:", fut.value()
   fut.add_callback(mycb)
   #continue working while tts.say proceeds
