.. _api-py-logging:

qi.logging API
**************

Introduction
============

This module provide logging capabilities integrated with the qi sdk.

.. warning::

  The debug log level is not available in python, because there is no debug build in python.
  Use verbose instead.

Reference
=========

.. autoclass:: qi.Logger
   :members:

.. autofunction:: qi.fatal

.. autofunction:: qi.error

.. autofunction:: qi.warning

.. autofunction:: qi.info

.. autofunction:: qi.verbose

.. autofunction:: qi.logging.setFilters

.. autofunction:: qi.logging.setLevel

.. autofunction:: qi.logging.setContext


Examples
========

Simple example:

.. code-block:: python

  import qi

  qi.warning("myfoo.bar", "my log message")


With a defined category:

.. code-block:: python

  import qi

  mylogger = qi.Logger("foo.bar")

  mylogger.message("my log message")
