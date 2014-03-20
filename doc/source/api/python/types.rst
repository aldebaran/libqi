.. _api-py-types:

qi.Types* API
*************

Introduction
============

All types referenced here are mostly useful in combination :py:func:`qi.bind`.

See the documentation of :py:func:`qi.bind` to understand how to use them.

Reference
=========

.. autoclass:: qi.Int8

.. autoclass:: qi.Int16

.. autoclass:: qi.Int32

.. autoclass:: qi.Int64

.. autoclass:: qi.UInt8

.. autoclass:: qi.UInt16

.. autoclass:: qi.UInt32

.. autoclass:: qi.UInt64

.. autoclass:: qi.Float

.. autoclass:: qi.Double

.. autoclass:: qi.String

.. autoclass:: qi.List

.. autoclass:: qi.Map

.. autoclass:: qi.Struct

.. autoclass:: qi.Object

.. autoclass:: qi.Dynamic

.. autoclass:: qi.Buffer

.. autoclass:: qi.AnyArguments

.. autofunction:: qi.typeof

.. autofunction:: qi.isinstance

Example
=======

Know if a variable is a qi.Object:

if isinstance(x, qi.Object):
  print("Yes it is!")
