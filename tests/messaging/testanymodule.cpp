/*
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <qi/application.hpp>
#include <qi/anyobject.hpp>
#include <qi/type/dynamicobjectbuilder.hpp>
#include <qi/session.hpp>
#include <qi/anymodule.hpp>

qiLogCategory("testanymodule");

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

int func(qi::SessionPtr session, int val)
{
  session->services(); // segfault?
  return val*2;
}

void init_naoqitestmodule_module(qi::ModuleBuilder* mb) {
  mb->advertiseMethod("test", &setup);
  mb->advertiseMethod("func", &func);
}

QI_REGISTER_MODULE("naoqi.testanymodule", &init_naoqitestmodule_module);
