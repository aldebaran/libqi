/*
 * Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

/** @file
 *  @brief dll import/export and compiler message
 */

#pragma once
#ifndef _LIBQI_QI_API_HPP_
#define _LIBQI_QI_API_HPP_

// Deprecated
#if defined(__GNUC__) && defined(WITH_DEPRECATED) && !defined(QI_NO_API_DEPRECATED)
#  define QI_API_DEPRECATED __attribute__((deprecated))
#elif defined(_MSC_VER) && defined(WITH_DEPRECATED) && !defined(QI_NO_API_DEPRECATED)
#  define QI_API_DEPRECATED __declspec(deprecated)
#else
#  define QI_API_DEPRECATED
#endif

// For shared library
#if defined _WIN32 || defined __CYGWIN__
#  define QI_EXPORT_API __declspec(dllexport)
#  if defined _WINDLL
#    define QI_IMPORT_API __declspec(dllimport)
#  else
#    define QI_IMPORT_API
#  endif
#elif __GNUC__ >= 4
#  define QI_EXPORT_API __attribute__ ((visibility("default")))
#  define QI_IMPORT_API __attribute__ ((visibility("default")))
#else
#  define QI_EXPORT_API
#  define QI_IMPORT_API
#endif


//! \cond internal
// Macros adapted from opencv2.2
#if defined(_MSC_VER)
  #define QI_DO_PRAGMA(x) __pragma(x)
  #define __ALSTR2__(x) #x
  #define __ALSTR1__(x) __ALSTR2__(x)
  #define _ALMSVCLOC_ __FILE__ "("__ALSTR1__(__LINE__)") : "
  #define QI_MSG_PRAGMA(_msg) QI_DO_PRAGMA(message (_ALMSVCLOC_ _msg))
#elif defined(__GNUC__)
  #define QI_DO_PRAGMA(x) _Pragma (#x)
  #define QI_MSG_PRAGMA(_msg) QI_DO_PRAGMA(message (_msg))
#else
  #define QI_DO_PRAGMA(x)
  #define QI_MSG_PRAGMA(_msg)
#endif
//! \endcond


// Use this macro to generate compiler warnings.
#if !defined(WITH_DEPRECATED) || defined(QI_NO_COMPILER_WARNING)
# define QI_COMPILER_WARNING(x)
#else
# define QI_COMPILER_WARNING(x) QI_MSG_PRAGMA("Warning: " #x)
#endif

// Deprecate a header, add a message to explain what user should do
#if !defined(WITH_DEPRECATED) || defined(QI_NO_DEPRECATED_HEADER)
# define QI_DEPRECATED_HEADER(x)
#else
# define QI_DEPRECATED_HEADER(x) QI_MSG_PRAGMA("\
This file includes at least one deprecated or antiquated ALDEBARAN header \
which may be removed without further notice in the next version. \
Please consult the changelog for details. " #x)
#endif


// A macro to disallow copy constructor and operator=
#define QI_DISALLOW_COPY_AND_ASSIGN(type)       \
  type(type const &);                           \
  void operator=(type const &)

#if defined(__GNUC__)
# define QI_WARN_UNUSED_RESULT __attribute__((warn_unused_result))
#else
# define QI_WARN_UNUSED_RESULT
#endif

#define QI_UNUSED(x)

#endif  // _LIBQI_QI_API_HPP_

