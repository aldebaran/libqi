/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#include <iostream>
#include <string>
#include <map>
#include <qi/messaging.hpp>
#include <qi/perf/sleep.hpp>

void usage()
{
  std::cout << "qi [address]" << std::endl;
}

void qi_call(std::string addr) {
  qi::Client client("qi client", addr);

  //client.call("master.listServices::{ss}")

  typedef std::map<std::string, std::string>  StringMap;
  StringMap                 mymap;
  StringMap::const_iterator it;

  mymap = client.call< StringMap >("master.listServices");

  for (it = mymap.begin(); it != mymap.end(); ++it)
  {
    std::cout << it->first << " :" << it->second << std::endl;
  }
//  std::string locatethismethod("master.listServices::{ss}:");
//  std::string result = client.call<std::string>("master.locateService", locatethismethod);
//  std::cout << "result:" << result << std::endl;
}

int main(int argc, char *argv[])
{
  std::string masterAddress;
  if (argc > 2)
    usage();
  if (argc == 2)
    masterAddress = argv[1];
  else
    masterAddress = "127.0.0.1:5555";

  qi_call(masterAddress);
  return 0;
}
