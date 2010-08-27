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
#include <alcommon-ng/tools/dataperftimer.hpp>


static const int gLoopCount   = 1000000;

using AL::Messaging::CallDefinition;
using AL::Messaging::ResultDefinition;

class NameLookup {
public:
  void bind(const std::string &name, AL::Functor *functor) {
    _map[name] = functor;
  }

  AL::Functor *get(const std::string &name)
  {
    return _map[name];
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
  void voidCall() { return; }
  int intStringCall(const std::string &plouf) { return plouf.size(); }
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

  cd.args().push_back(40);
  AL::makeFunctor(&chiche, &Chiche::lover)->call(cd, res);

  nl.get("lover")->call(cd, res);
}

TEST(TestBind, VoidCallPerf) {
  Chiche           chiche;
  Chiche          *p = &chiche;
  NameLookup       nl;
  ResultDefinition res;
  CallDefinition   cd;

  AL::Test::DataPerfTimer dp;
  AL::Functor    *functor = AL::makeFunctor(&chiche, &Chiche::voidCall);
  std::cout << "AL::Functor call" << std::endl;
  dp.start(gLoopCount);
  for (int i = 0; i < gLoopCount; ++i)
  {
    functor->call(cd, res);
  }
  dp.stop();

  std::cout << "pointer call" << std::endl;
  dp.start(gLoopCount);
  for (int i = 0; i < gLoopCount; ++i)
  {
    p->voidCall();
  }
  dp.stop();
}

TEST(TestBind, IntStringCallPerf) {
  Chiche           chiche;
  Chiche          *p = &chiche;
  NameLookup       nl;
  ResultDefinition res;

  AL::Test::DataPerfTimer dp;

  std::cout << "AL::Functor call (string with a growing size)" << std::endl;

  for (int i = 0; i < 12; ++i)
  {
    unsigned int    numBytes = (unsigned int)pow(2.0f,(int)i);
    std::string     request = std::string(numBytes, 'B');
    CallDefinition  cd;
    AL::Functor    *functor = AL::makeFunctor(&chiche, &Chiche::intStringCall);

    cd.args().push_back(request);
    dp.start(gLoopCount, numBytes);
    for (int j = 0; j < gLoopCount; ++j) {
      functor->call(cd, res);
    }
    dp.stop();
  }

  std::cout << "pointer call (string with a growing size)" << std::endl;
  for (int i = 0; i < 12; ++i)
  {
    unsigned int    numBytes = (unsigned int)pow(2.0f,(int)i);
    std::string     request = std::string(numBytes, 'B');

    dp.start(gLoopCount, numBytes);
    for (int j = 0; j < gLoopCount; ++j) {
      p->intStringCall(request);
    }
    dp.stop();
  }

}
