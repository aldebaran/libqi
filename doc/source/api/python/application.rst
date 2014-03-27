.. _api-py-application:

qi.Application API
******************

Introduction
============

Application initializes the qi framework, it extract --qi-* commandline arguments and initialise various options accordingly.
It also provides facilities to connect the main session. This ease the creation of console applications that connects to a session.

You can specify the address of the session to connect to and the address on which the session should listen using:

- --qi-url : address of the session to connect to
- --qi-listen-url : address on which the session will listen

By default the session url is set to "tcp://127.0.0.1:9559" and the listen url to "tcp://0.0.0.0:0".

If raw is specified there wont be a session embedded into the application and you are free to create and connect a session yourself if needed.

if autoExit is set to True, a session disconnection will quit the program. Set autoExit to False to inhibit this behavior. By default autoExit is True.

Application will parse your arguments and catch --qi-* arguments.

For a normal application you have to call start to let the Session embedded into the application connect.
If you want to handle --help yourself you can do that before the application starts avoiding a useless connection to the session.

Reference
=========

.. class:: qi.Application

   .. method:: __init__(args=None, autoExit=True, url=None, raw=False)

      :param args: the list of arguments (usually sys.argv)
      :param autoExit: By default the Application quit on session disconnection, set to False to avoid this behavior (default to True)
      :param url: The default value to use for qi-url (default to tcp://127.0.0.1:9559)
      :param raw: If set to True this do not include a Session into the Application. (advanced user only)

      initialise the Application

   .. method:: start()

      start the Application. Once start is called, your Application is fully working.
      The session is connected and available.

   .. method:: stop()

      stop the Application. (ask run to quit)

   .. method:: run()

      block until stop is called. This calls start if it was not already called.

   .. attribute:: session

      return the current session.

   .. attribute:: url

      Url given to the session to connect to.


Deprecated
==========

.. class:: qi.ApplicationSession

   .. deprecated:: 2.0.1

      Use qi.Application instead.

Examples
========

Simple example that list all ALMemory keys:

.. code-block:: python

   import qi
   import sys
   from pprint import pprint

   if __name__ == "__main__":
       app = qi.Application(sys.argv)

       # start the eventloop
       app.start()

       almemory = app.session.service("ALMemory")

       pprint(almemory.getDataListName())

       #no app.run() needed because we want to exit once getDataListName return

If you put the content of this script in a listmemory.py file, then you can:

.. code-block:: shell

  #connect to tcp://127.0.0.1:9559
  $ python monscript.py

  #connect to tcp://192.168.0.42:9559
  $ python monscript.py --qi-url=tcp://192.168.0.42:9559


Simple example that export a service.

.. code-block:: python

   import qi
   import sys

   class Foo:

       def bar(self):
           print("bar")

   if __name__ == "__main__":
       app = qi.Application(sys.argv)

       # start the eventloop
       app.start()

       app.session.registerService("foo", Foo())

       app.run()   # will exist when the connection is over
