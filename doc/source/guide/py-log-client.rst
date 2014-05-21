.. _guide-py-log-client:

How to receive logs form NAOqi SDK in Python
============================================

Introduction
------------


This guide will teach you how to subscribe to LogManager qimessaging services,
configure your listener, receive logs from NAOqi and send logs to it.

NAOqi has a unique logs system service named LogManager. You can:

- use a listener to :ref:`<py-log-listener>`,
- :ref:`<py-log-publisher>` to dispatch logs to all clients listeners subscribed to LogManager.

Prerequisites
-------------

- An installed python NAOqi SDK for your operating system.
- Read guide :ref:`python: Writing a client<guide-py-client>`


.. _py-log-listener:

Subscribe to LogManager to obtain logs
--------------------------------------

The first step is to get a LogManager service using a :py:class:`qi.Session`

.. code-block:: python

  import sys
  import qi
  import qi.logging

  def main(argv)
      app = qi.ApplicationSession(argv)
      app.start()
      logmanager = app.session.service("LogManager")
      app.run()

  if __name__ == "__main__":
      main(sys.argv)

Once you have your LogManager service, you need to get a Listener and connect
the onLogMessage signal to your own callback.

.. warning:: You must never use qiLog or call a methods that use qiLog inside your callback. Otherwise you will have an infinite loop.

You can change some settings of your local Listener
(verbosity level, filters :ref:`see below<py-log-listener-conf>`).

.. code-block:: python

  import sys
  import qi
  import qi.logging

  def onMessage(mess):
      print mess # mess is a dictionnary with all known LogMessage information.

  def main(argv):
      app = qi.ApplicationSession(argv)
      app.start()
      logmanager = app.session.service("LogManager")
      listener = logmanager.getListener()
      listener.onLogMessage.connect(onMessage)
      app.run()

  if __name__ == "__main__":
      main(sys.argv)


.. _py-log-listener-conf:

Once you have your listener you can change some verbosity settings:

- clearFilters() will reset/initialize to default all changes made to the listener,
- addFilter(category, level) changes the verbosity for the specific category,
- setLevel(level) changes the verbosity for all categories (setLevel(level) == addFilter("\*", level))

.. code-block:: python

  listener = logmanager.getListener()

  # init listener
  listener.clearFilters();

  # All logs with severity lower or equal to DEBUG will be received
  listener.setLevel(qi.logging.DEBUG)

  # reveive logs with 'foo.bar' category and severity lower or equal than SILENT
  # ei: "foo.bar" logs are never received.
  listener.addFilter("foo.bar", qi.logging.SILENT)

  # reveive logs with 'foo.baz.*' category and severity lower or equal than ERROR
  # globbing works fine
  listener.addFilter("foo.baz.*", qi.logging.ERROR)



.. _py-log-publisher:

Add a publisher to send logs
----------------------------

A log publisher is an entity you register to LogManager that sends logs to it.
Then the LogManager dispatches logs coming from all publishers to all
listeners having subscribed to it.

Be careful, it's recommended to have one publisher per process. Otherwise you
will send the same log as many times as you have publishers in your process.

Example
-------

.. code-block:: python

  import sys
  import qi
  import qi.path
  import qi.logging
  from ctypes import *

  def main(argv):
      # Load all Provider symbols
      cdll.LoadLibrary(qi.path.findLib("logprovider"))

      app = qi.ApplicationSession(sys.argv)
      app.start()

      # Get LogManager service
      logmanager = app.session.service("LogManager")

      # Create a provider
      provider = qi.createObject("LogProvider", logmanager)

      # Register LogProvider to LogManager
      providerId = logmanager.addProvider(provider)

      # Logs!!!
      mylogger = qi.Logger("myfoo.bar")
      mylogger.fatal("my fatal log")
      mylogger.error("my error log")
      mylogger.warning("my warning log")
      mylogger.info("my info log")
      mylogger.verbose("my verbose log")

      # Remove explicity provider from LogManager.
      # It will flush remaining logs
      logmanager.removeProvider(id);

  if __name__ == "__main__":
      main(sys.argv)

.. _py-log-provider-conf:

The provider settings are set by default to:

- verbosity: qi.logging.INFO
- filters: "qitype.*" to qi.logging.SILENT, "qimessaging.*" to qi.logging.SILENT, "qi.*" to qi.logging.SILENT. Thoses filters are NEEDED to avoid infinite recursion. You cannot change those filters.

However, you can add/reset filters or change the verbosity with 3 methods:

- addFilter(category, level) changes the verbosity for the specific category,
- setLevel(level) changes the verbosity for all categories (setLevel(level) == addFilter("\*", level))
- setFilters(filters) will set categories' filters to filters. Filters is a list of pair containing filtering rules (ei: pair("filter", level))

Example
-------

.. code-block:: python

  # Create Logger Provider
  provider = qi.createObject("LogProvider", logger);

  # All logs with severity lower or equal to DEBUG will be sent
  provider.setLevel(qi.logging.DEBUG)

  # Send logs with 'foo.bar' category and severity lower or equal than SILENT
  # ei: "foo.bar" logs are never sent.
  provider.addFilter("foo.bar", qi.logging.SILENT)

  # Send logs with 'foo.baz.*' category and severity lower or equal than ERROR
  # globbing works fine
  provider.addFilter("foo.baz.*", qi.logging.ERROR)

  # The following code has the same behavior than two previous examples
  # except that it resets filters.
  filters = [{"foo.bar": qi.logging.SILENT}, {"foo.bar.*": qi.logging.ERROR}]
  provider.setFilters(filters);
