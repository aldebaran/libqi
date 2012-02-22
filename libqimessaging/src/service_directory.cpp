/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010, 2012 Aldebaran Robotics
*/

#include <iostream>
#include <vector>
#include <map>

#include <qimessaging/object.hpp>
#include <qimessaging/transport_server.hpp>
#include <qimessaging/transport_socket.hpp>
#include <qimessaging/service_directory.hpp>
#include <qimessaging/session.hpp>
#include <qimessaging/datastream.hpp>
#include "src/url.hpp"
#include "src/network_thread.hpp"
#include <qi/os.hpp>
#include <qi/log.hpp>


namespace qi
{
class ServiceDirectoryPrivate : public TransportServerInterface,
                                public TransportSocketInterface,
                                public Object
{
public:
  ServiceDirectoryPrivate()
    : nthd(new qi::NetworkThread())
    , ts(new qi::TransportServer())
    , servicesCount(0)
  {
    ts->setDelegate(this);

    /*
     * Order is important. See qi::Message::ServiceDirectoryFunctions.
     */
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
    std::map<std::string, unsigned int>::const_iterator it;

    for (it = nameToIdx.begin(); it != nameToIdx.end(); ++it)
      result.push_back(it->first);

    return result;
  }

  std::vector<std::string> service(const std::string &name)
  {
    std::vector<std::string> result;
    std::map<unsigned int, std::vector<std::string> >::const_iterator servicesIt;
    std::stringstream ss;

    std::map<std::string, unsigned int>::const_iterator it;
    it = nameToIdx.find(name);
    if (it == nameToIdx.end())
    {
      return result;
    }

    unsigned int idx = it->second;

    // store id to string in the first slot
    ss << idx;
    result.push_back(ss.str());

    servicesIt = connectedServices.find(idx);
    for (std::vector<std::string>::const_iterator endpointsIt = servicesIt->second.begin();
         endpointsIt != servicesIt->second.end();
         endpointsIt++)
    {
      result.push_back(*endpointsIt);
    }

    return result;
  }

  unsigned int registerService(const std::string &name, const std::vector<std::string> &endpoints)
  {
    unsigned int idx = ++servicesCount;
    nameToIdx[name] = idx;

    for (std::vector<std::string>::const_iterator endpointsIt = endpoints.begin();
         endpointsIt != endpoints.end();
         endpointsIt++)
    {
      connectedServices[idx].push_back(*endpointsIt);
    }

    qiLogInfo("qimessaging.ServiceDirectory")  << "service " << name << " registered #" << servicesCount << std::endl;

    return idx;
  }

public:
  qi::NetworkThread                                 *nthd;
  qi::TransportServer                               *ts;
  std::map<unsigned int, std::vector<std::string> >  connectedServices;
  std::map<std::string, unsigned int>                nameToIdx;
  unsigned int                                       servicesCount;

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

void ServiceDirectory::join()
{
  _p->nthd->join();
}

}; // !qi
