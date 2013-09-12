.. _api-py-session:

qi.Session API
==============

Introduction
------------

A session connect to a :py:class:`qi.ServiceDirectory`. Once connected the session can:

- advertise new services using :py:meth:`qi.Session.registerService`
- get proxy on services using :py:meth:`qi.Session.service`

Reference
---------

.. autoclass:: qi.Session
   :members:


Examples
--------

Getting a service:

.. code-block:: python

  import qi

  s = qi.Session()
  s.connect("tcp://127.0.0.1:9559")

  foo = s.service("Foo")

Registering a service:

.. code-block:: python

  import qi

  #sample service doing nothing
  class Foo:
    pass

  s = qi.Session()
  s.connect("tcp://127.0.0.1:9559")

  serviceId = s.registerService("Foo", Foo())
