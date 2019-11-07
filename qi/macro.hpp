#pragma once
/*
 * Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

/**
 * \file
 * \brief Various macros for qi. (deprecated, export API, disallow copy, ..)
 * \includename{qi/macro.hpp}
 * \verbatim
 * This header file contains various macros for qi.
 *
 * - import/export symbol (:cpp:macro:`QI_IMPORT_API`,
 *   :cpp:macro:`QI_EXPORT_API`)
 * - mark function and header as deprecated (:cpp:macro:`QI_DEPRECATED_HEADER`,
 *   :cpp:macro:`QI_API_DEPRECATED`)
 * - generate compiler warning (:cpp:macro:`QI_COMPILER_WARNING`)
 * - disallow copy and assign (:cpp:macro:`QI_DISALLOW_COPY_AND_ASSIGN`)
 * \endverbatim
 */

#ifndef _QI_MACRO_HPP_
# define _QI_MACRO_HPP_

#include <qi/preproc.hpp>
#include <ka/macro.hpp>
#include <boost/predef/compiler.h>
#include <boost/config.hpp>

/**
 * \def QI_API_DEPRECATED
 * \brief Compiler flags to mark a function as deprecated. It will generate a
 *        compiler warning.
 */
#if defined(__GNUC__) && !defined(QI_NO_API_DEPRECATED)
#  define QI_API_DEPRECATED __attribute__((deprecated))
#elif defined(_MSC_VER) && !defined(QI_NO_API_DEPRECATED)
#  define QI_API_DEPRECATED __declspec(deprecated)
#else
#  define QI_API_DEPRECATED
#endif

 /**
 * \def QI_API_DEPRECATED_MSG(msg__)
 * \brief Compiler flags to mark a function as deprecated. It will generate a
 *        compiler warning.
 * \param msg__  A message providing a workaround.
 */
#if defined(__GNUC__) && !defined(QI_NO_API_DEPRECATED)
#  define QI_API_DEPRECATED_MSG(msg__) __attribute__((deprecated(#msg__)))
#elif defined(_MSC_VER) && !defined(QI_NO_API_DEPRECATED)
#  define QI_API_DEPRECATED_MSG(msg__) __declspec(deprecated(#msg__))
#else
#  define QI_API_DEPRECATED_MSG(msg__)
#endif

/**
 * \def QI_NORETURN
 * \brief Portable noreturn attribute, used to declare that a function does not return
 */
#if defined(__GNUC__)
#  define QI_NORETURN   __attribute__((noreturn))
#elif defined(_MSC_VER)
/// Portable noreturn attribute, used to declare that a function does not return
#  define QI_NORETURN   __declspec(noreturn)
#else
#  define QI_NORETURN
#endif

/**
 * \def QI_HAS_VARIABLE_LENGTH_ARRAY
 * \brief Mark compilers supporting variable length array (VLA)
 */
#if defined(__GNUC__) && !defined(__clang__)
#  define QI_HAS_VARIABLE_LENGTH_ARRAY 1
#else
#  define QI_HAS_VARIABLE_LENGTH_ARRAY 0
#endif

// For shared library


/**
 * \return the proper type specification for import/export
 * \param libname the name of your library.
 * This macro will use two preprocessor defines:
 * libname_EXPORTS (cmake convention) and libname_STATIC_BUILD.
 * Those macro can be unset or set to 0 to mean false, or set to empty or 1 to
 * mean true.
 * The first one must be true if the current compilation unit is within the library.
 * The second must be true if the library was built as a static archive.
 * The proper way to use this macro is to:
 * - Have your buildsystem set mylib_EXPORTS when building MYLIB
 * - Have your buildsystem produce a config.h file that
 * \#define mylib_STATIC_BUILD to 1 or empty if it is a static build, and
 * not define mylib_STATIC_BUILD or define it to 0 otherwise
 * In one header, write
 *   \#include <mylib/config.h>
 *   \#define MYLIB_API QI_LIB_API(mylib)
 */
#define QI_LIB_API(libname) QI_LIB_API_(BOOST_PP_CAT(libname, _EXPORTS), BOOST_PP_CAT(libname, _STATIC_BUILD))

#define QI_LIB_API_(IS_BUILDING_LIB, IS_LIB_STATIC_BUILD)               \
  QI_LIB_API_NORMALIZED(_QI_IS_ONE_OR_EMPTY(BOOST_PP_CAT(_ , IS_BUILDING_LIB)), _QI_IS_ONE_OR_EMPTY(BOOST_PP_CAT(_, IS_LIB_STATIC_BUILD)))

/**
 * \def QI_IMPORT_API
 * \brief Compiler flags to import a function or a class.
 */

/**
 * \def QI_EXPORT_API
 * \brief Compiler flags to export a function or a class.
 */

/**
 * \def QI_LIB_API_NORMALIZED(a, b)
 * \brief Each platform must provide a QI_LIB_API_NORMALIZED(isBuilding, isStatic)
 */
#if defined _WIN32 || defined __CYGWIN__
#  define QI_EXPORT_API __declspec(dllexport)
#  define QI_IMPORT_API __declspec(dllimport)
#  define QI_LIB_API_NORMALIZED(exporting, isstatic) BOOST_PP_CAT(BOOST_PP_CAT(_QI_LIB_API_NORMALIZED_, exporting), isstatic)
#  define _QI_LIB_API_NORMALIZED_00 QI_IMPORT_API
#  define _QI_LIB_API_NORMALIZED_10 QI_EXPORT_API
#  define _QI_LIB_API_NORMALIZED_11
#  define _QI_LIB_API_NORMALIZED_01
#elif __GNUC__ >= 4
#  define QI_EXPORT_API __attribute__ ((visibility("default")))
#  define QI_IMPORT_API QI_EXPORT_API
#  define QI_LIB_API_NORMALIZED(a, b) QI_EXPORT_API
#else
#  define QI_IMPORT_API
#  define QI_EXPORT_API
#  define QI_LIB_API_NORMALIZED(a, b)
#endif


//! \cond internal
// Macros adapted from opencv2.2
#define QI_DO_PRAGMA(x) KA_PRAGMA(x)
#define QI_MSG_PRAGMA(msg) KA_PRAGMA_MESSAGE(msg)
//! \endcond


/**
 * \def QI_COMPILER_WARNING(x)
 * \brief Generate a compiler warning.
 * \param x The string displayed as the warning.
 */
#ifndef QI_NO_COMPILER_WARNING
#  define QI_COMPILER_WARNING(x) KA_WARNING(x)
#endif

/**
 * \def QI_DEPRECATED_HEADER(x)
 * \brief Generate a compiler warning stating a header is deprecated.
 * add a message to explain what user should do
 */
#if !defined(WITH_DEPRECATED) || defined(QI_NO_DEPRECATED_HEADER)
# define QI_DEPRECATED_HEADER(x)
#else
# define QI_DEPRECATED_HEADER(x) KA_PRAGMA_MESSAGE("\
This file includes at least one deprecated or antiquated ALDEBARAN header \
which may be removed without further notice in the next version. \
Please consult the changelog for details. " #x)
#endif


#ifdef __cplusplus
namespace qi {
  template <typename T>
  struct IsClonable;
};
#endif

/**
 * \brief A macro used to deprecate another macro. Generate a compiler warning
 * when the given macro is used.
 * \param name The name of the macro.
 * \verbatim
 * Example:
 *
 * .. code-block:: cpp
 *
 *     #define MAX(x,y)(QI_DEPRECATE_MACRO(MAX), x > y ?  x : y)
 * \endverbatim
 */
#define QI_DEPRECATE_MACRO(name)                        \
  QI_COMPILER_WARNING(name macro is deprecated.)

/**
 * \deprecated Use boost::noncopyable instead.
 *
 * \verbatim
 * Example:
 *
 * .. code-block:: cpp
 *
 *     class Foo : private boost::nonpyable
 *     {};
 * \endverbatim
 *
 * \brief A macro to disallow copy constructor and operator=
 * \param type The class name of which we want to forbid copy.
 *
 * \verbatim
 * .. note::
 *     This macro should always be in the private (or protected) section of a
 *     class.
 *
 * Example:
 *
 * .. code-block:: cpp
 *
 *     class Foo {
 *         Foo();
 *     private:
 *         QI_DISALLOW_COPY_AND_ASSIGN(Foo);
 *     };
 * \endverbatim
 */
#define QI_DISALLOW_COPY_AND_ASSIGN(type)               \
  QI_DEPRECATE_MACRO(QI_DISALLOW_COPY_AND_ASSIGN)       \
  type(type const &);                                   \
  void operator=(type const &);                         \
  using _qi_not_clonable = int;                         \
  template<typename U> friend struct ::qi::IsClonable


/**
 * \def QI_WARN_UNUSED_RESULT
 * \brief Warns if the return value of the function is discarded.
 *
 * If a function declared QI_WARN_UNUSED_RESULT is called from a discarded-value expression other
 * than a cast to void, the compiler is encouraged to issue a warning.
 */
#if defined(__GNUC__)
# define QI_WARN_UNUSED_RESULT __attribute__((warn_unused_result))
#else
# define QI_WARN_UNUSED_RESULT
#endif

/**
 * \def QI_ATTR_UNUSED
 * \brief This macro tags a attribute as unused.
 */
#if defined(__GNUC__)
# define QI_ATTR_UNUSED __attribute__((unused))
#else
# define QI_ATTR_UNUSED
#endif

/**
 * \brief This macro tags a parameter as unused.
 *
 * \verbatim
 * Example:
 *
 * .. code-block:: cpp
 *
 *     int zero(int QI_UNUSED(x))
 *     {
 *       return 0;
 *     }
 * \endverbatim
 */
#define QI_UNUSED(x)

/**
 * This macro prevents the compiler from emitting warning when a variable is defined but not used.
 *
 * Note: You may not use this macro to declare that a function parameters is unused. For such uses,
 * see QI_UNUSED.
 *
 * \verbatim
 * Example:
 *
 * .. code-block:: cpp
 *
 *     void foo(std::vector<int> vec)
 *     {
 *       auto size = vec.size();
 *       // QI_ASSERT expands to nothing if debug informations are disabled in the compilation
 *       // configuration, which would make the `size` variable unused.
 *       QI_IGNORE_UNUSED(size);
 *       QI_ASSERT(size > 2);
 *     }
 * \endverbatim
 */
#define QI_IGNORE_UNUSED(x) (void)x

/**
 * \def QI_UNIQ_DEF(A)
 * \brief A macro to append the line number of the parent macro usage, to define a
 *        function in or a variable and avoid name collision.
 */
#define QI_UNIQ_DEF_LEVEL2_(A, B) A ## __uniq__ ## B
#define QI_UNIQ_DEF_LEVEL1_(A, B) QI_UNIQ_DEF_LEVEL2_(A, B)
#define QI_UNIQ_DEF(A) QI_UNIQ_DEF_LEVEL1_(A, __LINE__)

/**
 * \def QI_NOEXCEPT(cond)
 * \brief Specify that a function may throw or not. Do nothing if noexcept is not available.
 *
 */
#define QI_NOEXCEPT(cond) BOOST_NOEXCEPT_IF(cond)

/**
 * \def QI_NOEXCEPT_EXPR(expr)
 * \brief Specify that a function may throw if the given expression may throw.
 *        Do nothing if noexcept is not available.
 */
#define QI_NOEXCEPT_EXPR(expr) BOOST_NOEXCEPT_IF(BOOST_NOEXCEPT_EXPR(expr))


/**
 * \def QI_WARNING_PUSH()
 * \brief Pushes the current state of warning flags so it can be retrieved later with QI_WARNING_POP.
 */
#define QI_WARNING_PUSH KA_WARNING_PUSH

/**
 * \def QI_WARNING_DISABLE(msvcCode, gccName)
 * \brief Disables the warning that is identified by msvcCode under MSVC or gccName under GCC and
 * Clang.
 */
#define QI_WARNING_DISABLE KA_WARNING_DISABLE

/**
 * \def QI_WARNING_POP()
 * \brief Pops the last state of warning flags that was pushed with QI_WARNING_PUSH().
 */
#define QI_WARNING_POP KA_WARNING_POP

/**
 * \def QI_FALLTHROUGH
 * \brief Declares that the current case in a switch falls through the next case. It is mandatory to
 *        append a semicolon after this macro.
 */
#if defined(__has_cpp_attribute) && __cplusplus >= 201703L
#  if __has_cpp_attribute(fallthrough)
#    define QI_FALLTHROUGH [[fallthrough]]
#  endif
#endif
#ifndef QI_FALLTHROUGH
#  if __GNUC__ >= 7
#    define QI_FALLTHROUGH __attribute__((fallthrough))
#  elif defined(__clang__) && __cplusplus >= 201103L && \
    defined(__has_feature) && defined(__has_warning)
#    if __has_feature(cxx_attributes) && __has_warning("-Wimplicit-fallthrough")
#      define QI_FALLTHROUGH [[clang::fallthrough]]
#    endif
#  endif
#endif
#ifndef QI_FALLTHROUGH
#  define QI_FALLTHROUGH ((void)0)
#endif

#endif  // _QI_MACRO_HPP_
