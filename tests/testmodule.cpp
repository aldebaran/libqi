/*
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <qi/application.hpp>
#include <qitype/genericobject.hpp>
#include <qitype/genericobjectbuilder.hpp>
#include <qitype/objectfactory.hpp>
#include <qimessaging/session.hpp>

qiLogCategory("testmodule");

int testMethod(const int& v)
{
  return v+1;
}

qi::ObjectPtr make_other_object()
{
  qi::GenericObjectBuilder ob;
  ob.advertiseMethod("testMethod", testMethod);
  return ob.object();
}

qi::ObjectPtr setup(const std::string&)
{
  qiLogDebug() << "setup";
  qi::GenericObjectBuilder ob;
  ob.advertiseMethod("testMethod", testMethod);
  ob.advertiseMethod("make_other_object", make_other_object);
  return ob.object();
}

QI_REGISTER_OBJECT_FACTORY("test", &setup);
