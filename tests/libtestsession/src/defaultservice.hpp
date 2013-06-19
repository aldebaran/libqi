/*
**
** Author(s):
**  - Pierre Roullon <proullon@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

/*!
 * \internal
 * \fn __test_ping()
 * \brief ping method bound as "ping::s(v)" in default service.
 * \since 1.18
 * \author Pierre Roullon
 */

/*!
 * \internal
 * \class DefaultService
 * \brief Provide methods to create and allocate a pointer to a QiMessaging object with all default methods bound on it.
 * \since 1.18
 * \author Pierre Roullon
 */

/*!
 * \internal
 * \fn DefaultService::getDefaultService()
 * \brief Return a qi::AnyObject pointing to default QiMessaging service.
 * \since 1.18
 * \author Pierre Roullon
 */

#ifndef _TESTS_LIBTESTSESSION_DEFAULTSERVICE_HPP_
#define _TESTS_LIBTESTSESSION_DEFAULTSERVICE_HPP_

#include <qitype/genericobject.hpp>

std::string __test_ping();

class  DefaultService
{
public:
  static bool   generateUniqueServiceName(std::string &name);
  static qi::AnyObject getDefaultService();
};

#endif // !_TESTS_LIBTESTSESSION_DEFAULTSERVICE_HPP_
