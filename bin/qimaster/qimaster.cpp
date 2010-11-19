/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#include <iostream>
#include <cstdlib>
#include <qi/messaging/master.hpp>
#include <qi/perf/sleep.hpp>

void usage()
{
  std::cout << "qimaster address" << std::endl;
  exit(1);
}

int main(int argc, char *argv[])
{
  if (argc < 2)
    usage();

  qi::Master master(argv[1]);

  while (1)
    sleep(1);
  return 0;
}
