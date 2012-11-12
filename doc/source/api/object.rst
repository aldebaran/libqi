.. _qitype-object:

Object API
==========

Overview
--------

GenericObject::Metacall, allow calling a method without knowing it's type. It's based on erased type found in libqitype.

To manipulate objects you can use GenericObject.

Method calls
------------

Overload Resolution
+++++++++++++++++++

When we call a method we need to find the good overload. This wont be always possible, in that case the specific overload to call should be specified. (eg "echo::(s)" instead of "echo").
We want to be able to support fuzzy matching here and dynamic type. So to do the matching we do the following:

Assuming we have the effective signature of a type, we will do the following:

- filter by number of arguments
- filter by isCompatible signature with the effective signature, and the expected one.

If we still have more than one candidate, the user will have to specify the candidate he want.

Members access
--------------
