/*
**
** Author(s):
**  - Pierre Roullon <proullon@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

/*!
 * \internal
 * \class TestSessionPrivate
 * \brief Private implementation of TestSession class.
 * \since 1.18
 * \author Pierre Roullon
 */

#ifndef _TESTS_LIBTESTSESSION_TESTSESSIONPRIVATE_HPP_
#define _TESTS_LIBTESTSESSION_TESTSESSIONPRIVATE_HPP_

#include <testsession/testsession.hpp>
#include "sessioninitializer.hpp"

class TestSessionPrivate
{
public:
  TestSessionPrivate(const std::string &serviceDirectoryUrl, TestMode::Mode mode, bool listen);
  ~TestSessionPrivate();

public:
  qi::SessionPtr       session();

private:
  TestMode::Mode      _mode;
  SessionInitializer  _manager;
  qi::SessionPtr      _session;
};

#endif // !_TESTS_LIBTESTSESSION_TESTSESSIONPRIVATE_HPP_
