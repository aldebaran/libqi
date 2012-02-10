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
#include <qimessaging/session.hpp>
#include <qimessaging/datastream.hpp>
#include <qi/os.hpp>

#include "qimaster.hpp"

namespace qi
{

  ServiceDirectoryServer::ServiceDirectoryServer()
  {
    nthd = new qi::NetworkThread();
    ts = new qi::TransportServer();
    ts->setDelegate(this);
  }

  ServiceDirectoryServer::~ServiceDirectoryServer()
  {
    delete ts;
    delete nthd;
  }

  void ServiceDirectoryServer::start(const std::string &address)
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

  void ServiceDirectoryServer::onConnected(const qi::Message &msg)
  {
  }

  void ServiceDirectoryServer::onWrite(const qi::Message &msg)
  {
  }

  void ServiceDirectoryServer::onRead(const qi::Message &msg)
  {
    std::cout << "qimaster read" << std::endl;

    if (msg.path() == "services")
      services(msg);

    if (msg.path() == "service")
      service(msg);

    if (msg.path() == "registerEndpoint")
      registerEndpoint(msg);

    if (msg.path() == "unregisterEndpoint")
      unregisterEndpoint(msg);
  }

  void ServiceDirectoryServer::services(const qi::Message &msg)
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


  void ServiceDirectoryServer::service(const qi::Message &msg)
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


  void ServiceDirectoryServer::registerEndpoint(const qi::Message &msg)
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

  void ServiceDirectoryServer::unregisterEndpoint(const qi::Message &msg)
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


}; // !qi
