/*
 * Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */
 
 #pragma once
#ifndef _LIBQI_QI_LOG_HXX_
#define _LIBQI_QI_LOG_HXX_


#if defined(NO_QI_LOG_DETAILED_CONTEXT) || defined(NDEBUG)
#   define _qiLogDebug(...)      qi::log::LogStream(qi::log::debug, "", __FUNCTION__, 0, __VA_ARGS__).self()
#else
#   define _qiLogDebug(...)      qi::log::LogStream(qi::log::debug, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__).self()
#endif

#if defined(NO_QI_LOG_DETAILED_CONTEXT) || defined(NDEBUG)
# define _qiLogVerbose(...)      qi::log::LogStream(qi::log::verbose, "", __FUNCTION__, 0, __VA_ARGS__).self()
#else
# define _qiLogVerbose(...)      qi::log::LogStream(qi::log::verbose, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__).self()
#endif

#if defined(NO_QI_LOG_DETAILED_CONTEXT) || defined(NDEBUG)
# define _qiLogInfo(...)         qi::log::LogStream(qi::log::info, "", __FUNCTION__, 0, __VA_ARGS__).self()
#else
# define _qiLogInfo(...)         qi::log::LogStream(qi::log::info, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__).self()
#endif

#if defined(NO_QI_LOG_DETAILED_CONTEXT) || defined(NDEBUG)
# define _qiLogWarning(...)      qi::log::LogStream(qi::log::warning, "", __FUNCTION__, 0, __VA_ARGS__).self()
#else
# define _qiLogWarning(...)      qi::log::LogStream(qi::log::warning, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__).self()
#endif

#if defined(NO_QI_LOG_DETAILED_CONTEXT) || defined(NDEBUG)
# define _qiLogError(...)        qi::log::LogStream(qi::log::error, "", __FUNCTION__, 0, __VA_ARGS__).self()
#else
# define _qiLogError(...)        qi::log::LogStream(qi::log::error, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__).self()
#endif

#if defined(NO_QI_LOG_DETAILED_CONTEXT) || defined(NDEBUG)
# define _qiLogFatal(...)        qi::log::LogStream(qi::log::fatal, "", __FUNCTION__, 0, __VA_ARGS__).self()
#else
# define _qiLogFatal(...)        qi::log::LogStream(qi::log::fatal, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__).self()
#endif


// enum level {
//   silent = 0,
//   fatal,
//   error,
//   warning,
//   info,
//   verbose,
//   debug
// };


#  define _QI_FORMAT_ELEM(_, a, elem) % (elem)

#  define _QI_LOG_FORMAT(Msg, ...)                   \
  QI_CAT(_QI_LOG_FORMAT_HASARG_, _QI_LOG_ISEMPTY(__VA_ARGS__))(Msg, __VA_ARGS__)

#define _QI_LOG_FORMAT_HASARG_0(Msg, ...) \
  boost::str(::qi::log::detail::getFormat(Msg)  QI_VAARGS_APPLY(_QI_FORMAT_ELEM, _, __VA_ARGS__ /**/))

#define _QI_LOG_FORMAT_HASARG_1(Msg, ...) Msg

#define _QI_SECOND(a, ...) __VA_ARGS__

/* For fast category access, we use lookup to a fixed name symbol.
 * The user is required to call qiLogCategory somewhere in scope.
 */

#  define _QI_LOG_CATEGORY_GET() _qi_log_category

#if defined(NO_QI_LOG_DETAILED_CONTEXT) || defined(NDEBUG)
#  define _QI_LOG_MESSAGE(Type, Message)                        \
  do                                                            \
  {                                                             \
    if (::qi::log::detail::isVisible(_QI_LOG_CATEGORY_GET(), ::qi::log::Type))  \
      ::qi::log::log(::qi::log::Type,                           \
                         _QI_LOG_CATEGORY_GET(),                \
                         Message,                               \
                         "", __FUNCTION__, 0);                  \
  }                                                             \
  while (false)
#else
#  define _QI_LOG_MESSAGE(Type, Message)                        \
  do                                                            \
  {                                                             \
    if (::qi::log::detail::isVisible(_QI_LOG_CATEGORY_GET(), ::qi::log::Type))  \
      ::qi::log::log(::qi::log::Type,                           \
                         _QI_LOG_CATEGORY_GET(),                \
                         Message,                               \
                         __FILE__, __FUNCTION__, __LINE__);     \
  }                                                             \
  while (false)
#endif

/* Tricky, we do not want to hit category_get if a category is specified
* Usual glitch of off-by-one list size: put argument 'TypeCased' in the vaargs
* Basically we want variadic macro, but it does not exist, so emulate it using _QI_LOG_EMPTY.
*/
#  define _QI_LOG_MESSAGE_STREAM(Type, TypeCased, ...)                 \
  QI_CAT(_QI_LOG_MESSAGE_STREAM_HASCAT_, _QI_LOG_ISEMPTY( __VA_ARGS__))(Type, TypeCased, __VA_ARGS__)

// no extra argument
#define _QI_LOG_MESSAGE_STREAM_HASCAT_1(Type, TypeCased, ...) \
  ::qi::log::detail::isVisible(_QI_LOG_CATEGORY_GET(), ::qi::log::Type) \
  && BOOST_PP_CAT(_qiLog, TypeCased)(_QI_LOG_CATEGORY_GET())

// Visual bouncer for macro evalution order glitch.
#ifdef _MSC_VER
#define _QI_LOG_MESSAGE_STREAM_HASCAT_0(...) QI_DELAY(_QI_LOG_MESSAGE_STREAM_HASCAT_0) ## _BOUNCE(__VA_ARGS__)
#else
#define _QI_LOG_MESSAGE_STREAM_HASCAT_0(...) _QI_LOG_MESSAGE_STREAM_HASCAT_0_BOUNCE(__VA_ARGS__)
#endif

// At leas one argument: category. Check for a format argument
#define _QI_LOG_MESSAGE_STREAM_HASCAT_0_BOUNCE(Type, TypeCased, cat, ...) \
 QI_CAT(_QI_LOG_MESSAGE_STREAM_HASCAT_HASFORMAT_, _QI_LOG_ISEMPTY( __VA_ARGS__))(Type, TypeCased, cat, __VA_ARGS__)


// No format argument
#define _QI_LOG_MESSAGE_STREAM_HASCAT_HASFORMAT_1(Type, TypeCased, cat, ...) \
  BOOST_PP_CAT(_qiLog,TypeCased)(cat)

// Format argument
#define _QI_LOG_MESSAGE_STREAM_HASCAT_HASFORMAT_0(Type, TypeCased, cat, ...) \
   BOOST_PP_CAT(_qiLog, TypeCased)(cat, _QI_LOG_FORMAT(__VA_ARGS__))


/* Detecting empty arg is tricky.
 * Trick 1 below does not work with gcc, because  x ## "foo" produces a preprocessor error.
 * Trick 2 rely on ##__VA_ARGS__
*/
#ifdef _MSC_VER

#define _WQI_IS_EMPTY_HELPER___ a,b
#define WQI_IS_EMPTY(a,...) QI_CAT_20(QI_LIST_VASIZE,((QI_CAT_22(_WQI_IS_EMPTY_HELPER, QI_CAT_24(QI_CAT_26(_, a), _)))))

#define _QI_FIRST_ARG(a, ...) a
#define _QI_LOG_ISEMPTY(...) WQI_IS_EMPTY(QI_CAT_18(_, _QI_FIRST_ARG(__VA_ARGS__, 12)))

#else

#define _QI_LOG_REVERSE 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 0, 0
#define _QI_LOG_REVERSEEMPTY 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1
#define _QI_LOG_ARGN(a, b, c, d, e, f, g, h, i, N, ...) N
#define _QI_LOG_NARG_(dummy, ...) _QI_LOG_ARGN(__VA_ARGS__)
#define _QI_LOG_NARG(...) _QI_LOG_NARG_(dummy, ##__VA_ARGS__, _QI_LOG_REVERSE)
#define _QI_LOG_ISEMPTY(...) _QI_LOG_NARG_(dummy, ##__VA_ARGS__, _QI_LOG_REVERSEEMPTY)

#endif

namespace qi {
  namespace log{

    namespace detail {
      // Used to remove warning "statement has no effect"
      inline bool qiFalse() {return false;}
      class NullStream {
      public:
        NullStream(...)
        {
        }

        NullStream &self()
        {
          return *this;
        }
        template <typename T>
        NullStream& operator<<(const T &QI_UNUSED(val))
        {
          return self();
        }

        NullStream& operator<<(std::ostream& (*QI_UNUSED(f))(std::ostream&))
        {
          return self();
        }
      };
      // Hack required to silence spurious warning in compile-time disabled macros
      // We need an operator with priority below << and above &&
      inline bool operator < (bool b, const NullStream& ns)
      {
        return false;
      }

      struct Category
      {
        std::string name;
        LogLevel mainLevel;
        std::vector<LogLevel> levels;
      };
      QI_API boost::format getFormat(const std::string& s);
      /// @return a pointer to the global loglevel setting
      QI_API LogLevel* globalLogLevelPtr();
      // We will end up with one instance per module, but we don't care
      inline LogLevel globalLogLevel()
      {
        static LogLevel* l = globalLogLevelPtr();
        return *l;
      }
      inline bool isVisible(Category* category, LogLevel level)
      {
        return level <= globalLogLevel() && level <= category->mainLevel;
      }
    }
    typedef detail::Category* category_type;
    class LogStream: public std::stringstream
    {
    public:
      LogStream(const LogLevel    level,
                const char        *file,
                const char        *function,
                const int         line,
                const char        *category)
        : _logLevel(level)
        , _category(category)
        , _categoryType(0)
        , _file(file)
        , _function(function)
        , _line(line)
      {
      }
      LogStream(const LogLevel    level,
                const char        *file,
                const char        *function,
                const int         line,
                category_type     category)
        : _logLevel(level)
        , _category(0)
        , _categoryType(category)
        , _file(file)
        , _function(function)
        , _line(line)
      {
      }
      LogStream(const LogLevel    level,
                const char        *file,
                const char        *function,
                const int         line,
                const char        *category,
                const std::string& message)
        : _logLevel(level)
        , _category(category)
        , _categoryType(0)
        , _file(file)
        , _function(function)
        , _line(line)
      {
        *this << message;
      }

      ~LogStream()
      {
        if (!this->str().empty())
        {
          if (_category)
            qi::log::log(_logLevel, _category, this->str().c_str(), _file, _function, _line);
          else
            qi::log::log(_logLevel, _categoryType, this->str(), _file, _function, _line);
        }
      }

      LogStream& self() {
        return *this;
      }

    private:
      LogLevel    _logLevel;
      const char *_category;
      category_type _categoryType;
      const char *_file;
      const char *_function;
      int         _line;

      //avoid copy
      LogStream(const LogStream &rhs);
      LogStream &operator=(const LogStream &rhs);
    };
  }
}

#endif
