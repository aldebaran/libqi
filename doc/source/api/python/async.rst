.. _api-py-async:

qi.async, qi.PeriodicTask API
*****************************

Introduction
============

:py:func:`qi.async` and :py:class:`qi.PeriodicTask` allow managing concurrent tasks.

Use of these should be prefered over classic *threading.Thread* or *threading.Timer* operations.

Thread creation and destruction are low-level operations that take resources for context switching
each time they happen.

Using qi operations will let the system manage the low level operations and improve its
overall performance.

qi.async
=========

Launch an operation in a concurrent task.

The qi.future returned can be used to interact with the operation. (cancel, get return value ...)

.. autofunction:: qi.async

qi.PeriodicTask
===============

Execute an operation periodically and asynchronously.

By default, we do not compensate the callback time.
The period will be constant between the end of a call and the beginning of another.

.. autoclass:: qi.PeriodicTask
   :members:


Examples
========

Doing something in 2 seconds and getting the result.

.. code-block:: python

  import qi

  def getAnswerToLifeAndUniverse(a, b)
    return a + b

  fut = qi.async(getAnswerToLifeAndUniverse, 40, 2, delay=2000000)
  #do work while the result is being processed
  print("Result:", fut.value())

Calling tts.say in 42 seconds.

.. code-block:: python

  import qi

  #assume we have a connected session
  tts = session.service("ALTextToSpeech")

  fut = qi.async(tts.say, "42 seconds elapsed", delay=42000000)

  #wait for the sentence to be said
  fut.wait()

Calling tts.say every 10 seconds.

.. code-block:: python

  import qi
  import time
  import functools

  #assume we have a connected session

  tts = session.service("ALTextToSpeech")
  sayHelloCallable = functools.partial(tts.say, "hello")

  sayHelloTask = qi.PeriodicTask()
  sayHelloTask.setCallback(sayHelloCallable)
  sayHelloTask.setUsPeriod(10000000)
  sayHelloTask.start(True)

  time.sleep(30)
  sayHelloTask.stop()

Canceling a delayed operation before its execution.

.. code-block:: python

  import qi
  import time

  def dummyAction():
    # do your operations here
    qi.info("async example", "My dummy action is over")

  action = qi.async(dummyAction, delay=15000000)
  time.sleep(5)
  action.cancel()
  action.wait()
  if action.isCanceled():
    qi.info("async example", "dummyAction was canceled as expected")



Tips
====

Standard choregraphe boxes "Wait" and "Timer" will show you possible usages
of qi.async and qi.PeriodicTask in choregraphe.
