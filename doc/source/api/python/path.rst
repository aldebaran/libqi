.. _api-py-path:

qi.path API
***********

Introduction
============

qi.path is a set of functions to find binaries, libraries, data and configuration files on the system.


Reference
=========

.. automodule:: qi.path
   :members:
   :undoc-members:


Libraries and binaries names
============================

Some qi.path functions take the name of a library or a binary as a parameter.

**Full name of the library** means you will include the OS specific extensions in your name.

For example, **libopencv_objdetect.so** .

**Just the name** means that you can just have the simple name of the library you want,

and do not need to worry about its extension.

For example, **opencv_objdetect** .

Example
=======
