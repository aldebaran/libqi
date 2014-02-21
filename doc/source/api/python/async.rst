.. _api-py-async:

qi.Async API
************

Introduction
============

:py:func:`qi.async` and :py:class:`qi.PeriodicTask` allow managing concurrent tasks.

Reference
=========

.. autofunction:: qi.async

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

Calling tts.Say in 42 seconds.

.. code-block:: python

  import qi

  #assume we have a connected session
  tts = session.service("ALTextToSpeech")

  fut = qi.async(tts.say, "42 seconds elapsed", delay=42000000)

  #wait for the sentence to be said
  fut.wait()
