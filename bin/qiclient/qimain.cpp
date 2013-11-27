/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010, 2012 Aldebaran Robotics
*/

#include <iostream>
#include <string>
#include <map>

#include <qi/os.hpp>
#include <qimessaging/applicationsession.hpp>

#include <boost/program_options.hpp>

namespace po = boost::program_options;

qiLogCategory("qiclient");

void call(int count, qi::Session& session)
{
  qi::AnyObject obj = session.service("serviceTest");

  for (int i = 0; i < count; ++i) {
    std::string result = obj.call<std::string>("reply", "plaf");
    std::cout << "result" << i << ":" << result << std::endl;
  }
}

void eventCb(const std::string &event) {
  qiLogInfo() << "Received event data: " << event;
}

void recEvent(qi::ApplicationSession& app) {
  qi::AnyObject obj = app.session().service("serviceTest");

  obj.connect("testEvent", &eventCb);

  app.run();
}


int main(int argc, char *argv[])
{
  qi::ApplicationSession a(argc, argv);
  // declare the program options
  po::options_description desc("Usage:\n  qi-client --qi-url masterAddress [options]\nOptions");
  desc.add_options()
      ("help", "Print this help.")
      ("event", "listen to event.")
      ("loop", po::value<int>()->default_value(1), "loop count");

  // parse and store
  po::variables_map vm;
  try
  {
    po::store(po::command_line_parser(argc, argv).
              options(desc).run(), vm);
    po::notify(vm);

    if (vm.count("help"))
    {
      std::cout << desc << std::endl;
      return 0;
    }

    qiLogInfo() << "Connecting to:" << a.url().str();
    a.start();
    if (vm.count("event"))
      recEvent(a);
    else
      call(vm["loop"].as<int>(), a.session());
  } catch (const boost::program_options::error&) {
    std::cout << desc << std::endl;
  }

  return 0;
}
