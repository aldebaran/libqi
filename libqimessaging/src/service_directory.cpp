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
#include "src/transport/url.hpp"
#include "src/transport/network_thread.hpp"
#include <qi/os.hpp>


namespace qi
{
class ServiceDirectoryPrivate : public TransportServerInterface,
                                public TransportSocketInterface,
                                public Object
{
public:
  ServiceDirectoryPrivate()
  {
    nthd = new qi::NetworkThread();
    ts = new qi::TransportServer();
    ts->setDelegate(this);
    advertiseMethod("service", this, &ServiceDirectoryPrivate::service);
    advertiseMethod("services", this, &ServiceDirectoryPrivate::services);
    advertiseMethod("registerService", this, &ServiceDirectoryPrivate::registerService);

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

  virtual void onReadyRead(TransportSocket *socket, qi::Message &msg)
  {
    DataStream din(msg.buffer());

    qi::Message out;
    out.buildReplyFrom(msg);
    DataStream dout(out.buffer());

    metaCall(msg.function(), "sig", din, dout);
    socket->send(out);
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

  std::vector<std::string> services()
  {
    std::vector<std::string> result;
    std::map<std::string, std::vector<std::string> >::iterator it;

    for (it = connectedServices.begin(); it != connectedServices.end(); ++it)
      result.push_back(it->first);

    return result;
  }

  std::vector<std::string> service(const std::string &name)
  {
    std::vector<std::string> result;
    std::map<std::string, std::vector<std::string> >::const_iterator servicesIt;

    result.push_back(name);

    servicesIt = connectedServices.find(name);
    if (servicesIt == connectedServices.end())
    {
      return result;
    }

    for (std::vector<std::string>::const_iterator endpointsIt = servicesIt->second.begin();
         endpointsIt != servicesIt->second.end();
         endpointsIt++)
    {
      result.push_back(*endpointsIt);
    }

    return result;
  }

  void registerService(const std::string &name, const std::vector<std::string> &endpoints)
  {
    for (std::vector<std::string>::const_iterator endpointsIt = endpoints.begin();
         endpointsIt != endpoints.end();
         endpointsIt++)
    {
      connectedServices[name].push_back(*endpointsIt);
    }
  }

public:
  qi::NetworkThread                                 *nthd;
  qi::TransportServer                               *ts;
  std::map<std::string, std::vector<std::string> >   connectedServices;

}; // !ServiceDirectoryPrivate


ServiceDirectory::ServiceDirectory()
  : _p(new ServiceDirectoryPrivate())
{
}

ServiceDirectory::~ServiceDirectory()
{
  delete _p;
}

void ServiceDirectory::listen(const std::string &address)
{
  qi::Url url(address);

  _p->ts->start(url.host(), url.port(), _p->nthd->getEventBase());
}

}; // !qi
