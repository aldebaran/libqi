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
#include <qimessaging/service_info.hpp>
#include "src/network_thread.hpp"
#include "src/transport_server_p.hpp"
#include "src/server_functor_result_future_p.hpp"
#include <qi/os.hpp>
#include <qi/log.hpp>


namespace qi
{

  class ServiceDirectoryPrivate : public TransportServerInterface,
      public TransportSocketInterface,
      public Object
  {
  public:
    ServiceDirectoryPrivate();
    ~ServiceDirectoryPrivate();

    virtual void newConnection();
    virtual void onSocketReadyRead(TransportSocket *socket, int id);
    virtual void onSocketWriteDone(TransportSocket *client);
    virtual void onSocketConnected(TransportSocket *client);
    virtual void onSocketDisconnected(TransportSocket *client);

    std::vector<ServiceInfo> services();
    ServiceInfo              service(const std::string &name);
    unsigned int             registerService(const ServiceInfo &svcinfo);

  public:
    qi::NetworkThread                                 *nthd;
    qi::TransportServer                               *ts;
    std::map<unsigned int, ServiceInfo>                connectedServices;
    std::map<std::string, unsigned int>                nameToIdx;
    unsigned int                                       servicesCount;
  }; // !ServiceDirectoryPrivate




  ServiceDirectoryPrivate::ServiceDirectoryPrivate()
    : nthd(new qi::NetworkThread())
    , ts(new qi::TransportServer())
    , servicesCount(0)
  {
    ts->setCallbacks(this);

    ServiceInfo si;
    si.setName("serviceDirectory");
    si.setServiceId(1);
    unsigned int regid = registerService(si);
    //serviceDirectory must have id '1'
    assert(regid == 1);

    /*
     * Order is important. See qi::Message::ServiceDirectoryFunctions.
     */
    advertiseMethod("service", this, &ServiceDirectoryPrivate::service);
    advertiseMethod("services", this, &ServiceDirectoryPrivate::services);
    advertiseMethod("registerService", this, &ServiceDirectoryPrivate::registerService);
  }

  ServiceDirectoryPrivate::~ServiceDirectoryPrivate()
  {
    delete ts;
    delete nthd;
  }

  void ServiceDirectoryPrivate::newConnection()
  {
    TransportSocket *socket = ts->nextPendingConnection();
    if (!socket)
      return;
    socket->setCallbacks(this);
  }

  void ServiceDirectoryPrivate::onSocketReadyRead(TransportSocket *socket, int id)
  {
    qi::Message msg;
    socket->read(id, &msg);
    FunctorParameters din(msg.buffer());

    ServerFunctorResultPromise *sfrp = new ServerFunctorResultPromise(socket, msg);
    metaCall(msg.function(), "sig", din, sfrp);
  }

  void ServiceDirectoryPrivate::onSocketWriteDone(TransportSocket *client)
  {
  }

  void ServiceDirectoryPrivate::onSocketConnected(TransportSocket *client)
  {
  }

  void ServiceDirectoryPrivate::onSocketDisconnected(TransportSocket *client)
  {
  }

  std::vector<ServiceInfo> ServiceDirectoryPrivate::services()
  {
    std::vector<ServiceInfo> result;
    std::map<unsigned int, ServiceInfo>::const_iterator it;

    for (it = connectedServices.begin(); it != connectedServices.end(); ++it)
      result.push_back(it->second);

    return result;
  }

  ServiceInfo ServiceDirectoryPrivate::service(const std::string &name)
  {
    std::map<unsigned int, ServiceInfo>::const_iterator servicesIt;
    std::map<std::string, unsigned int>::const_iterator it;

    it = nameToIdx.find(name);
    if (it == nameToIdx.end())
      return ServiceInfo();

    unsigned int idx = it->second;

    servicesIt = connectedServices.find(idx);
    if (servicesIt == connectedServices.end())
      return ServiceInfo();

    return servicesIt->second;
  }

  unsigned int ServiceDirectoryPrivate::registerService(const ServiceInfo &svcinfo)
  {
    unsigned int idx = ++servicesCount;
    nameToIdx[svcinfo.name()] = idx;
    connectedServices[idx] = svcinfo;
    connectedServices[idx].setServiceId(idx);
    qiLogInfo("qimessaging.ServiceDirectory")  << "service " << svcinfo.name() << " registered #" << idx << std::endl;
    return idx;
  }


ServiceDirectory::ServiceDirectory()
  : _p(new ServiceDirectoryPrivate())
{
}

ServiceDirectory::~ServiceDirectory()
{
  delete _p;
}

bool ServiceDirectory::listen(const std::string &address)
{
  qi::Url                  url(address);
  std::vector<std::string> eps;
  ServiceInfo             &si = _p->connectedServices[1];

  eps.push_back(address);
  si.setEndpoints(eps);

  return _p->ts->_p->start(_p->nthd->getEventBase(), url);
}

void ServiceDirectory::join()
{
  _p->nthd->join();
}

}; // !qi
