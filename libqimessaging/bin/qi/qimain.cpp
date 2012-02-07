/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010, 2012 Aldebaran Robotics
*/

#include <iostream>
#include <string>
#include <map>
#include <qimessaging/broker.hpp>
#include <qimessaging/client.hpp>
#include <qimessaging/transport.hpp>
#include <boost/program_options.hpp>
#include <qi/os.hpp>

namespace po = boost::program_options;

static int uniqueReqId = 200;

void qi_call(const std::string &addr)
{
  qi::Broker nc;
  qi::NetworkThread nt;

  nc.setThread(&nt);
  qi::os::sleep(1);
  nc.connect(addr);
  nc.waitForConnected();
  nc.setName("moi");

  qi::TransportSocket* ts = nc.service("serviceTest");

  qi::Message msg;
  msg.setId(uniqueReqId++);
  msg.setSource("moi");
  msg.setDestination("qi.serviceTest");

  ts->send(msg);

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
      std::string address = vm["master-address"].as<std::string>();
      qi_call(address);
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
