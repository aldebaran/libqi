/*
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <qi/application.hpp>
#include <qimessaging/object.hpp>
#include <qimessaging/object_factory.hpp>
#include <qimessaging/session.hpp>

int testMethod(const int& v)
{
  return v+1;
}

qi::Object* setup(const std::string&)
{
  qiLogDebug("testmodule") << "setup";
  qi::ObjectBuilder ob;
  ob.advertiseMethod("testMethod", testMethod);
  qi::Object* e = new qi::Object(ob.object());
  return e;
}

QI_REGISTER_OBJECT_FACTORY("test", &setup);
