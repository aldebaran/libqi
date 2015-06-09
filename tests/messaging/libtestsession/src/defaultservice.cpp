/*
**
** Author(s):
**  - Pierre Roullon <proullon@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <qi/os.hpp>
#include "defaultservice.hpp"

#include <qi/type/dynamicobjectbuilder.hpp>
#include "defaultservice.hpp"

std::string __test_ping()
{
  return "pong";
}

qi::AnyObject DefaultService::getDefaultService()
{
  qi::DynamicObjectBuilder ob;
  ob.advertiseMethod("ping", &__test_ping);
  return qi::AnyObject(ob.object());
}

bool DefaultService::generateUniqueServiceName(std::string &name)
{
  std::stringstream ss;
  ss << "__default" << qi::SystemClock::now().time_since_epoch().count()/1000;
  name = ss.str();
  return true;
}
