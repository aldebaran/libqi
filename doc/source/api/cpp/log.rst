.. _api-log:

qi::log
*******

Brief
-----

.. cpp:brief::

Macro
-----

.. cpp:automacro:: qiLogCategory
.. cpp:automacro:: qiLogDebug
.. cpp:automacro:: qiLogDebugF
.. cpp:automacro:: qiLogVerbose
.. cpp:automacro:: qiLogVerboseF
.. cpp:automacro:: qiLogInfo
.. cpp:automacro:: qiLogInfoF
.. cpp:automacro:: qiLogWarning
.. cpp:automacro:: qiLogWarningF
.. cpp:automacro:: qiLogError
.. cpp:automacro:: qiLogErrorF
.. cpp:automacro:: qiLogFatal
.. cpp:automacro:: qiLogFatalF


Namespace
---------

.. .. cpp:autonamespace:: qi::log

Enum
----

.. .. cpp:autoenum:: qi::LogLevel
.. cpp:autoenum:: qi::LogColor
.. cpp:autoenum:: qi::LogContextAttr

Functions
---------

.. cpp:autofunction:: qi::log::addCategory(const std::string&)
.. cpp:autofunction:: qi::log::addLogHandler(const std::string&, qi::log::logFuncHandler, qi::LogLevel)
.. cpp:autofunction:: qi::log::categories()
.. cpp:autofunction:: qi::log::color()
.. cpp:autofunction:: qi::log::context()
.. cpp:autofunction:: qi::log::destroy()
.. cpp:autofunction:: qi::log::disableCategory(const std::string&, SubscriberId)
.. cpp:autofunction:: qi::log::enableCategory(const std::string&, SubscriberId)
.. cpp:autofunction:: qi::log::flush()
.. cpp:autofunction:: qi::log::init(qi::LogLevel, qi::LogContext, bool)
.. cpp:autofunction:: qi::log::isVisible(CategoryType, qi::LogLevel)
.. cpp:autofunction:: qi::log::isVisible(const std::string&, qi::LogLevel)
.. cpp:autofunction:: qi::log::log(const qi::LogLevel, const char*, const char*, const char*, const char*, const int)
.. cpp:autofunction:: qi::log::log(const qi::LogLevel, CategoryType, const std::string&, const char*, const char*, const int)
.. cpp:autofunction:: qi::log::logLevelToString(const qi::LogLevel, bool)
.. cpp:autofunction:: qi::log::removeLogHandler(const std::string&)
.. cpp:autofunction:: qi::log::setCategory(const std::string&, qi::LogLevel, SubscriberId)
.. cpp:autofunction:: qi::log::setColor(LogColor)
.. cpp:autofunction:: qi::log::setContext(int)
.. cpp:autofunction:: qi::log::setSynchronousLog(bool)
.. cpp:autofunction:: qi::log::setVerbosity(const std::string&, SubscriberId)
.. cpp:autofunction:: qi::log::setVerbosity(const qi::LogLevel, SubscriberId)
.. cpp:autofunction:: qi::log::stringToLogLevel(const char*)
.. cpp:autofunction:: qi::log::verbosity(SubscriberId)
