/*
** Author(s):
**  - Herve Cuche <hcuche@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <iostream>
#include <vector>
#include <map>

#include <qimessaging/transport.hpp>
#include <qimessaging/transport/qigateway.hpp>
#include <qimessaging/datastream.hpp>
#include <qimessaging/session.hpp>
#include <qi/os.hpp>

static int id = 300;

namespace qi
{
  Gateway::Gateway()
  {
    _nthd = new qi::NetworkThread();
    _ts = new qi::TransportServer();
    _ts->setDelegate(this);
  }

  Gateway::~Gateway()
  {
    delete _ts;
    delete _nthd;
  }

  void Gateway::start(const std::string &address)
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

  void Gateway::onConnected(const qi::Message &msg)
  {
  }

  void Gateway::onWrite(const qi::Message &msg)
  {
  }

  void Gateway::onRead(const qi::Message &msg)
  {
    if (msg.path() == "services")
    {
      services(msg);
    }
    else if (msg.path() == "service")
    {
      service(msg);
    }
    else
    {
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
  }

  void Gateway::services(const qi::Message &msg)
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


  void Gateway::service(const qi::Message &msg)
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

  void Gateway::registerGateway(const std::string &masterAddress,
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

  void Gateway::unregisterGateway(const std::string &gatewayAddress)
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

} // !qi
