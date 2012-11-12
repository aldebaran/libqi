libqitype's documentation
=========================

.. toctree::
   :hidden:

   api/type
   api/signature
   api/value
   api/object


Overview
--------

libqitype provide type erasure for most c++ types. It support all PODs, list, map, tuple, object, buffer and dynamic types.
It is our c++ implementation of a type system that provide everything needed for introspection and reflection of variable and objects.

The library has multiple purposes:

- type erasure
- type conversion
- type description/identification
- provide common type operations: assignement, method calls, etc..
- support codegeneration to avoid written cumbersome type description

We have faced multiples chalenges:

- The type system should be compatible with multiples langages. (at least c++, c, python, urbiscript, java)
- We need to catch as many errors as possible, at compile time if possible.
- The library should provide common hooks for introspection, serialization and reflection.


API Reference
-------------

* :ref:`qitype-type`
* :ref:`qitype-signature`
* :ref:`qitype-value`
* :ref:`qitype-object`


Additional informations
-----------------------

* how to register types
* how to generate code
* how to use type erasure

Indices and tables
------------------

* :ref:`genindex`
* :ref:`modindex`
* :ref:`search`
