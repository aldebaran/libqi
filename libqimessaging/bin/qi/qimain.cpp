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
#include <qimessaging/session.hpp>
#include <qimessaging/transport.hpp>

#include <boost/program_options.hpp>

namespace po = boost::program_options;

static int uniqueReqId = 200;

void call(const std::string &addr)
{
  qi::Session session;
  session.setName("client.session");
  session.setDestination("qi.master");
  session.connect(addr);
  session.waitForConnected();

  std::vector<std::string> servs = session.services();
  for (int i = 0; i < servs.size(); ++i)
    std::cout << "service named " << servs[i] << std::endl;


  qi::TransportSocket* servConnection = session.service("serviceTest");

  qi::Message msg;
  msg.setId(uniqueReqId++);
  msg.setSource("client");
  msg.setDestination("serviceTest");
  msg.setPath("reply");

  servConnection->send(msg);
  servConnection->waitForId(msg.id());
  qi::Message ans;
  servConnection->read(msg.id(), &ans);

  std::cout << ans << std::endl;

  qi::os::sleep(2);
}


int main(int argc, char *argv[])
{
  // declare the program options
  po::options_description desc("Usage:\n  qi masterAddress [options]\nOptions");
  desc.add_options()
      ("help", "Print this help.")
      ("master-address",
       po::value<std::string>()->default_value(std::string("127.0.0.1:5555")),
       "The master address")
      ("gateway-address",
       po::value<std::string>()->default_value(std::string("127.0.0.1:12345")),
       "The gateway address");

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

    if (vm.count("master-address") == 1 &&
        vm.count("gateway-address") == 1)
    {
      std::string masteraddr = vm["master-address"].as<std::string>();
      call(masteraddr);

      std::string gatewayaddr = vm["gateway-address"].as<std::string>();
      call(gatewayaddr);
    }
    else
    {
      std::cout << desc << "\n";
    }
  } catch (const boost::program_options::error&) {
    std::cout << desc << "\n";
  }

  return 0;
}
