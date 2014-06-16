/*
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <qi/application.hpp>
#include <qi/anyobject.hpp>
#include <qitype/dynamicobjectbuilder.hpp>
#include <qitype/objectfactory.hpp>
#include <qimessaging/session.hpp>

qiLogCategory("testmodule");

int testMethod(const int& v)
{
  return v+1;
}

qi::AnyObject make_other_object()
{
  qi::DynamicObjectBuilder ob;
  ob.advertiseMethod("testMethod", testMethod);
  return ob.object();
}

qi::AnyObject setup()
{
  qiLogDebug() << "setup";
  qi::DynamicObjectBuilder ob;
  ob.advertiseMethod("testMethod", testMethod);
  ob.advertiseMethod("make_other_object", make_other_object);
  return ob.object();
}

QI_REGISTER_OBJECT_FACTORY("test", qi::AnyFunction::from(&setup));
