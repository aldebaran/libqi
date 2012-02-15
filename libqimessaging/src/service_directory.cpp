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
#include <qimessaging/service_directory.hpp>
#include <qimessaging/session.hpp>
#include <qimessaging/datastream.hpp>
#include <qi/os.hpp>


namespace qi
{
class ServiceDirectoryPrivate : public TransportServerInterface, public TransportSocketInterface
{
public:
  ServiceDirectoryPrivate()
  {
    nthd = new qi::NetworkThread();
    ts = new qi::TransportServer();
    ts->setDelegate(this);
  }

  ~ServiceDirectoryPrivate()
  {
    delete ts;
    delete nthd;
  }

  virtual void newConnection()
  {
    TransportSocket *socket = ts->nextPendingConnection();
    if (!socket)
      return;
    socket->setDelegate(this);
  }

  virtual void onReadyRead(TransportSocket *socket, const qi::Message &msg)
  {
    qi::Message retval;
    if (msg.path() == "services")
      services(msg, retval);
    else if (msg.path() == "service")
      service(msg, retval);
    else if (msg.path() == "registerEndpoint")
      registerEndpoint(msg, retval);
    else if (msg.path() == "unregisterEndpoint")
      unregisterEndpoint(msg, retval);
    else
      return;
    socket->send(retval);
  }

  virtual void onWriteDone(TransportSocket *client)
  {
  }

  virtual void onConnected(TransportSocket *client)
  {
  }

  virtual void onDisconnected(TransportSocket *client)
  {
  }


  void services(const qi::Message &msg, qi::Message &retval)
  {
    std::vector<std::string> result;

    std::map<std::string, qi::ServiceInfo>::iterator it;
    for (it = connectedServices.begin(); it != connectedServices.end(); ++it)
      result.push_back(it->first);

    qi::DataStream d;
    d << result;

    retval.setType(qi::Message::Answer);
    retval.setId(msg.id());
    retval.setSource(msg.destination());
    retval.setDestination(msg.source());
    retval.setPath(msg.path());
    retval.setData(d.str());
  }


  void service(const qi::Message &msg, qi::Message &retval)
  {
    qi::DataStream d;
    std::map<std::string, qi::ServiceInfo>::iterator servicesIt;
    servicesIt = connectedServices.find(msg.data());
    if (servicesIt != connectedServices.end())
    {
      qi::ServiceInfo si = servicesIt->second;
      d << si.endpoint;
    }

    retval.setType(qi::Message::Answer);
    retval.setId(msg.id());
    retval.setSource(msg.destination());
    retval.setDestination(msg.source());
    retval.setPath(msg.path());
    retval.setData(d.str());
  }

  void registerEndpoint(const qi::Message &msg, qi::Message &retval)
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

    retval.setType(qi::Message::Answer);
    retval.setId(msg.id());
    retval.setSource(msg.destination());
    retval.setDestination(msg.source());
    retval.setPath(msg.path());
    retval.setData(msg.source() + " register.");
  }

  void unregisterEndpoint(const qi::Message &msg, qi::Message &retval)
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

    retval.setType(qi::Message::Answer);
    retval.setId(msg.id());
    retval.setSource(msg.destination());
    retval.setDestination(msg.source());
    retval.setPath(msg.path());
    retval.setData(msg.id() + " unregister.");
  }

public:
  qi::NetworkThread                     *nthd;
  qi::TransportServer                   *ts;
  std::map<std::string, qi::ServiceInfo> connectedServices;

}; // !ServiceDirectoryPrivate


ServiceDirectory::ServiceDirectory()
  : _p(new ServiceDirectoryPrivate())
{
}

ServiceDirectory::~ServiceDirectory()
{
  delete _p;
}

void ServiceDirectory::start(const std::string &address)
{
  size_t begin = 0;
  size_t end = 0;
  end = address.find(":");

  std::string ip = address.substr(begin, end);
  begin = end + 1;

  unsigned int port;
  std::stringstream ss(address.substr(begin));
  ss >> port;

  _p->ts->start(ip, port, _p->nthd->getEventBase());
}

}; // !qi
