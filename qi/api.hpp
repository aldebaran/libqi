/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#ifndef   __QI_API_HPP__
#define   __QI_API_HPP__

// qi_EXPORTS controls which symbols are exported when
// compiled as a SHARED lib.
#ifdef qi_EXPORTS
# if defined _WIN32 || defined __CYGWIN__
#   define QIAPI __declspec(dllexport)
# elif __GNUC__ >= 4
#   define QIAPI __attribute__ ((visibility("default")))
# else
#   define QIAPI
# endif
#else
# if defined _WIN32 || defined __CYGWIN__
#   if defined _WINDLL
#     define QIAPI __declspec(dllimport)
#   else
#     define QIAPI
#   endif
# elif __GNUC__ >= 4
#   define QIAPI __attribute__ ((visibility("default")))
# else
#   define QIAPI
# endif
#endif

// Deprecated
#if defined(__GNUC__)
#define QIAPI_DEPRECATED __attribute__((deprecated))
#else
#define QIAPI_DEPRECATED
#endif

#endif // __QI_API_HPP__
