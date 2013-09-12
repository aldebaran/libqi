.. _api-py-signal:

qi.Signal API
*************

Introduction
============

Signal allow communication between thread. One thread emit events, other threads register callback to the signal,
and process the events appropriately.

Reference
=========

.. autoclass:: qi.Signal
   :members:


   .. py:method:: (*args)

      trigger the signal. for example:

      .. code-block:: python

        s = Signal()
        s(42)
        s(42, 43)


Examples
========

.. code-block:: python

  import qi

  def onSignal(value):
    print "signal value:", value

  s = qi.Signal()

  s.connect(onSignal)
  #trigger the signal
  s(42)
