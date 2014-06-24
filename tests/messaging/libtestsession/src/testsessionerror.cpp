/*
**
** Author(s):
**  - Pierre Roullon <proullon@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <testsession/testsession.hpp>

TestSessionError::TestSessionError(const std::string& what_arg) : std::runtime_error(what_arg)
{
}

TestSessionError::~TestSessionError() throw()
{
}
