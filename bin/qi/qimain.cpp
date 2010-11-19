/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#include <iostream>
#include <qi/messaging.hpp>
#include <qi/perf/sleep.hpp>

void usage()
{
  std::cout << "qimaster address" << std::endl;
}

void qi_call(const char *addr) {
  qi::Client client("clicli", std::string(addr));

  //client.call("master.listServices::{ss}")
  std::string locatethismethod("master.listServices::{ss}:");

  std::string result = client.call<std::string>("master.locateService", locatethismethod);
  std::cout << "result:" << result << std::endl;
}

int main(int argc, char *argv[])
{
  if (argc < 2)
    usage();

  qi_call(argv[1]);
  return 0;
}
