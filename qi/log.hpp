#pragma once
/*
 * Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

/**
 * \file
 * \includename{qi/log.hpp}
 * \brief Convenient log macro
 */


#ifndef _QI_LOG_HPP_
# define _QI_LOG_HPP_

# include <string>
# include <sstream>
# include <cstdarg>
# include <cstdio>

# include <boost/format.hpp>
# include <boost/function/function_fwd.hpp>

# include <qi/os.hpp>

// For ExceptionLog:
# include <stdexcept>
# include <boost/exception/exception.hpp>
# include <boost/exception/diagnostic_information.hpp>
# include <ka/macro.hpp>
# include <ka/errorhandling.hpp>
# include <ka/macroregular.hpp>
# include <ka/typetraits.hpp>
# include <ka/utility.hpp>


/**
 * \verbatim
 * Add default category for the current scope.
 * Each log has a category defined by the call to qiLogCategory()
 * found in scope. So you must call qiLogCategory() at least once, for
 * instance at the beginning of your source file or function.
 *
 * .. code-block:: cpp
 *
 *     {
 *       qiLogCategory(my.category);
 *       qiLogInfoF("1 + 1 is %s", 1+1);
 *       qiLogInfo() << "1 + 1 is " << 1+1;
 *     }
 * \endverbatim
 */
#define qiLogCategory(Cat)                                               \
  static ::qi::log::CategoryType _QI_LOG_CATEGORY_GET() QI_ATTR_UNUSED = \
    ::qi::log::addCategory(Cat)


/**
 * \verbatim
 * Log in debug mode. Not compiled on release and not shown by default.
 * Use as follow:
 *
 * .. code-block:: cpp
 *
 *     qiLogDebug("foo.bar", "my foo is %d bar", 42);
 *     // or
 *     qiLogDebug("foo.bar") << "my foo is " << 42 << "bar";
 *
 * If you don't want to see any log, use silent log level.
 * \endverbatim
 */
#if defined(NO_QI_DEBUG) || defined(NDEBUG)
# define qiLogDebug(...) ::qi::log::detail::qiFalse() && false < qi::log::detail::NullStream().self()
# define qiLogDebugF(Msg, ...) do {} while(0)
#else
# define qiLogDebug(...)   _QI_LOG_MESSAGE_STREAM(LogLevel_Debug,   Debug ,  __VA_ARGS__)
# define qiLogDebugF(Msg, ...)   _QI_LOG_MESSAGE(LogLevel_Debug,   _QI_LOG_FORMAT(Msg, __VA_ARGS__))
#endif

/**
 * \brief Log in verbose mode. This level is not shown by default.
 */
#if defined(NO_QI_VERBOSE)
# define qiLogVerbose(...) ::qi::log::detail::qiFalse() && false < qi::log::detail::NullStream().self()
# define qiLogVerboseF(Msg, ...) do {} while(0)
#else
# define qiLogVerbose(...) _QI_LOG_MESSAGE_STREAM(LogLevel_Verbose, Verbose, __VA_ARGS__)
# define qiLogVerboseF(Msg, ...)   _QI_LOG_MESSAGE(LogLevel_Verbose,   _QI_LOG_FORMAT(Msg, __VA_ARGS__))
#endif

/**
 * \brief Log in info mode.
 */
#if defined(NO_QI_INFO)
# define qiLogInfo(...) ::qi::log::detail::qiFalse() && false < qi::log::detail::NullStream().self()
# define qiLogInfoF(Msg, ...) do {} while(0)
#else
# define qiLogInfo(...)    _QI_LOG_MESSAGE_STREAM(LogLevel_Info,    Info,    __VA_ARGS__)
# define qiLogInfoF(Msg, ...)   _QI_LOG_MESSAGE(LogLevel_Info,   _QI_LOG_FORMAT(Msg, __VA_ARGS__))
#endif

/**
 * \brief Log in warning mode.
 */
#if defined(NO_QI_WARNING)
# define qiLogWarning(...) ::qi::log::detail::qiFalse() && false < qi::log::detail::NullStream().self()
# define qiLogWarningF(Msg, ...) do {} while(0)
#else
# define qiLogWarning(...) _QI_LOG_MESSAGE_STREAM(LogLevel_Warning, Warning, __VA_ARGS__)
# define qiLogWarningF(Msg, ...)   _QI_LOG_MESSAGE(LogLevel_Warning,   _QI_LOG_FORMAT(Msg, __VA_ARGS__))
#endif

/**
 * \brief Log in error mode.
 */
#if defined(NO_QI_ERROR)
# define qiLogError(...)   ::qi::log::detail::qiFalse() && false < qi::log::detail::NullStream().self()
# define qiLogErrorF(Msg, ...) do {} while(0)
#else
# define qiLogError(...)   _QI_LOG_MESSAGE_STREAM(LogLevel_Error,   Error,   __VA_ARGS__)
# define qiLogErrorF(Msg, ...)   _QI_LOG_MESSAGE(LogLevel_Error,   _QI_LOG_FORMAT(Msg, __VA_ARGS__))
#endif

/**
 * \brief Log in fatal mode.
 */
#if defined(NO_QI_FATAL)
# define qiLogFatal(...)  ::qi::log::detail::qiFalse() && false < qi::log::detail::NullStream().self()
# define qiLogFatalF(Msg, ...) do {} while(0)
#else
# define qiLogFatal(...)   _QI_LOG_MESSAGE_STREAM(LogLevel_Fatal,   Fatal,   __VA_ARGS__)
# define qiLogFatalF(Msg, ...)   _QI_LOG_MESSAGE(LogLevel_Fatal,   _QI_LOG_FORMAT(Msg, __VA_ARGS__))
#endif


namespace qi {
  /**
   * \brief Log level verbosity.
   */
  enum LogLevel {
    LogLevel_Silent = 0, ///< silent log level
    LogLevel_Fatal,      ///< fatal log level
    LogLevel_Error,      ///< error log level
    LogLevel_Warning,    ///< warning log level
    LogLevel_Info,       ///< info log level
    LogLevel_Verbose,    ///< verbose log level
    LogLevel_Debug       ///< debug log level
  };

  /**
   * \brief Logs color mode.
   */
  enum LogColor {
    LogColor_Never, ///< Never show color
    LogColor_Auto,  ///< Auto color
    LogColor_Always ///< Always show color
  };

  /**
   * \brief Logs context attribute.
   */
  enum LogContextAttr {
    LogContextAttr_None           = 0,      ///< No context
    LogContextAttr_Verbosity      = 1 << 0, ///< Show logs level
    LogContextAttr_ShortVerbosity = 1 << 1, ///< Show short logs level
    LogContextAttr_SystemDate     = 1 << 2, ///< Show qi::SystemClock dates
    LogContextAttr_Tid            = 1 << 3, ///< Show threads id
    LogContextAttr_Category       = 1 << 4, ///< Show categories
    LogContextAttr_File           = 1 << 5, ///< Show logs files
    LogContextAttr_Function       = 1 << 6, ///< Show functions name
    LogContextAttr_Return         = 1 << 7, ///< Print an end line between contexts and logs
    LogContextAttr_Date           = 1 << 8, ///< Show qi::Clock dates
  };

  /**
   * \brief Logs context attribute value.
   */
  using LogContext = int;

  /**
   * \brief Log functions with different levels of verbosity.
   */
  namespace log {

    /// \deprecated 1.22 Use qi::LogLevel_Silent
    QI_API_DEPRECATED_MSG(Use 'LogLevel_Silent' instead) static const qi::LogLevel silent = LogLevel_Silent;
    /// \deprecated 1.22 Use qi::LogLevel_Fatal
    QI_API_DEPRECATED_MSG(Use 'LogLevel_Fatal' instead) static const qi::LogLevel fatal = LogLevel_Fatal;
    /// \deprecated 1.22 Use qi::LogLevel_Error
    QI_API_DEPRECATED_MSG(Use 'LogLevel_Error' instead) static const qi::LogLevel error = LogLevel_Error;
    /// \deprecated 1.22 Use qi::LogLevel_Warning
    QI_API_DEPRECATED_MSG(Use 'LogLevel_Warning' instead) static const qi::LogLevel warning = LogLevel_Warning;
    /// \deprecated 1.22 Use qi::LogLevel_Info
    QI_API_DEPRECATED_MSG(Use 'LogLevel_Info' instead) static const qi::LogLevel info = LogLevel_Info;
    /// \deprecated 1.22 Use qi::LogLevel_Verbose
    QI_API_DEPRECATED_MSG(Use 'LogLevel_Verbose' instead) static const qi::LogLevel verbose = LogLevel_Verbose;
    /// \deprecated 1.22 Use qi::LogLevel_Debug
    QI_API_DEPRECATED_MSG(Use 'LogLevel_Debug' instead) static const qi::LogLevel debug = LogLevel_Debug;

    /// \deprecated 1.22 Use qi::LogLevel
    QI_API_DEPRECATED_MSG('qi::log::LogLevel' is deprecated. Use 'qi::LogLevel' instead) typedef qi::LogLevel LogLevel;
  }
}

namespace qi {
  namespace log {
    namespace detail {
      struct Category;
    }

    using SubscriberId = unsigned int; ///< Subscriber Identifier.
    using CategoryType = detail::Category*; ///< Catergory Informations.

    /// \deprecated 1.22 Use qi::log::SubscriberId
    QI_API_DEPRECATED_MSG(Use 'SubscriberId' instead)
    typedef unsigned int Subscriber;

    /**
     * \brief Boost delegate to log function (verbosity lv, date of log,
     *        category, message, file, function, line).
     *  \deprecated 1.24 use qi::log::Handler
     */
    using logFuncHandler = boost::function7<void,
                             const qi::LogLevel,
                             const qi::os::timeval,
                             const char*,
                             const char*,
                             const char*,
                             const char*,
                             int>;
    /**
     * \brief Boost delegate to log function (verbosity lv, dates of log,
     *        category, message, file, function, line).
     */
    using Handler = boost::function8<void,
                             const qi::LogLevel,
                             const qi::Clock::time_point,
                             const qi::SystemClock::time_point,
                             const char*,
                             const char*,
                             const char*,
                             const char*,
                             int>;

    /// Environment variables used by qi::log.
    /// Use qi::os::getenv() to get their value.
    namespace env {
      namespace QI_DEFAULT_LOGHANDLER {
        /// Environment variable QI_DEFAULT_LOGHANDLER specifies which kind of log handler
        /// to set-up at logging system initialization time.
        /// Used by qi::log::init().

        /// The table below sums up which log handler is automatically added according
        /// to the QI_DEFAULT_LOGHANDLER environment, the target OS platform and libqi
        /// WITH_SYSTEMD build option:
        ///
        /// QI_DEFAULT_LOGHANDLER | on Linux | on Windows | on Android | on other platforms
        /// --------------------- | -------- | ---------- | ---------- | ------------------
        /// "none"                |    -     |     -      |     -      |     -
        /// "stdout"              | stdout   |  stdout    |   stdout   |   stdout
        /// "logger"              | logger*  |     -      |   logger   |     -
        /// "debugger"            |    -     |  debugger  |     -      |     -
        /// "" or not defined     | logger*  |  stdout    |   logger   |   stdout
        /// any other value       |    -     |     -      |     -      |     -
        ///
        /// @note * Only if WITH_SYSTEMD is defined.
        /// @note Notice that when WITH_SYSTEMD is defined, it is assumed that
        ///       systemd-journal is actually available on the (linux) platform.
        ///
        /// Into the details:
        ///  * if QI_DEFAULT_LOGHANDLER=="none", no log handler is automatically
        ///    registered at logging system initialization.
        ///    @note Notice that log handlers may still be added with the
        ///          qi::log::addHandler() function, but they won't receive the messages
        ///          submitted before their addition. Even log handlers added at
        ///          the very beginning of the main function, may still miss log messages.
        ///          On the contrary, the default log handler should not miss any message
        ///          as it is set up earlier, before the main is started (actually at
        ///          static initialization time).
        ///  * if QI_DEFAULT_LOGHANDLER=="stdout", the logs are written to stdout
        ///    (using consoleloghandler).
        ///    @note Notice that on Android platform stdout is usually redirected to
        ///          /dev/null.
        ///  * if QI_DEFAULT_LOGHANDLER=="logger", the logs are written to the
        ///    system-wide logger. Currently supported are linux's journald (with
        ///    WITH_SYSTEMD defined) and Android's log output.
        ///  * if QI_DEFAULT_LOGHANDLER=="debugger", the logs are written to a
        ///     debugging facility. Currently only Windows' debug output is
        ///     implemented.
        ///  * if QI_DEFAULT_LOGHANDLER=="" or is not defined, the behavior depends
        ///    on the target OS platform:
        ///     * it behaves as if QI_DEFAULT_LOGHANDLER=="logger" on Linux or on Android,
        ///     * it behaves as if QI_DEFAULT_LOGHANDLER=="stdout" otherwise.
        ///  * if QI_DEFAULT_LOGHANDLER has an unsupported value, or if adding the
        ///    handler fails, no fallback is performed:
        ///    no log handler is registered by libqi, like if QI_DEFAULT_LOGHANDLER=="none",
        ///    but an error message is sent to stderr.
        QI_API extern char const * const name; // TODO: use constexpr after upgrading to c++17
        namespace value {
          QI_API extern char const * const none; // TODO: use constexpr after upgrading to c++17
          QI_API extern char const * const stdOut; // TODO: use constexpr after upgrading to c++17
          QI_API extern char const * const logger; // TODO: use constexpr after upgrading to c++17
          QI_API extern char const * const debugger; // TODO: use constexpr after upgrading to c++17
        }
      }

      // TODO: Publish the names of the other environment variables used by qi::log.
    }

    /**
     * \brief Initialization of the logging system
     *        Creates and registers the default log handler according to
     *        QI_DEFAULT_LOGHANDLER environment variable and compilation flags
     *        WITH_SYSTEMD, ANDROID and BOOST_OS_WINDOWS.
     * \param verb Log verbosity level
     * \param context Display Context
     * \param synchronous Synchronous log
     */
    QI_API void init(qi::LogLevel   verb = qi::LogLevel_Info,
                     qi::LogContext context = qi::LogContextAttr_ShortVerbosity | qi::LogContextAttr_Tid | qi::LogContextAttr_Category,
                     bool           synchronous = true);


    /**
     * \brief Stop and flush the logging system.
     *
     * \verbatim
     * Should be called in the main of program using atexit. For example:
     *
     * .. code-block:: cpp
     *
     *     atexit(qi::log::destroy)
     *
     * This is useful only for asynchronous log.
     * \endverbatim
     */
    QI_API void destroy();


    /**
     * \brief Log function. You should call qiLog* macros instead.
     *
     * \param verb The verbosity of the message.
     * \param category Log category (for filtering).
     * \param msg Log message.
     * \param file Filename from which this function was called (ex: __FILE__).
     * \param fct Function name from which this function was called (ex: __FUNCTION__).
     * \param line Line from which this function was called (ex: __LINE__).
     */
    QI_API void log(const qi::LogLevel verb,
                    const char*        category,
                    const char*        msg,
                    const char*        file = "",
                    const char*        fct = "",
                    const int          line = 0);

    /**
     * \copydoc log()
     */
    QI_API void log(const qi::LogLevel verb,
                    CategoryType       category,
                    const std::string& msg,
                    const char*        file = "",
                    const char*        fct = "",
                    const int          line = 0);


    /**
     * \brief Convert log verbosity to a readable string.
     * \param verb Verbosity value.
     * \param verbose Enable verbose conversion.
     * \return Returns a string matching the log level verbosity.
     */
    QI_API const char* logLevelToString(const qi::LogLevel verb, bool verbose = true);

    /**
     * \brief Convert string to log verbosity.
     * \param verb debug, verbose, info, warning, error, fatal, silent
     * \return Log level verbosity
     */
    QI_API qi::LogLevel stringToLogLevel(const char* verb);

    /**
     * \brief Set log Level.
     * \param lv Default verbosity level shown in the logs.
     * \param sub Log subscriber id.
     *
     * Levels set by this function is a default value, overriden by
     * all addFilter() and addFilters() calls.
     *
     * Change the log minimum level: [0-6] (default:4):
     *   - 0: silent
     *   - 1: fatal
     *   - 2: error
     *   - 3: warning
     *   - 4: info
     *   - 5: verbose
     *   - 6: debug
     *
     * Can be set with env var QI_LOG_LEVEL.
     *
     * If you don't want any log use silent mode.
     */
    QI_API void setLogLevel(const qi::LogLevel lv, SubscriberId sub = 0);

    /**
     * \brief Get log verbosity.
     * \param sub Log subscriber id.
     * \return Maximal verbosity displayed.
     */
    QI_API qi::LogLevel logLevel(SubscriberId sub = 0);

    /**
     * \brief Get the list of all categories.
     * \return The list of existing categories
     */
    QI_API std::vector<std::string> categories();

    /**
     * \brief Add/get a category.
     * \param name Category to add/get.
     * \return CategoryType structure.
     */
    QI_API CategoryType addCategory(const std::string& name);
    /**
     * \brief Set category to current verbosity level. Globbing is supported.
     * \param cat Category to set to current verbosity level.
     * \param sub Log subscriber id.
     */
    QI_API void enableCategory(const std::string& cat, SubscriberId sub = 0);
    /**
     * \brief Set category to silent log level. Globbing is supported.
     * \param cat Category to set to silence level.
     * \param sub Log subscriber id.
     */
    QI_API void disableCategory(const std::string& cat, SubscriberId sub = 0);

    /**
     * \brief Check if the given combination of category and level is enable
     * \param category Category to check.
     * \param level Level associate to category.
     * \return true if given combination of category and level is enabled.
     */
    inline bool isVisible(CategoryType category, qi::LogLevel level);

    /**
     * \copydoc isVisible()
     */
    QI_API bool isVisible(const std::string& category, qi::LogLevel level);

    /**
     * \brief Parse and execute a set of verbosity rules.
     * \param rules Colon separated of rules.
     *  Each rule can be:
     *    - (+)?CAT    : enable category CAT
     *    - -CAT       : disable category CAT
     *    - CAT=level  : set category CAT to level
     *
     * Each category can include a '*' for globbing.
     * Can be set with env var QI_LOG_FILTERS. For instance
     * 'qi.*=debug:-qi.foo:+qi.foo.bar' stands for
     * "all qi.* logs in debug, remove all qi.foo logs except qi.foo.bar".
     * \param sub Log subscriber id.
     */
    QI_API void addFilters(const std::string& rules, SubscriberId sub = 0);

    /**
     * \brief Set per-subscriber category to level. Globbing is supported.
     * \param cat Category to set.
     * \param level Level to set to the category.
     * \param sub Log subscriber id.
     *
     * \verbatim
     * .. code-block:: cpp
     *
     *   addFilter("internal.*", silent);
     *
     * One can also set a filtering rule in QI_LOG_FILTERS environment variable.
     * syntax is colon-separated list of rules of the form (+|-)CAT or CAT=level.
     * For example, -internal.*:file=verbose
     *
     * \endverbatim
     */
    QI_API void addFilter(const std::string& cat, qi::LogLevel level, SubscriberId sub = 0);

    /**
     * \brief Set log context verbosity.
     *
     * Show context logs, it's a bit field (add the values below).
     *
     * \param ctx Value to set context.
     *
     * Context values possible:
     *   - 1  : Verbosity
     *   - 2  : ShortVerbosity
     *   - 4  : Date
     *   - 8  : ThreadId
     *   - 16 : Category
     *   - 32 : File
     *   - 64 : Function
     *   - 128: EndOfLine
     *  some useful values for context are:
     *   - 26 : (verb+threadId+cat)
     *   - 30 : (verb+threadId+date+cat)
     *   - 126: (verb+threadId+date+cat+file+fun)
     *   - 254: (verb+threadId+date+cat+file+fun+eol)
     *
     * Can be set with env var QI_LOG_CONTEXT
     */
    QI_API void setContext(int ctx);

    /**
     * \brief Get log context.
     * \return Returns the level of context verbosity.
     */
    QI_API int context();

    /**
     * \brief Set log color.
     * \param color Log color value.
     */
    QI_API void setColor(LogColor color);

    /**
     * \brief Get log color.
     * \return Returns LogColor enum.
     */
    QI_API LogColor color();

    /**
     * \brief Enables or disables synchronous logs.
     * \param sync Value to set or unset synchronous.
     *
     * When setting to async, this function must be called after main has
     * started.
     */
    QI_API void setSynchronousLog(bool sync);

    /**
     * \brief Add a log handler for this process' logs.
     * \warning Handlers are usually called synchronously, they must not block.
     * \param name Name of the handler, useful to remove handler (prefer lowercase).
     * \param fct Boost delegate to log handler function.
     * \param defaultLevel default log verbosity.
     * \return New log subscriber id added.
     */
    QI_API SubscriberId addHandler(const std::string& name,
                                   qi::log::Handler fct,
                                   qi::LogLevel defaultLevel = LogLevel_Info);
    /**
     * \brief Add a log handler.
     * \param name Name of the handler, useful to remove handler (prefer lowercase).
     * \param fct Boost delegate to log handler function.
     * \param defaultLevel default log verbosity.
     * \return New log subscriber id added.
     * \deprecated 1.24 use qi::log::addHandler
     */
    QI_API_DEPRECATED_MSG(Use 'addHandler' instead)
    QI_API SubscriberId addLogHandler(const std::string& name,
                                      qi::log::logFuncHandler fct,
                                      qi::LogLevel defaultLevel = LogLevel_Info);

    /**
     * \brief Remove a log handler.
     * \param name Name of the handler.
     */
    QI_API void removeHandler(const std::string& name);

    /**
     * \brief Remove a log handler.
     * \param name Name of the handler.
     * \deprecated 1.24 use qi::log::removeHandler
     */
    QI_API_DEPRECATED_MSG(Use 'removeHandler' instead)
    QI_API void removeLogHandler(const std::string& name);

    /**
     * \brief Flush asynchronous logs.
     */
    QI_API void flush();

KA_WARNING_PUSH()
KA_WARNING_DISABLE(4996, deprecated-declarations) // ignore LogLevel deprecation warnings

    /// \deprecated since 1.22. Use qi::log::setLogLevel(const qi::LogLevel, SubscriberId)
    QI_API_DEPRECATED_MSG(Use 'setLogLevel' instead)
    inline void setVerbosity(SubscriberId sub, const qi::log::LogLevel lv) { setLogLevel(static_cast<qi::LogLevel>(lv), sub); }
    /// \deprecated since 1.22. Use qi::log::addFilter(const std::string&, qi::LogLevel, SubscriberId)
    QI_API_DEPRECATED_MSG(Use 'addFilter' instead)
    inline void setCategory(SubscriberId sub, const std::string& cat, qi::log::LogLevel level) { addFilter(cat, static_cast<qi::LogLevel>(level), sub); }

KA_WARNING_POP()

    /**
     * \copydoc qi::log::level
     * \deprecated since 2.2. Use qi::log::logLevel instead.
     */
    QI_API QI_API_DEPRECATED_MSG(Use 'logLevel' instead)
    qi::LogLevel verbosity(SubscriberId sub = 0);

    /**
     * \copydoc qi::log::addFilters()
     * \deprecated since 2.2 Use qi::log::addFilters instead.
     */
    QI_API QI_API_DEPRECATED_MSG(Use 'addFilters' instead)
    void setVerbosity(const std::string& rules, SubscriberId sub = 0);

    /**
     * \copydoc qi::log::setLogLevel()
     * \deprecated since 2.2 Use qi::log::setLogLevel instead.
     */
    QI_API QI_API_DEPRECATED_MSG(Use 'setLogLevel' instead)
    void setVerbosity(const qi::LogLevel lv, SubscriberId sub = 0);

    /**
     * \copydoc qi::log::setFilter
     * \deprecated since 2.2 Use qi::log::addFilter instead.
     */
    QI_API QI_API_DEPRECATED_MSG(Use 'addFilter' instead)
    void setCategory(const std::string& catName, qi::LogLevel level, SubscriberId sub = 0);

  }

}

# include <qi/detail/log.hxx>

namespace qi
{

  namespace detail {

#define QI_DERIVE_EXCEPTIONLOGIMPL(LEVEL)                                    \
    /* Logs data for level #LEVEL */                                         \
    /* TODO: Make variadic when C++ version >= 17. */                        \
    template<typename T, typename U, typename V, typename W>                 \
    void exceptionLogImpl(ka::int_constant_t<LogLevel_ ## LEVEL> /* level*/, \
      [[maybe_unused]] const T& category, const U& prefix,                   \
      const V& except, const W& msg)                                         \
    {                                                                        \
      qiLog ## LEVEL(category) << prefix << except << msg;                   \
    }

    // Empty version for the 'silent' log level.
    template<typename T, typename U, typename V, typename W>
    void exceptionLogImpl(ka::int_constant_t<LogLevel_Silent>, const T&,
      const U&, const V&, const W&)
    {
    }

    QI_DERIVE_EXCEPTIONLOGIMPL(Fatal)
    QI_DERIVE_EXCEPTIONLOGIMPL(Error)
    QI_DERIVE_EXCEPTIONLOGIMPL(Warning)
    QI_DERIVE_EXCEPTIONLOGIMPL(Info)
    QI_DERIVE_EXCEPTIONLOGIMPL(Verbose)
    QI_DERIVE_EXCEPTIONLOGIMPL(Debug)
#undef QI_DERIVE_EXCEPTIONLOGIMPL
  } // namespace detail

  /// Logs an exception in the log at the specified level, distinguishing
  /// std::exception, boost::exception and unknown exception
  /// (typically for the `catch (...)` case).
  ///
  /// You can provide a log category and a prefix to the log.
  ///
  /// OStreamable O, ConvertibleTo<const char*> S
  template<LogLevel L, typename O, typename S = char const*>
  struct ExceptionLog
  {
    S category;
    O prefix;
  // Regular (if S and O are):
    explicit ExceptionLog(S category = {}, O prefix = {})
      : category(category), prefix(prefix)
    {
    }
    KA_GENERATE_FRIEND_REGULAR_OPS_2(ExceptionLog, category, prefix)
  // Custom:
    void operator()(const std::exception& e) const
    {
      detail::exceptionLogImpl(ka::int_constant_t<L>{}, category, prefix,
        ": standard exception: ", e.what());
    }
    void operator()(const boost::exception& e) const
    {
      detail::exceptionLogImpl(ka::int_constant_t<L>{}, category, prefix,
        ": boost exception: ", boost::diagnostic_information(e));
    }
    void operator()() const
    {
      detail::exceptionLogImpl(ka::int_constant_t<L>{}, category, prefix,
        ": unknown exception.", "");
    }
  };

  // Defines specialization of `ExceptionLog` for each log level.
  // E.g. `ExceptionLogError` for `ExceptionLog<LogLevel_Error>`.
#define QI_DERIVE_EXCEPTIONLOG(LEVEL)                                   \
  /** Binds a log level to `ExceptionLog`. See `ExceptionLog`. */       \
  template<typename O, typename S = char const*>                        \
  using ExceptionLog ## LEVEL = ExceptionLog<LogLevel_ ## LEVEL, O, S>;

  QI_DERIVE_EXCEPTIONLOG(Silent)
  QI_DERIVE_EXCEPTIONLOG(Fatal)
  QI_DERIVE_EXCEPTIONLOG(Error)
  QI_DERIVE_EXCEPTIONLOG(Warning)
  QI_DERIVE_EXCEPTIONLOG(Info)
  QI_DERIVE_EXCEPTIONLOG(Verbose)
  QI_DERIVE_EXCEPTIONLOG(Debug)

#undef QI_DERIVE_EXCEPTIONLOG

  /// Helper-function to deduce types for `ExceptionLog`.
  ///
  /// Example: Using catch-clauses.
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /// const auto logError = qi::exceptionLog<LogLevel_Error>(
  ///   "myapp", "The function that could throw threw");
  /// try
  /// {
  ///   functionThatMightThrow();
  /// }
  /// catch (const std::exception& ex)
  /// {
  ///   logError(ex);
  /// }
  /// catch (const boost::exception& ex)
  /// {
  ///   logError(ex);
  /// }
  /// catch (...)
  /// {
  ///   logError();
  /// }
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  ///
  /// Example: Using ka::invoke_catch.
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /// ka::invoke_catch(
  ///   qi::exceptionLog<LogLevel_Error>(
  ///     "myapp", "The function that could throw threw"),
  ///   functionThatMightThrow
  /// );
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  ///
  /// OStreamable O, ConvertibleTo<const char*> S
  template<LogLevel level, typename O, typename S>
  ExceptionLog<level, ka::Decay<O>, ka::Decay<S>> exceptionLog(S&& category, O&& prefix)
  {
    return ExceptionLog<level, ka::Decay<O>, ka::Decay<S>>{
      ka::fwd<S>(category), ka::fwd<O>(prefix)
    };
  }

  // Defines variants of `exceptionLog` for each log level.
  // E.g. `exceptionLogError` for `exceptionLog<LogLevel_Error>`.
#define QI_DERIVE_EXCEPTIONLOG_FN(LEVEL)                          \
  /** Binds a log level to `exceptionLog`. See `exceptionLog`. */ \
  template<typename O, typename S>                                \
  auto exceptionLog ## LEVEL(S&& category, O&& prefix)            \
    -> decltype(exceptionLog<LogLevel_ ## LEVEL>(                 \
         ka::fwd<S>(category), ka::fwd<O>(prefix)))               \
  {                                                               \
    return exceptionLog<LogLevel_ ## LEVEL>(                      \
      ka::fwd<S>(category), ka::fwd<O>(prefix));                  \
  }

  QI_DERIVE_EXCEPTIONLOG_FN(Silent)
  QI_DERIVE_EXCEPTIONLOG_FN(Fatal)
  QI_DERIVE_EXCEPTIONLOG_FN(Error)
  QI_DERIVE_EXCEPTIONLOG_FN(Warning)
  QI_DERIVE_EXCEPTIONLOG_FN(Info)
  QI_DERIVE_EXCEPTIONLOG_FN(Verbose)
  QI_DERIVE_EXCEPTIONLOG_FN(Debug)
#undef QI_DERIVE_EXCEPTIONLOG_FN

  /// Polymorphic procedure that invokes a procedure, catching any exception,
  /// logging details, and rethrowing the exception, if any.
  ///
  /// Example: Creating a resource, logging in error any exception and rethrowing it.
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /// InvokeCatchLogRethrow<LogLevel_Error> call; // Or `InvokeCatchLogRethrowError call;`
  /// auto resource = call(
  ///   "resource creation",             // log category
  ///   "could not create the resource", // log prefix
  ///   createResource,                  // auto (string, Mode)
  ///   name,                            // first argument of `createResource`
  ///   readOnly                         // second argument of `createResource`
  ///  );
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  ///
  /// TODO: Remove decltype when C++ version >= 14.
  template<LogLevel L>
  struct InvokeCatchLogRethrow
  {
  // Regular:
    KA_GENERATE_FRIEND_REGULAR_OPS_0(InvokeCatchLogRethrow)
  // PolymorphicProcedure:
    /// ConvertibleTo<const char*> S, OStreamable O, Procedure<_ (Args...)> Proc
    template<typename S, typename O, typename Proc, typename... Args>
    auto operator()(S&& logCategory, O&& logPrefix, Proc&& proc, Args&&... args)
      -> decltype(ka::invoke_catch(
                    ka::handle_exception_rethrow(
                      exceptionLog<L>(ka::fwd<S>(logCategory), ka::fwd<O>(logPrefix)),
                      ka::type_t<ka::CodomainFor<ka::Decay<Proc>, Args...>>{}
                    ),
                    ka::fwd<Proc>(proc),
                    ka::fwd<Args>(args)...
                  ))
    {
      using namespace ka;
      return invoke_catch(
        handle_exception_rethrow(
          exceptionLog<L>(fwd<S>(logCategory), fwd<O>(logPrefix)),
          type_t<CodomainFor<ka::Decay<Proc>, Args...>>{} // force return type to match Proc's one
        ),
        fwd<Proc>(proc),
        fwd<Args>(args)...
      );
    }
  };

  // Defines type aliases of `InvokeCatchLogRethrow` for each log level.
  // E.g. `InvokeCatchLogRethrowError` for `InvokeCatchLogRethrow<LogLevel_Error>`.
#define QI_DERIVE_INVOKECATCHLOGRETHROW(LEVEL)                                      \
  /** Binds a log level to `InvokeCatchLogRethrow`. See `InvokeCatchLogRethrow` **/ \
  using InvokeCatchLogRethrow ## LEVEL = InvokeCatchLogRethrow<LogLevel_ ## LEVEL>;

  QI_DERIVE_INVOKECATCHLOGRETHROW(Silent)
  QI_DERIVE_INVOKECATCHLOGRETHROW(Fatal)
  QI_DERIVE_INVOKECATCHLOGRETHROW(Error)
  QI_DERIVE_INVOKECATCHLOGRETHROW(Warning)
  QI_DERIVE_INVOKECATCHLOGRETHROW(Info)
  QI_DERIVE_INVOKECATCHLOGRETHROW(Verbose)
  QI_DERIVE_INVOKECATCHLOGRETHROW(Debug)
#undef QI_DERIVE_INVOKECATCHLOGRETHROW

} // namespace qi

#endif  // _QI_LOG_HPP_
