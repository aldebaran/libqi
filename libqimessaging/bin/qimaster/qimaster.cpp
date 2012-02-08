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
#include <qimessaging/broker.hpp>
#include <qimessaging/datastream.hpp>
#include <qi/os.hpp>
#include <boost/program_options.hpp>

namespace po = boost::program_options;

class ServiceDirectoryServer : public qi::TransportServerDelegate
{
public:
  ServiceDirectoryServer()
  {
    nthd = new qi::NetworkThread();

    ts = new qi::TransportServer();
    ts->setDelegate(this);
  }

  ~ServiceDirectoryServer()
  {
    delete ts;
  }

  void setThread(qi::NetworkThread *n)
  {
    nthd = n;
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
  }

  virtual void onWrite(const qi::Message &msg)
  {
  }

  virtual void onRead(const qi::Message &msg)
  {
    std::cout << "read qimaster: " << msg << std::endl;

    if (msg.path() == "services")
      services(msg);

    if (msg.path() == "service")
      service(msg);

    if (msg.path() == "registerEndpoint")
      registerEndpoint(msg);

    if (msg.path() == "unregisterEndpoint")
      unregisterEndpoint(msg);

  }

  void services(const qi::Message &msg)
  {
    std::vector<std::string> servs;

    std::map<std::string, qi::ServiceInfo>::iterator it;
    for (it = connectedServices.begin(); it != connectedServices.end(); ++it)
      servs.push_back(it->first);

    qi::DataStream d;
    d << servs;

    qi::Message retval;
    retval.setType(qi::Message::Answer);
    retval.setId(msg.id());
    retval.setSource(msg.destination());
    retval.setDestination(msg.source());
    retval.setPath(msg.path());
    retval.setData(d.str());

    ts->send(retval);
  }


  void service(const qi::Message &msg)
  {
    qi::DataStream d;
    std::map<std::string, qi::ServiceInfo>::iterator servicesIt;
    servicesIt = connectedServices.find(msg.data());
    if (servicesIt != connectedServices.end())
    {
      qi::ServiceInfo si = servicesIt->second;
      d << si.endpoint;
    }

    qi::Message retval;
    retval.setType(qi::Message::Answer);
    retval.setId(msg.id());
    retval.setSource(msg.destination());
    retval.setDestination(msg.source());
    retval.setPath(msg.path());
    retval.setData(d.str());

    ts->send(retval);
  }


  void registerEndpoint(const qi::Message &msg)
  {
    qi::EndpointInfo e;
    qi::DataStream d(msg.data());
    d >> e;

    std::map<std::string, qi::ServiceInfo>::iterator servicesIt;
    servicesIt = connectedServices.find(msg.source());
    if (servicesIt != connectedServices.end())
    {
      qi::ServiceInfo *si = &servicesIt->second;
      std::vector<qi::EndpointInfo> *ei = &si->endpoint;
      ei->push_back(e);
    }
    else
    {
      qi::ServiceInfo si;
      si.name = msg.source();
      si.endpoint.push_back(e);
      connectedServices[msg.source()] = si;
    }

    qi::Message retval;
    retval.setType(qi::Message::Answer);
    retval.setId(msg.id());
    retval.setSource(msg.destination());
    retval.setDestination(msg.source());
    retval.setPath(msg.path());
    retval.setData(msg.source() + " register.");
    ts->send(retval);
  }

  void unregisterEndpoint(const qi::Message &msg)
  {
    qi::EndpointInfo e;
    qi::DataStream d(msg.data());
    d >> e;

    std::map<std::string, qi::ServiceInfo>::iterator servicesIt;
    servicesIt = connectedServices.find(msg.source());
    if (servicesIt != connectedServices.end())
    {
      qi::ServiceInfo *si = &servicesIt->second;
      std::vector<qi::EndpointInfo> *ei = &si->endpoint;
      std::vector<qi::EndpointInfo>::iterator endpointIt = ei->begin();

      // fixme
      for (int i = 0; i < ei->size(); ++i, ++endpointIt)
      {
        if (*endpointIt == e)
        {
          ei->erase(endpointIt);
          break;
        }
      }

      if (ei->empty())
        connectedServices.erase(servicesIt);
    }

    qi::Message retval;
    retval.setType(qi::Message::Answer);
    retval.setId(msg.id());
    retval.setSource(msg.destination());
    retval.setDestination(msg.source());
    retval.setPath(msg.path());
    retval.setData(msg.id() + " unregister.");
    ts->send(retval);
  }


private:
  qi::NetworkThread   *nthd;
  qi::TransportServer *ts;

  std::map<std::string, qi::ServiceInfo> connectedServices;
};


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

      ServiceDirectoryServer sds;
      sds.start(masterAddress);

      std::cout << "ready." << std::endl;

      while (1)
        qi::os::sleep(1);
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
