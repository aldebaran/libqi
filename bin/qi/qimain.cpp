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
#include <boost/program_options.hpp>

namespace po = boost::program_options;

void qi_call(std::string addr) {
  qi::Client client("qi command line client", addr);
  if (!client.isInitialized()) {
    return;
  }
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
  // declare the program options
  po::options_description desc("Usage:\n  qi masterAddress [options]\nOptions");
  desc.add_options() 
    ("help", "Print this help.")
    ("master-address",
    po::value<std::string>()->default_value(std::string("127.0.0.1:5555")),
    "The master address");

  // allow master address to be specified as the first arg
  po::positional_options_description pos;
  pos.add("master-address", 1);

  // parse and store
  po::variables_map vm;
  try {
    po::store(po::command_line_parser(argc, argv).
      options(desc).positional(pos).run(), vm);
    po::notify(vm);

    if (vm.count("help")) {
      std::cout << desc << "\n";
      return 0;
    }

    if(vm.count("master-address")==1) {
      std::string address = vm["master-address"].as<std::string>();
      qi_call(address);
    } else {
      std::cout << desc << "\n";
    }
  } catch (const boost::program_options::error&) {
    std::cout << desc << "\n";
  }

  return 0;
}
