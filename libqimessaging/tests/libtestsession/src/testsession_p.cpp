/*
**
** Author(s):
**  - Pierre Roullon <proullon@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include "testsession_p.hpp"

TestSessionPrivate::TestSessionPrivate(const std::string &serviceDirectoryUrl, TestMode::Mode mode, bool listen)
{
  _mode = mode;
  _session = new qi::Session();
  _manager = new SessionInitializer();
  _manager->setUp(_session, serviceDirectoryUrl, _mode, listen);
}

TestSessionPrivate::~TestSessionPrivate()
{
  _manager->tearDown(_session, _mode);
  delete _manager;
  delete _session;
}

qi::Session* TestSessionPrivate::session()
{
  return _session;
}
