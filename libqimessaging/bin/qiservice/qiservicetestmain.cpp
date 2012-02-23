/*
** Author(s):
**  - Herve Cuche <hcuche@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <iostream>
#include <string>

#include <boost/program_options.hpp>

#include <qi/os.hpp>
#include <qimessaging/session.hpp>
#include <qimessaging/server.hpp>
#include <qimessaging/object.hpp>

//#include "qiservicetest.hpp"

std::string reply(const std::string &msg) {
  std::cout << "Message recv:" << msg << std::endl;
  return msg + "bim";
}

namespace po = boost::program_options;

int main(int argc, char *argv[])
{
  // declare the program options
  po::options_description desc("Usage:\n  qi-service masterAddress [options]\nOptions");
  desc.add_options()
      ("help", "Print this help.")
      ("master-address",
       po::value<std::string>()->default_value(std::string("tcp://0.0.0.0:5555")),
       "The master address");

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
      std::cout << desc << "\n";
      return 0;
    }

    if (vm.count("master-address") == 1)
    {
      std::string masterAddress = vm["master-address"].as<std::string>();
      qi::Session       session;
      qi::Object        obj;
      qi::Server        srv;
      obj.advertiseMethod("reply", &reply);

      session.connect(masterAddress);
      session.waitForConnected();

      std::vector<std::string> endpoints;
      endpoints.push_back("tcp://127.0.0.1:9571");
      srv.listen(&session, endpoints);
      srv.registerService("serviceTest", &obj);

      std::cout << "ready." << std::endl;

      session.join();

      srv.stop();
      session.disconnect();
    }
    else
    {
      std::cout << desc << "\n";
    }
  }
  catch (const boost::program_options::error&)
  {
    std::cout << desc << "\n";
  }

  return 0;
}
