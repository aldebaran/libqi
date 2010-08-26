/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#include <gtest/gtest.h>
#include <map>
#include <alcommon-ng/functor/functor.hpp>
#include <alcommon-ng/functor/makefunctor.hpp>
#include <alcommon-ng/serialization/call_definition.hpp>
#include <alcommon-ng/serialization/result_definition.hpp>

#define BIND_METHOD( x )  do { completeAndCheck(&x, fMethodDesc); bindMethod(createFunctor(this, &x)); } while (0);
#define BIND_METHOD_ASYNCHRONOUS( x )  do { completeAndCheck(&x, fMethodDesc); bindMethodAsynchronous(createFunctor(this, &x)); } while (0);

using AL::Messaging::CallDefinition;
using AL::Messaging::ResultDefinition;

class NameLookup {
public:
  void bind(const std::string &name, AL::Functor *functor) {
    _map[name] = functor;
  }

protected:
  std::map<std::string, AL::Functor *> _map;
};

int toto(int bim)
{
  std::cout << "poutre:" << bim << std::endl;
  return bim + 1;
}

struct Chiche {
  void tartine() { std::cout << "poutre la tartine" << std::endl; }
  void lover(const int &poteau) { std::cout << "poutre du poteau" << poteau << std::endl; }
};

TEST(TestBind, Simple) {
  Chiche     chiche;
  NameLookup nl;

  nl.bind("tartine", AL::makeFunctor(&chiche, &Chiche::tartine));
  nl.bind("lover",   AL::makeFunctor(&chiche, &Chiche::lover));

  ResultDefinition res;
  CallDefinition   cd;
  AL::makeFunctor(&chiche, &Chiche::tartine)->call(CallDefinition(), res);

  cd.push(40);
  AL::makeFunctor(&chiche, &Chiche::lover)->call(cd, res);

  //AL::createFunctor(&chiche, &Chiche::tartine)->as<void>();
  //nl.call("toto", 42);
}

