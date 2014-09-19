/*
**
** Copyright (C) 2012 Aldebaran Robotics
*/
#include <qi/session.hpp>
#include <qi/anymodule.hpp>

using namespace qi;

//the foo class taking a Session as parameter
class Foo {
public:
  Foo(const SessionPtr& session)
    : _session(session)
  {
    _session->service("ServiceDirectory"); // SEGV?
  }

  int bar()
  {
    _session->service("ServiceDirectory"); // SEGV?
    return 42;
  }

private:
  SessionPtr _session;
};
QI_REGISTER_OBJECT(Foo, bar);

//a simple egg function
void eggFun()
{
}

void register_foo_module(ModuleBuilder* mb)
{
  mb->advertiseMethod("egg", &eggFun);
  mb->advertiseFactory<Foo, const SessionPtr&>("Foo");
}
QI_REGISTER_MODULE("naoqi.testanymodulesession", &register_foo_module);
