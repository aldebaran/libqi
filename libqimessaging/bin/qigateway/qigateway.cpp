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
#include <qimessaging/session.hpp>
#include <qi/os.hpp>
#include <boost/program_options.hpp>

namespace po = boost::program_options;

static int id = 300;


class Gateway : public qi::TransportServerDelegate
{
public:
  Gateway()
  {
    _nthd = new qi::NetworkThread();

    _ts = new qi::TransportServer();
    _ts->setDelegate(this);
  }

  ~Gateway()
  {
    delete _ts;
  }

  void start(const std::string &address)
  {
    qi::EndpointInfo e;
    size_t begin = 0;
    size_t end = 0;
    end = address.find(":");

    e.type = "tcp";
    e.ip = address.substr(begin, end);
    begin = end + 1;

    std::stringstream ss(address.substr(begin));
    ss >> e.port;

    _endpoints.push_back(e);
    _ts->start(e.ip, e.port, _nthd->getEventBase());
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
    std::cout << "read qigateway: " << msg << std::endl;

    if (msg.path() == "services")
    {
      services(msg);
      return;
    }

    if (msg.path() == "service")
    {
      service(msg);
      return;
    }

    qi::TransportSocket* ts;
    std::map<std::string, qi::TransportSocket*>::iterator it = _serviceConnection.find(msg.destination());
    // no connection to service
    if (it == _serviceConnection.end())
    {
      ts = _session.service(msg.destination());
      _serviceConnection[msg.destination()] = ts;
    }
    else
    {
      ts = it->second;
    }

    qi::Message fwd(msg);
    fwd.setId(id++);
    fwd.setSource("gateway");
    ts->send(fwd);
    ts->waitForId(fwd.id());
    qi::Message ans;
    ts->read(fwd.id(), &ans);
    ans.setId(msg.id());
    ans.setDestination(msg.source());

    _ts->send(ans);
  }

  void services(const qi::Message &msg)
  {
    std::vector<std::string> result = _session.services();

    qi::DataStream d;
    d << result;

    qi::Message retval;
    retval.setType(qi::Message::Answer);
    retval.setId(msg.id());
    retval.setSource(msg.destination());
    retval.setDestination(msg.source());
    retval.setPath(msg.path());
    retval.setData(d.str());

    _ts->send(retval);
  }


  void service(const qi::Message &msg)
  {
    qi::DataStream d;
    d << _endpoints;

    qi::Message retval;
    retval.setType(qi::Message::Answer);
    retval.setId(msg.id());
    retval.setSource(msg.destination());
    retval.setDestination(msg.source());
    retval.setPath(msg.path());
    retval.setData(d.str());

    _ts->send(retval);
  }

  void registerGateway(const std::string &masterAddress,
                       const std::string &gatewayAddress)
  {
    qi::EndpointInfo e;

    size_t begin = 0;
    size_t end = 0;
    end = gatewayAddress.find(":");

    e.ip = gatewayAddress.substr(begin, end);
    begin = end + 1;

    std::stringstream ss(gatewayAddress.substr(begin));
    ss >> e.port;
    e.type = "tcp";

    _session.connect(masterAddress);
    _session.waitForConnected();
    _session.setName("gateway");
    _session.setDestination("qi.master");
    _session.registerEndpoint(e);
  }

  void unregisterGateway(const std::string &gatewayAddress)
  {
    qi::EndpointInfo e;

    size_t begin = 0;
    size_t end = 0;
    end = gatewayAddress.find(":");

    e.ip = gatewayAddress.substr(begin, end);
    begin = end + 1;

    std::stringstream ss(gatewayAddress.substr(begin));
    ss >> e.port;
    e.type = "tcp";

    _session.unregisterEndpoint(e);
  }

private:
  qi::Session          _session;
  qi::NetworkThread   *_nthd;
  qi::TransportServer *_ts;
  std::vector<qi::EndpointInfo>               _endpoints;
  std::map<std::string, qi::TransportSocket*> _serviceConnection;
};


int main(int argc, char *argv[])
{
  // declare the program options
  po::options_description desc("Usage:\n  qi-service masterAddress [options]\nOptions");
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
        vm.count("gateway-address") == 1 )
    {
      std::string gatewayAddress = vm["gateway-address"].as<std::string>();
      std::string masterAddress = vm["master-address"].as<std::string>();

      Gateway           gate;
      gate.start(gatewayAddress);
      std::cout << "ready." << std::endl;

      gate.registerGateway(masterAddress, gatewayAddress);

//      qi::Message msg;
//      msg.setId(1000);
//      msg.setSource("gateway");
//      msg.setDestination("qi.master");
//      msg.setPath("services");
//      msg.setType(qi::Message::Call);
//      gate.services(msg);

      while (1)
        qi::os::sleep(1);

      gate.unregisterGateway(gatewayAddress);
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
