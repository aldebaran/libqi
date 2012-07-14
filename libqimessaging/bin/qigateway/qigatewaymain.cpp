/*
** Author(s):
**  - Herve Cuche <hcuche@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <iostream>
#include <string>

#include <boost/program_options.hpp>
#include <qi/log.hpp>
#include <qimessaging/gateway.hpp>

namespace po = boost::program_options;

int main(int argc, char *argv[])
{
  // declare the program options
  po::options_description desc("Usage:\n  qi-service masterAddress [options]\nOptions");
  desc.add_options()
      ("help", "Print this help.")
      ("master-address",
       po::value<std::string>()->default_value(std::string("tcp://127.0.0.1:5555")),
       "The master address")
      ("gateway-address",
       po::value<std::string>()->default_value(std::string("tcp://127.0.0.1:12345")),
       "The gateway address")
      ("gateway-type",
       po::value<std::string>()->default_value(std::string("local")),
       "The gateway type");

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

    std::string gatewayAddress = vm["gateway-address"].as<std::string>();
    std::string masterAddress = vm["master-address"].as<std::string>();
    std::string gatewayType = vm["gateway-type"].as<std::string>();

    if (gatewayType == "local")
    {
      qi::Gateway gateway;
      if (!gateway.attachToServiceDirectory(masterAddress) ||
          !gateway.listen(gatewayAddress))
      {
        return 1;
      }
      std::cout << "Local gateway ready: " << gatewayAddress << std::endl;
      gateway.join();
    }
    else if (gatewayType == "remote")
    {
      qi::RemoteGateway gateway;
      if (!gateway.listen(gatewayAddress))
      {
        return 1;
      }
      std::cout << "Remote gateway ready: " << gatewayAddress << std::endl;
      gateway.join();
    }
    else if (gatewayType == "reverse")
    {
      qi::ReverseGateway gateway;
      if (!gateway.attachToServiceDirectory(masterAddress))
      {
        return 1;
      }

      std::cout << "Reverse gateway ready" << std::endl;
      std::string line;
      while (std::getline(std::cin, line))
      {
        qi::Url url(line);
        gateway.connect(url);
      }
      gateway.join();
    }
    else
    {
      std::cerr << "unknown type: " << gatewayType << std::endl;
      return 1;
    }
  }
  catch (const boost::program_options::error&)
  {
    std::cout << desc << "\n";
  }

  return 0;
}
