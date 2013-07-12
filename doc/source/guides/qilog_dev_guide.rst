=======================
Logging Developer Guide
=======================

Overview
--------

For C++ API Reference on logging, go to :doc:`qi::log API Reference</api/namespaces/qilog>`.

Hierarchical Category
^^^^^^^^^^^^^^^^^^^^^

There is a hierarchical categorization of all the logs like in python. This will
be used to filter logs in the future. Each category can contain sub-categories
and the name of the category is expressed with its full path, sub-categories
separated with points.

- qi

  - qi.core

    - qi.core.client

  - qi.audio

    - qi.audio.tts
    - qi.audio.asr

Handlers
^^^^^^^^

The default handler logs to console. The color is enabled on a tty. An handler
can be added or deleted. You just need to give a delegate to a log function with
the following prototype:

.. code-block:: cpp

    void logfct(const qi::log::LogLevel verbosity,
                const char*             category,
                const char*             message,
                const char*             filename = "",
                const char*             function = "",
                const int               line = 0);

Then you can add or remove an handler with
:cpp:func:`qi::log::addLogHandler(const std::string&, qi::log::logFuncHandler)`
and :cpp:func:`qi::log::removeLogHandler(const std::string&)` using the
following code:

.. code-block:: cpp

    // Adds the handler.
    qi::log::addLogHandler("nameOfLogHandler", logfct);

    // Removes the handler.
    qi::log::removeLogHandler("nameOfLogHandler");

Verbosity
^^^^^^^^^

There is now 7 levels of logging that you can change using ``--log-level`` (``-L``)
option in order from 0 to 6. Options work only if program initializes
:cpp:class:`qi::Application` in its main.

- ``silent``: hides every log.
- ``fatal``: used before the program exits if a fatal error occurred.
- ``error``: classical error.
- ``warning``: useful to warn the user about something that failed but is not an error.
- ``info``: standard messages should be logged using this verbosity, it is the highest
            level shown by default.
- ``verbose``: not mandatory, but useful as user information. It is enabled with
               ``--verbose`` (``-v``) option on command line.
- ``debug``: useful to the developer. Not compiled in release mode. it is enabled
             with ``--debug`` (``-d``) option on command line.

Example printing only error messages and lower:

.. code-block:: console

    $ ./a.out -L 2

    I've just finished to log!
    [FATAL] ...log.example.1: 41
    [ERROR] ...log.example.1: 42
    [FATAL] ...log.example.2: f4
    [ERROR] ...log.example.2: e4
    [FATAL] ...log.example.3: 21f4
    [ERROR] ...log.example.3: 21e4
    [ERROR] ...log.example.4: Where is nao? - Nao is in the kitchen. - How many are they? 42

Code location
^^^^^^^^^^^^^

Code location is supported. You can get some information about where is the log
call including line, function and filename. You can get the location using
``--context`` (``-c``) option on command line.

.. code-block:: console

    $ ./a.out -c
    I've just finished to log!
    [FATAL] ...log.example.1: log_example.cpp(91) main 41
    [ERROR] ...log.example.1: log_example.cpp(92) main 42
    [WARN ] ...log.example.1: log_example.cpp(93) main 43
    [INFO ] ...log.example.1: log_example.cpp(94) main 44
    [FATAL] ...log.example.2: log_example.cpp(98) main f4
    [ERROR] ...log.example.2: log_example.cpp(99) main e4
    [WARN ] ...log.example.2: log_example.cpp(100) main w4
    [INFO ] ...log.example.2: log_example.cpp(101) main i4
    [FATAL] ...log.example.3: log_example.cpp(105) main 21f4
    [ERROR] ...log.example.3: log_example.cpp(106) main 21e4
    [WARN ] ...log.example.3: log_example.cpp(107) main 21w4
    [INFO ] ...log.example.3: log_example.cpp(108) main 21i4
    [WARN ] ...log.example.4: log_example.cpp(115) main Oups my buffer is too bad: badcafe
    [ERROR] ...log.example.4: log_example.cpp(118) main Where is nao? - Nao is in the kitchen. - How many are they? 42
    [INFO ] ...log.example.4: log_example.cpp(124) main 41 42 43 44

Asynchronous log
^^^^^^^^^^^^^^^^

Log are asynchronous, because we don't want to waste time in logs, furthermore
logs can do networking. The handling of the log output is in a separated thread.

There is a way to disable asynchronous logs. Starting naoqi with
``--synchronous-log`` command line option. Be careful, it will slow down naoqi if
you have lots of handlers.

Logs may be asynchronous, but they are actually being displayed in their request
order.

Compatibility
^^^^^^^^^^^^^

allog should continue working and API should be identical.

Full Example
^^^^^^^^^^^^

.. literalinclude:: examples/qilog.cpp
    :language: cpp
