/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#ifndef         QI_API_HPP_
# define        QI_API_HPP_

// :TODO: use __attribute__((visibility("hidden"))) on compatible platforms
#ifdef QI_EXPORTS
# ifdef _WIN32
#   define QIAPI __declspec(dllexport)
# else
#   define QIAPI
# endif
#else
# ifdef _WIN32
#   define QIAPI __declspec(dllimport)
# else
#   define QIAPI
# endif
#endif

#endif    /* !QI_API_HPP_ */
