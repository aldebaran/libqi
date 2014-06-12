.. _api-log:
.. cpp:namespace:: qi
.. cpp:auto_template:: True
.. default-role:: cpp:guess

qi::log
*******


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

.. .. cpp:autoenum:: qi::LogLevel
.. cpp:autoenum:: qi::LogColor
.. cpp:autoenum:: qi::LogContextAttr

.. cpp:autonamespace:: qi::log

.. cpp:autofunction:: qi::log::init(qi::LogLevel, qi::LogContext, bool)
.. cpp:autofunction:: qi::log::destroy
.. cpp:autofunction:: qi::log::log(const qi::LogLevel, const char*, const char*, const char*, const char*, const int)
.. cpp:autofunction:: qi::log::log(const qi::LogLevel, CategoryType, const std::string&, const char*, const char*, const int)
.. cpp:autofunction:: qi::log::logLevelToString(const qi::LogLevel, bool)
.. cpp:autofunction:: qi::log::stringToLogLevel(const char*)
.. cpp:autofunction:: qi::log::verbosity(SubscriberId)
.. cpp:autofunction:: qi::log::categories
.. cpp:autofunction:: qi::log::setVerbosity(const std::string&, SubscriberId)
.. cpp:autofunction:: qi::log::setVerbosity(const qi::LogLevel, SubscriberId)
.. cpp:autofunction:: qi::log::enableCategory(const std::string&, SubscriberId)
.. cpp:autofunction:: qi::log::disableCategory(const std::string&, SubscriberId)
.. cpp:autofunction:: qi::log::setCategory(const std::string&, qi::LogLevel, SubscriberId)
.. cpp:autofunction:: qi::log::isVisible(CategoryType, qi::LogLevel)
.. cpp:autofunction:: qi::log::isVisible(const std::string&, qi::LogLevel)
.. cpp:autofunction:: qi::log::setContext(int)
.. cpp:autofunction:: qi::log::setColor(LogColor)
