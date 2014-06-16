.. _guide-cxx-log-client:

.. cpp:auto_template:: True

.. default-role:: cpp:guess

How to receive and send logs
============================

Introduction
-------------

This guide will teach you how to subscribe to LogManager qimessaging services,
configure your listener, receive logs from NAOqi and send logs to it.

NAOqi has a unique logs system service named LogManager. You can:

- use a listener to :ref:`<cxx-log-listener>`,
- :ref:`<cxx-log-publisher>` to dispatch logs to all clients listeners subscribed to LogManager.

Prerequisites
-------------

- An installed NAOqi SDK for your operating system.
- A C++ project setup in your favorite C++ development environment, ready
  to use the headers and libraries provided by the NAOqi SDK.
- Read guilde :ref:`C++: Writing a client<guide-cxx-client>`


.. _cxx-log-listener:

Subscribe to LogManager to obtain logs
--------------------------------------

The first step is to get a LogManager service using a :py:class:`qi.Session`

.. code-block:: cpp

  #include <iostream>

  #include <qi/applicationsession.hpp>

  #include <qicore/logmessage.hpp>
  #include <qicore/logmanager.hpp>

  int main(int argc, char** argv)
  {
    qi::ApplicationSession app(argc, argv);
    app.start();
    qi::LogManagerPtr logger = app.session()->service("LogManager");

    app.run();
  }

Once you have your LogManager service, you need to get a Listener and connect
the onLogMessage signal to your own callback.

.. warning:: You must never use qiLog or call a methods that use qiLog inside your callback. Otherwise you will have an infinite loop.

You can change some settings of your local Listener
(verbosity level, filters :ref:`see below<cxx-log-listener-conf>`).


.. code-block:: cpp

  #include <iostream>

  #include <qi/applicationsession.hpp>

  #include <qicore/logmessage.hpp>
  #include <qicore/logmanager.hpp>
  #include <qicore/loglistener.hpp>

  // More information on logmessage document into LogManager API
  void onMessage(const qi::LogMessage& msg)
  {
    std::stringstream ss;
    ss << msg.category
       << " " << msg.source
       << " " << msg.message;
    std::cout << ss.str() << std::endl;
  }

  int main(int argc, char** argv)
  {
    qi::ApplicationSession app(argc, argv);
    app.start();

    qi::LogManagerPtr logger = app.session()->service("LogManager");
    qi::LogListenerPtr listener = logger->getListener();
    listener->onLogMessage.connect(&onMessage);

    app.run();
  }

.. _cxx-log-listener-conf:

Once you have your listener you can change some verbosity settings:

- clearFilters() will reset/initialize to default all changes made to the listener,
- addFilter(category, level) changes the verbosity for the specific category,
- setLevel(level) changes the verbosity for all categories (setLevel(level) == addFilter("\*", level))

.. code-block:: cpp

  qi::LogListenerPtr listener = logger->getListener();

  // init listener
  listener->clearFilters();

  // All logs with severity lower or equal to DEBUG will be received
  listener->setLevel(qi::LogLevel_Debug)

  // reveive logs with 'foo.bar' category and severity lower or equal than SILENT
  // ei: "foo.bar" logs are never received.
  listener->addFilter("foo.bar", qi::LogLevel_Silent)

  // reveive logs with 'foo.baz.*' category and severity lower or equal than ERROR
  // globbing works fine
  listener->addFilter("foo.baz.*", qi::LogLevel_Error)


.. _cxx-log-publisher:

Add a publisher to send logs
----------------------------

A log publisher is an entity you register to LogManager that sends logs to it.
Then the LogManager will dispatch logs coming from all publishers to all
listeners having subscribed to it.

Be careful, it's recommended to have one publisher per process. Otherwise you
will send the same log as many times as you have publishers in your process.

Example
-------

.. code-block:: cpp

  #include <iostream>

  #include <qi/log.hpp>
  #include <qi/os.hpp>

  #include <qi/applicationsession.hpp>

  #include <qicore/logmessage.hpp>
  #include <qicore/logmanager.hpp>
  #include <qicore/logprovider.hpp>

  int main(int argc, char** argv)
  {
    qi::ApplicationSession app(argc, argv);
    app.start();

    // Get LogManager service
    qi::LogManagerPtr logger = app.session()->service("LogManager");

    // Create LogProvider
    qi::LogProviderPtr provider = qi::createObject("LogProvider", logger);

    // Register LogProvider to LogManager
    int id = logger->addProvider(provider);

    // Logs!!!
    qiLogFatal("myfoo.bar") << "my fatal log";
    qiLogError("myfoo.bar") << "my error log";
    qiLogWarning("myfoo.bar") << "my warning log";
    qiLogInfo("myfoo.bar") << "my info log";
    qiLogVerbose("myfoo.bar") << "my verbose log";
    qiLogDebug("myfoo.bar") << "my debug log";

    // Remove explicity provider from LogManager.
    // It will flush remaining logs
    logger->removeProvider(id);
  }

.. _cxx-log-provider-conf:

The provider settings are set by default to:

- verbosity: qi::LogLevel_Info
- filters: "qitype.*" to qi::LogLevel_Silent, "qimessaging.*" to qi::LogLevel_Silent, "qi.*" to qi::LogLevel_Silent. Thoses filters are NEEDED to avoid infinite recursion. You cannot change those filters.

However, you can add/reset filters or change the verbosity with 3 methods:

- addFilter(category, level) changes the verbosity for the specific category,
- setLevel(level) changes the verbosity for all categories (setLevel(level) == addFilter("\*", level))
- setFilters(filters) will set categories' filters to filters. Filters is a list of pair containing filtering rules (ei: pair("filter", level))

Example
-------

.. code-block:: cpp

    // Create LogProvider
    qi::LogProviderPtr provider = qi::createObject("LogProvider", logger);

    // All logs with severity lower or equal to DEBUG will be sent
    provider->setLevel(qi::LogLevel_Debug)

    // Send logs with 'foo.bar' category and severity lower or equal than SILENT
    // ei: "foo.bar" logs are never sent.
    provider->addFilter("foo.bar", qi::LogLevel_Silent)

    // Send logs with 'foo.baz.*' category and severity lower or equal than ERROR
    // globbing works fine
    provider->addFilter("foo.baz.*", qi::LogLevel_Error)

    // The following code has the same behavior than two previous examples
    // except that it resets filters.
    std::vector<std::pair<std::string, qi::LogLevel> > filters;
    filters.push_back(std::make_pair("foo.bar", qi::LogLevel_Silent));
    filters.push_back(std::make_pair("foo.baz.*", qi::LogLevel_Error));
    provider->setFilters(filters);
