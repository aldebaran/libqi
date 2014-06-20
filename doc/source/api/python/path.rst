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

.. code-block:: python

   import qi
   #where should I write the files for my application "myapp"?

   dataPath = qi.path.userWritableDataPath("myapp", "mydatafile")
   # datapath => /home/nao/.local/share/myapp/mydatafile
   confPath = qi.path.userWritableConfPath("myapp", "myconffile")
   # confPath => /home/nao/.config/myapp/myconffile


   # I want to save all my data and configuration files in a personal folder
   qi.path.setWritablePath("/home/nao/my_path")

   dataPath = qi.path.userWritableDataPath("myapp", "mydatafile")
   # datapath => /home/nao/my_path/data/myapp/mydatafile
   confPath = qi.path.userWritableConfPath("myapp", "myconffile")
   # confPath => /home/nao/my_path/config/myapp/myconffile
