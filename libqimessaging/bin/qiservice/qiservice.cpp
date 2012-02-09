/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010, 2012 Aldebaran Robotics
*/

#include <iostream>
#include <vector>
#include <map>
#include <qimessaging/transport.hpp>
#include <qimessaging/datastream.hpp>
#include <qimessaging/broker.hpp>
#include <qi/os.hpp>
#include <boost/program_options.hpp>

namespace po = boost::program_options;

class ServiceTest : public qi::TransportServerDelegate
{
public:
  ServiceTest()
  {
    nthd = new qi::NetworkThread();

    ts = new qi::TransportServer();
    ts->setDelegate(this);
  }

  ~ServiceTest()
  {
    delete ts;
  }

  void start(const std::string &address)
  {
    size_t begin = 0;
    size_t end = 0;
    end = address.find(":");

    std::string ip = address.substr(begin, end);
    begin = end + 1;

    unsigned int port;
    std::stringstream ss(address.substr(begin));
    ss >> port;

    ts->start(ip, port, nthd->getEventBase());
  }

  virtual void onConnected(const qi::Message &msg)
  {
//    std::cout << "connected qiservice: " << msg << std::endl;
  }

  virtual void onWrite(const qi::Message &msg)
  {
//    std::cout << "written qiservice: " << msg << std::endl;
  }

  virtual void onRead(const qi::Message &msg)
  {
    std::cout << "read  qiservice: " << msg << std::endl;
  }

private:
  qi::NetworkThread   *nthd;
  qi::TransportServer *ts;
};


int main(int argc, char *argv[])
{
  // declare the program options
  po::options_description desc("Usage:\n  qi-service masterAddress [options]\nOptions");
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
      qi::Session        broker;
      ServiceTest       st;

      st.start("127.0.0.1:9571");
      std::cout << "ready." << std::endl;

      qi::EndpointInfo e;
      e.ip = "127.0.0.1";
      e.port = 9571;
      e.type = "tcp";

      std::string masterAddress = vm["master-address"].as<std::string>();

      broker.connect(masterAddress);
      broker.waitForConnected();
      broker.setName("serviceTest");
      broker.setDestination("qi.master");
      broker.registerEndpoint(e);

      while (1)
        qi::os::sleep(1);

      broker.unregisterEndpoint(e);
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
