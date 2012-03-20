/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#include <iostream>
#include "dataperftimer.hpp"
#include <boost/pool/pool.hpp>
#include <boost/pool/object_pool.hpp>

struct MyStruct {
  char b[3223];
  int x;
};


static const int gLoopCount = 10000000;
int main()
{
  qi::perf::DataPerfTimer dp;

  unsigned long long toto = 0;

  boost::object_pool<MyStruct> op;
  dp.start(gLoopCount);
  for (int i = 0; i < gLoopCount; ++i) {
    MyStruct *ms = op.malloc();
    ms->x = 2;
    toto += (unsigned long long)ms;
    op.free(ms);
  }
  dp.stop();
  //only need to use toto (to force the code used)
  std::cout << "toto:" << (int)toto << std::endl;

  dp.start(gLoopCount);
  for (int i = 0; i < gLoopCount; ++i) {
    MyStruct *ms = new MyStruct;
    ms->x = 2;
    toto += (unsigned long long)ms;
    free(ms);
  }
  dp.stop();
  //only need to use toto (to force the code used)
  std::cout << "toto:" << (int)toto << std::endl;

  return 0;
}

