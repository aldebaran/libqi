.. _qitype-signature:

Signature API
=============

Overview
--------

- qi::Signature


Each object have a set of methods. Methods have parameters. Parameters are represented by a AnyValue. To call a method you need to be sure that for each parameter the qi::Type is the same for the method parameter and the effective payload.

Effective Signature
-------------------

Generate an effective signature starting from the type and the value. The goals is to remove the ambiguity introduced by dynamics types.

- convert AnyValue to the "runtime" real type. For a Value containing a string we will have "m" -> "s".
- convert homogenous container to the real type. For a vector<value> only containing string we will have "[m]" -> "[s]"

.. note::

   what should we do with small heterogeneous list? and what about homogenous tuple?
   I think it's the isCompatible that should handle those cases.

Signature isCompatible
----------------------

We compare if signature are compatible.

- fondamental types are compatible togethers.
- all types can be converted to a dynamic
