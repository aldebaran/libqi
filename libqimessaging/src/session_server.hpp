/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/


#pragma once
#ifndef _QIMESSAGING_SERVER_HPP_
#define _QIMESSAGING_SERVER_HPP_

#include <string>
#include <set>
#include <boost/thread/recursive_mutex.hpp>
#include <qimessaging/api.hpp>
#include <qimessaging/session.hpp>
#include <qimessaging/transport_server.hpp>

namespace qi {

  class Object;
  class ServiceDirectoryClient;

  // (service, linkId)
  struct RemoteLink
  {
    RemoteLink()
      : localLinkId(0)
      , event(0)
    {}
    RemoteLink(unsigned int localLinkId, unsigned int event)
    : localLinkId(localLinkId)
    , event(event) {}
    unsigned int localLinkId;
    unsigned int event;
  };

  // remote link id -> local link id
  typedef std::map<unsigned int, RemoteLink> ServiceLinks;
  // service id -> links
  typedef std::map<unsigned int, ServiceLinks> PerServiceLinks;
  // socket -> all links
  // Keep track of links setup by clients to disconnect when the client exits.
  typedef std::map<TransportSocket*, PerServiceLinks > Links;


  class QIMESSAGING_API Session_Server : public TransportServerInterface,
                                         public TransportSocketInterface,
                                         public FutureInterface<unsigned int> {
  public:
    Session_Server(ServiceDirectoryClient *_sdClient);
    virtual ~Session_Server();

    bool listen(const std::string &address);
    void close();

    qi::Future<unsigned int> registerService(const std::string &name, qi::Object *obj);
    qi::Future<void>         unregisterService(unsigned int idx);

    std::vector<qi::ServiceInfo>  registeredServices();
    qi::ServiceInfo               registeredService(const std::string &service);
    qi::Object                   *registeredServiceObject(const std::string &service);

    qi::Url                       listenUrl() const;

  private:
    virtual void onTransportServerNewConnection(TransportServer* server, TransportSocket *client, void *data);
    virtual void onSocketReadyRead(TransportSocket *client, int id, void *data);
    virtual void onSocketDisconnected(TransportSocket *client, void *data);
    virtual void onFutureFinished(const unsigned int &future, void *data);
    virtual void onFutureFailed(const std::string &error, void *data);

  private:
    Links                                   _links;

    std::set<TransportSocket*>              _clients;
    std::map<unsigned int, qi::Object*>     _services;
    std::map<std::string, qi::Object*>      _servicesByName;
    std::map<std::string, qi::ServiceInfo>  _servicesInfo;
    std::map<qi::Object*, qi::ServiceInfo>  _servicesObject;
    std::map<unsigned int, std::string>     _servicesIndex;
    TransportServer                         _server;
    boost::mutex                            _mutexServices;
    boost::recursive_mutex                  _mutexOthers;
    bool                                    _dying;
    ServiceDirectoryClient                 *_sdClient;
  };
}

#endif  // _QIMESSAGING_SERVER_HPP_
