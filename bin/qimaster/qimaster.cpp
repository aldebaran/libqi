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
  std::cout << "qimaster [address]" << std::endl;
  exit(1);
}

int main(int argc, char *argv[])
{
  std::string masterAddress = "127.0.0.1:5555";
  if (argc > 2)
    usage();
  if (argc == 2)
    masterAddress = argv[1];
  qi::Master master(masterAddress);

  while (1)
    sleep(1);
  return 0;
}
