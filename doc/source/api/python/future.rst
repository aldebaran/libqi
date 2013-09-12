.. _api-py-future:

qi.Future API
*************

Introduction
============

Promise and Future are a way to synchronise data between multiples threads.
The number of future associated to a promise is not limited.
Promise is the setter, future is the getter.
Basically, you have a task to do, that will return a value, you give it a Promise.
Then you have others thread that depend on the result of that task, you give them future
associated to that promise, the future will block until the value or an error is set
by the promise.


Reference
=========

.. py:data:: qi.FutureState

  Constants that describe the state of the Future.
  This is the status returned by :py:meth:`qi.Future.wait`

  ================================    ====================================================
  qi.FutureState.None                 The future is not bound.
  qi.FutureState.Running              The future is attached to a Promise.
  qi.FutureState.Canceled             The future has been canceled
  qi.FutureState.FinishedWithError    The future has finished and a value is available
  qi.FutureState.FinishedWithValue    The future has finished and an error is available
  ================================    ====================================================


.. py:data:: qi.FutureTimeout

  Constants to use for timeout arguments.

  ================================    ====================================================
  qi.FutureTimeout.None               Do not wait.
  qi.FutureTimeout.Infinite           Block forever
  ================================    ====================================================

.. autoclass:: qi.Promise
   :members:

.. autoclass:: qi.Future
   :members:

Examples
========

Simple example:

.. code-block:: python

  import qi
  import time

  def doSomeWork(p):
    #do your work here instead of sleeping
    time.sleep(1)
    p.setValue(42)

  p = qi.Promise()
  f = p.future()
  threading.Thread(target=doSomeWork, args=[p]).start()
  print "result:", f.value()

With callback:

.. code-block:: python

  import qi
  import time

  def doSomeWork(p):
    #do your work here instead of sleeping
    time.sleep(1)
    p.setValue(42)

  def resultReady(f):
    if f.hasValue():
      print "Value:", f.value()
    elif f.hasError():
      print "Error:", f.error()

  p = qi.Promise()
  f = p.future()
  threading.Thread(target=doSomeWork, args=[p]).start()

  #resultReady will be called even if the result is already there.
  f.addCallback(resultReady)
