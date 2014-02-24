.. _api-py-bind:

Bind API
********

Introduction
============

:py:class:`qi.bind` allow specifying types for bound methods.
By default methods parameters are of type `AnyValue` and always return an AnyValue.

With qi.bind you can specify which types the function really accept and return.

Reference
=========

.. autoclass:: qi.bind
  :members:

Examples
========

How to have two functions with the same name?
---------------------------------------------

This works for methods with different arguments types, or with different arguments count.

.. code-block:: python

  class MyFoo:

    @qi.bind(paramsType=(qi.Int32,) , methodName="bar")
    def bar1(self, arg):
      pass

    @qi.bind(paramsType=(qi.String,) , methodName="bar")
    def bar2(self, arg):
      pass


How to specify the return type of a method?
-------------------------------------------

.. code-block:: python

  class MyFoo:

    @qi.bind(returnType=qi.String)
    def bar(self, arg):
      pass


How to specify the arguments types of a method?
-----------------------------------------------

.. code-block:: python

  class MyFoo:

    #this function take a string and an int. all others arguments types
    #will be rejected even before calling the method.
    @qi.bind(paramsType=(qi.String, qi.Int32))
    def bar(self, arg1, arg2):
      pass
