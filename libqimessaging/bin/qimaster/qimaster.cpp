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
#include <boost/program_options.hpp>

namespace po = boost::program_options;

int main(int argc, char *argv[])
{
   // declare the program options
  po::options_description desc("Usage:\n  qi-master masterAddress [options]\nOptions");
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
      std::string masterAddress = vm["master-address"].as<std::string>();
      //qi::Context* context = new qi::Context();
      //qi::Master master(masterAddress, context);
      qi::Master master(masterAddress);
      master.run();
      if (master.isInitialized()) {
        while (1)
          sleep(1);
      }
    } else {
      std::cout << desc << "\n";
    }
  } catch (const boost::program_options::error&) {
    std::cout << desc << "\n";
  }

  return 0;
}
