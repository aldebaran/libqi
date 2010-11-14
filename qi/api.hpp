/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#ifndef         QI_API_HPP_
# define        QI_API_HPP_

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
#   define QIAPI __declspec(dllimport)
# elif __GNUC__ >= 4
#   define QIAPI __attribute__ ((visibility("default")))
# else
#   define QIAPI
# endif
#endif

#endif    /* !QI_API_HPP_ */
