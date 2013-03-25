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
#include <qi/application.hpp>
#include <qimessaging/session.hpp>

#include <boost/program_options.hpp>

namespace po = boost::program_options;

qiLogCategory("qiclient");

qi::ObjectPtr getObj(qi::Session *ses) {
  qi::Session &session = *ses;

  qi::Future<qi::ObjectPtr> fut = session.service("serviceTest");
  fut.wait();
  if (fut.hasError())
  {
    std::cerr << "Error returned:" << fut.error() << std::endl;
    return qi::ObjectPtr();
  }
  qi::ObjectPtr obj = fut.value();
  return obj;
}

void call(int count, const std::string &addr)
{
  qi::Session session;
  session.connect(addr);

  qi::ObjectPtr obj = getObj(&session);

  for (int i = 0; i < count; ++i) {
    std::string result = obj->call<std::string>("reply", "plaf");
    std::cout << "result" << i << ":" << result << std::endl;
  }

  session.close();
}

void eventCb(const std::string &event) {
  qiLogInfo() << "Received event data: " << event;
}

void recEvent(qi::Application *app, const std::string &addr) {
  qi::Session session;
  session.connect(addr);

  qi::ObjectPtr obj = getObj(&session);

  obj->connect("testEvent", &eventCb);

  app->run();
  session.close();

}


int main(int argc, char *argv[])
{
  qi::Application a(argc, argv);

  // declare the program options
  po::options_description desc("Usage:\n  qi-client masterAddress [options]\nOptions");
  desc.add_options()
      ("help", "Print this help.")
      ("master-address",
       po::value<std::string>()->default_value(std::string("tcp://127.0.0.1:5555")),
       "The master address")
      ("event", "listen to event.")
      ("loop", po::value<int>()->default_value(1), "loop count");

  // allow master address to be specified as the first arg
  po::positional_options_description pos;
  pos.add("master-address", 1);

  // parse and store
  po::variables_map vm;
  try
  {
    po::store(po::command_line_parser(argc, argv).
              options(desc).positional(pos).run(), vm);
    po::notify(vm);

    if (vm.count("help"))
    {
      std::cout << desc << std::endl;
      return 0;
    }


    if (vm.count("master-address") == 1)
    {
      std::string masteraddr = vm["master-address"].as<std::string>();
      if (vm.count("event"))
        recEvent(&a, masteraddr);
      else
        call(vm["loop"].as<int>(), masteraddr);
    }
    else
    {
      std::cout << desc << std::endl;
    }
  } catch (const boost::program_options::error&) {
    std::cout << desc << std::endl;
  }

  return 0;
}
