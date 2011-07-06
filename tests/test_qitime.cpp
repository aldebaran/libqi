/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2011 Aldebaran Robotics
*/

#include <iostream>

#include <qi/time.hpp>
#include <gtest/gtest.h>

TEST(qitime, testadd) {
  qi::time::duration d;
  qi::time::time     t;

  qi::time::point p1 = qi::time::now();
  qi::time::usleep(100);
  qi::time::point p2 = qi::time::now();
  d = p2 - p1;


  qi::time::sleep(qi::time::usec(32));
  qi::time::sleep(qi::time::sec(1));

  d += qi::time::msec(1000);
  d += qi::time::sec(12);
  std::cout << p1 << d << p2 << std::endl;
}


int main(int argc, char *argv[])
{
  qi::time::counter c;
  c.start();
  c.stop();

  qi::time::period p(qi::time::duration(1, 42000));
  p.set_period(42);
  p.init();
  for (int i = 0; i < 10; ++i)
  {
    p.wait(true);
  }

  qi::time::time   t;
  qi::time::point  p;
  qi::time::moment mo;
  qi::time::marker ma;
  qi::time::timepoint tp;
  qi::time::instant it;
  qi::time::timeval tv;


  qi::time::clock c;

  //weird
  try {
    c.setclock(qi::time::clock_realtime);
  } except {
    c.setclock(qi::time::clock_monotonic);
  };

  qi::time::point    p;
  qi::time::duration d;

  p = c.now();
  d = p - c.now();

  p = qi::time::now();

  //wallclock?
  std::cout << "stat:" << p << std::endl;
  return 0;
}

