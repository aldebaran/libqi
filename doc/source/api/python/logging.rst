.. _api-py-logging:

qi.logging API
**************

Introduction
============

This module provides logging capabilities integrated with the qi sdk.

.. warning::

  The debug log level is not available in python, because there is no debug build in python.
  Use verbose instead.

There are two ways to use this module:

  - class :ref:`qi.Logger <api-py-logging-class>` with defined category
  - :ref:`static methods <api-py-logging-static>` without defined category


.. _api-py-logging-class:


Class qi.Logger Reference
=========================

.. autoclass:: qi.Logger
   :members:

Example
=======

With a defined category:

.. code-block:: python

  import qi

  mylogger = qi.Logger("myfoo.bar")

  mylogger.warning("my log message")


.. _api-py-logging-static:

Static Methods Reference
========================

.. autofunction:: qi.fatal

.. autofunction:: qi.error

.. autofunction:: qi.warning

.. autofunction:: qi.info

.. autofunction:: qi.verbose

Deprecated Methods
==================

.. function:: qi.getLogger

   .. deprecated:: 2.0.1

      Please use :py:func:`qi.Logger` instead

.. function:: qi.logFatal

   .. deprecated:: 2.0.1

      Please use :py:func:`qi.fatal` instead

.. function:: qi.logError

   .. deprecated:: 2.0.1

      Please use :py:func:`qi.error` instead

.. function:: qi.logWarning

   .. deprecated:: 2.0.1

      Please use :py:func:`qi.warning` instead

.. function:: qi.logInfo

   .. deprecated:: 2.0.1

      Please use :py:func:`qi.info` instead

.. function:: qi.logVerbose

   .. deprecated:: 2.0.1

      Please use :py:func:`qi.verbose` instead

.. function:: qi.logDebug

   .. deprecated:: 2.0.1

      Please use :py:func:`qi.verbose` instead


Example
=======

Simple example:

.. code-block:: python

  import qi

  qi.warning("myfoo.bar", "my log message")

Simple example with a category:

.. code-block:: python

  import qi

  log = qi.Logger("myfoo.bar")
  log.warning("my log message")


Reference
=========

Global methods

.. autofunction:: qi.logging.setFilters

.. autofunction:: qi.logging.setLevel

.. autofunction:: qi.logging.setContext
