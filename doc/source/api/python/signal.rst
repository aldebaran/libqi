.. _api-py-signal:

qi.Signal API
*************

Introduction
============

Signal allows communication between threads. One thread emits events, other threads register callback to the signal, and process the events appropriately.

.. warning::

   In python services, signals must be created before register the service to be advertised.

A type can be specified in the constructor of the signal, otherwise any value are supported.

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
