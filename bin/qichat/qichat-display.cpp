/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#include <signal.h>
#include <iostream>
#include <qi/messaging/subscriber.hpp>
#include <qi/perf/sleep.hpp>
#include <boost/program_options.hpp>

namespace po = boost::program_options;

bool gContinue;

void exitHandler(int sig) {
  // Trap CTRL-C
  // The sleep loop will exit cleanly
  gContinue = false;
}

void displayHandler(const std::string& text) {
  std::cout << text;
}

void qichatdisplay(const std::string& addr) {
  qi::Subscriber subscriber("qichat-display");
  subscriber.connect(addr);
  subscriber.subscribe("qichat", &displayHandler);

  std::cout <<
    "============== qichat-display =============" << std::endl <<
    "== Press CTRL-C to quit."                    << std::endl <<
    "===========================================" << std::endl;

  // FIXME: find a nice way of sleeping
  while(gContinue) {
    sleep(1);
  }
  // CTRL-C was pressed
  std::cout << "===========================================" << std::endl;
}

int main(int argc, char *argv[])
{
  gContinue = true;
  (void) signal(SIGINT, exitHandler);
  // declare the program options
  po::options_description desc("Usage:\n  qichat-display [options]\nOptions");
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
      qichatdisplay(address);
    } else {
      std::cout << desc << "\n";
    }
  } catch (const boost::program_options::error&) {
    std::cout << desc << "\n";
  }

  return 0;
}
