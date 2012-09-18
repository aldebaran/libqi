/*
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <qi/application.hpp>
#include <qimessaging/genericobject.hpp>
#include <qimessaging/objectbuilder.hpp>
#include <qimessaging/object_factory.hpp>
#include <qimessaging/session.hpp>

int testMethod(const int& v)
{
  return v+1;
}

qi::GenericObject setup(const std::string&)
{
  qiLogDebug("testmodule") << "setup";
  qi::DynamicObjectBuilder ob;
  ob.advertiseMethod("testMethod", testMethod);
  return ob.object();
}

QI_REGISTER_OBJECT_FACTORY("test", &setup);
