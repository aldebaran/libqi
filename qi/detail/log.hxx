#pragma once

/*
 * Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

#ifndef _QI_DETAIL_LOG_HXX_
#define _QI_DETAIL_LOG_HXX_

#include <boost/format.hpp>
#include <boost/noncopyable.hpp>
#include <boost/preprocessor/cat.hpp>
#include <boost/locale/encoding_utf.hpp>

// #include <locale>  TODO: Use these includes when they become available on all platforms,
// #include <codecvt> instead of replaced by boost.locale
#include <type_traits>

#include <ka/typetraits.hpp>

#if defined(NO_QI_LOG_DETAILED_CONTEXT)
#   define _qiLogDebug(...)      qi::log::LogStream(qi::LogLevel_Debug, "", __FUNCTION__, 0, __VA_ARGS__).self()
#else
#   define _qiLogDebug(...)      qi::log::LogStream(qi::LogLevel_Debug, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__).self()
#endif

#if defined(NO_QI_LOG_DETAILED_CONTEXT)
# define _qiLogVerbose(...)      qi::log::LogStream(qi::LogLevel_Verbose, "", __FUNCTION__, 0, __VA_ARGS__).self()
#else
# define _qiLogVerbose(...)      qi::log::LogStream(qi::LogLevel_Verbose, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__).self()
#endif

#if defined(NO_QI_LOG_DETAILED_CONTEXT)
# define _qiLogInfo(...)         qi::log::LogStream(qi::LogLevel_Info, "", __FUNCTION__, 0, __VA_ARGS__).self()
#else
# define _qiLogInfo(...)         qi::log::LogStream(qi::LogLevel_Info, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__).self()
#endif

#if defined(NO_QI_LOG_DETAILED_CONTEXT)
# define _qiLogWarning(...)      qi::log::LogStream(qi::LogLevel_Warning, "", __FUNCTION__, 0, __VA_ARGS__).self()
#else
# define _qiLogWarning(...)      qi::log::LogStream(qi::LogLevel_Warning, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__).self()
#endif

#if defined(NO_QI_LOG_DETAILED_CONTEXT)
# define _qiLogError(...)        qi::log::LogStream(qi::LogLevel_Error, "", __FUNCTION__, 0, __VA_ARGS__).self()
#else
# define _qiLogError(...)        qi::log::LogStream(qi::LogLevel_Error, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__).self()
#endif

#if defined(NO_QI_LOG_DETAILED_CONTEXT)
# define _qiLogFatal(...)        qi::log::LogStream(qi::LogLevel_Fatal, "", __FUNCTION__, 0, __VA_ARGS__).self()
#else
# define _qiLogFatal(...)        qi::log::LogStream(qi::LogLevel_Fatal, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__).self()
#endif


#  define _QI_FORMAT_ELEM(_, a, elem) % (elem)

#  define _QI_LOG_FORMAT(Msg, ...)                   \
  QI_CAT(_QI_LOG_FORMAT_HASARG_, _QI_LOG_ISEMPTY(__VA_ARGS__))(Msg, __VA_ARGS__)

#define _QI_LOG_FORMAT_HASARG_0(Msg, ...) \
  boost::str(::qi::log::detail::getFormat(Msg)  QI_VAARGS_APPLY(_QI_FORMAT_ELEM, _, __VA_ARGS__ /**/))

#define _QI_LOG_FORMAT_HASARG_1(Msg, ...) Msg

#define _QI_SECOND(a, ...) __VA_ARGS__

/* For fast category access, we use lookup to a fixed name symbol.
 * The user is required to call qiLogCategory somewhere in scope.
 *
 * _QI_LOG_VARIABLE_SUFFIX is used to make variable name (_qi_log_category)
 * unique when using unity(blob) builds
 */
#ifndef _QI_LOG_VARIABLE_SUFFIX
# define _QI_LOG_VARIABLE_SUFFIX _x // dummy/default suffix
#endif

#  define _QI_LOG_CATEGORY_GET() BOOST_PP_CAT(_qi_log_category, _QI_LOG_VARIABLE_SUFFIX)

#if defined(NO_QI_LOG_DETAILED_CONTEXT) || defined(NDEBUG)
#  define _QI_LOG_MESSAGE(Type, Message)                        \
  do                                                            \
  {                                                             \
    if (::qi::log::isVisible(_QI_LOG_CATEGORY_GET(), ::qi::Type))  \
      ::qi::log::log(::qi::Type,                                \
                         _QI_LOG_CATEGORY_GET(),                \
                         Message,                               \
                         "", __FUNCTION__, 0);                  \
  }                                                             \
  while (false)
#else
#  define _QI_LOG_MESSAGE(Type, Message)                        \
  do                                                            \
  {                                                             \
    if (::qi::log::isVisible(_QI_LOG_CATEGORY_GET(), ::qi::Type))  \
      ::qi::log::log(::qi::Type,                                \
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
  ::qi::log::isVisible(_QI_LOG_CATEGORY_GET(), ::qi::Type) \
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

namespace qi { namespace log{ namespace detail {
// This type has no definition on purpose.
class Secret;
}}}

// Ensures that there is at least one operator<< in the global namespace.
// This is needed by the function template<T> LogStream& operator<<(LogStream&, T&&)
// Thanks to GoogleTest devs for the idea
void operator<<(const qi::log::detail::Secret&, int);

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
      inline bool operator<(bool /*b*/, const NullStream& /*ns*/)
      {
        return false;
      }

      struct Category
      {
        Category()
          : maxLevel(qi::LogLevel_Silent)
        {}

        Category(const std::string &name)
          : name(name)
          , maxLevel(qi::LogLevel_Silent)
        {}

        std::string               name;
        qi::LogLevel              maxLevel; //max level among all subscribers
        std::vector<qi::LogLevel> levels;   //level by subscribers

        void setLevel(SubscriberId sub, qi::LogLevel level);
      };

      QI_API boost::format getFormat(const std::string& s);

      // given a set of rules in the format documented in the public header,
      // return a list of (category name, LogLevel) tuples.
      QI_API std::vector<std::tuple<std::string, qi::LogLevel>> parseFilterRules(
          const std::string &rules);

      template <typename T, typename = ka::EnableIf<!std::is_convertible<T, std::wstring>::value>>
      T&& narrow(T&& t)
      {
        return std::forward<T>(t);
      }

      inline std::string narrow(std::wstring const& str)
      {
        // Use the std version when locale and codecvt become available.
        // return std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t>().to_bytes(str);
        return boost::locale::conv::utf_to_utf<char>(str.c_str(), str.c_str() + str.size());
      }
    } // namespace detail

    //inlined for perf
    inline bool isVisible(CategoryType category, qi::LogLevel level)
    {
      return category && level <= category->maxLevel;
    }

    using CategoryType = detail::Category*;

    class LogStream
    {
    public:

      LogStream(const LogStream&) = delete;
      LogStream& operator=(const LogStream&) = delete;

      LogStream(const qi::LogLevel level,
                const char         *file,
                const char         *function,
                const int          line,
                const char         *category)
        : _logLevel(level)
        , _category(category)
        , _categoryType(0)
        , _file(file)
        , _function(function)
        , _line(line)
      {
      }
      LogStream(const qi::LogLevel level,
                const char         *file,
                const char         *function,
                const int          line,
                CategoryType       category)
        : _logLevel(level)
        , _category(0)
        , _categoryType(category)
        , _file(file)
        , _function(function)
        , _line(line)
      {
      }
      LogStream(const qi::LogLevel  level,
                const char         *file,
                const char         *function,
                const int           line,
                const char         *category,
                const std::string&  message)
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
        if (_category)
          qi::log::log(_logLevel, _category, this->str().c_str(), _file, _function, _line);
        else
          qi::log::log(_logLevel, _categoryType, this->str(), _file, _function, _line);
      }

      LogStream& self() { return *this; }

      /* Here we provide a minimal interface so that LogStream behaves as an ostream:
       * str() to extract the internal string,
       * bool() is used to check the validity of the stream
       * operator<< is used to insert a parameter in the stream
       */
      inline std::string str() const
      {
        return _oss.str();
      }

      explicit inline operator bool() const
      {
        return static_cast<bool>(_oss);
      }

      template <typename T>
      friend LogStream& operator<<(LogStream& l, T&& t)
      {
        // To allow all the types defined in NAOqi for which a operator<< for std::ostream is
        // defined in the global namespace to be used in this function, declare that we use the
        // overload in the global namespace.
        using ::operator<<;
        l._oss << detail::narrow(std::forward<T>(t));
        return l;
      }

      /* overload for std::endl and other manipulators */
      friend inline LogStream& operator<<(LogStream& l, std::ostream& (*pf) (std::ostream&))
      {
        l._oss << pf;
        return l;
      }

    private:

      std::ostringstream _oss;
      qi::LogLevel  _logLevel;
      const char   *_category;
      CategoryType  _categoryType;
      const char   *_file;
      const char   *_function;
      int           _line;

    };
  }
}

#endif  // _QI_DETAIL_LOG_HXX_
