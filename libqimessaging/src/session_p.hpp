/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#pragma once
#ifndef _SRC_SESSION_P_HPP_
#define _SRC_SESSION_P_HPP_

#include <map>
#include <vector>
#include <boost/thread.hpp>
#include <qimessaging/transport_socket.hpp>
#include <qimessaging/object.hpp>
#include <qimessaging/service_info.hpp>
#include <qimessaging/session.hpp>

namespace qi {

  struct ServiceRequest
  {
    qi::Promise<qi::Object *> promise;
    std::string               name;
    unsigned int              serviceId;
    std::string               protocol;
    bool                      connected; // True if the service server was reached
    unsigned int              attempts; // Number of connection attempts pending.
  };

  class NetworkThread;
  class SessionPrivate : public qi::TransportSocketInterface, public qi::SessionInterface {
  public:
    SessionPrivate(qi::Session *session);
    virtual ~SessionPrivate();

    qi::Future<unsigned int>    registerService(const qi::ServiceInfo &si,
                                                qi::Future<unsigned int> future);
    qi::Future<void>            unregisterService(unsigned int idx);

    virtual void onSocketConnected(TransportSocket *client);
    virtual void onSocketConnectionError(TransportSocket *client);
    virtual void onSocketDisconnected(TransportSocket *client);
    virtual void onSocketReadyRead(TransportSocket *client, int id);
    virtual void onSocketTimeout(TransportSocket *client, int id);

    void serviceEndpointEnd(int id, qi::TransportSocket *client, qi::Message *msg, boost::shared_ptr<ServiceRequest> sr);
    void serviceMetaobjectEnd(int id, qi::TransportSocket *client, qi::Message *msg, boost::shared_ptr<ServiceRequest> sr);

    virtual void onServiceRegistered(Session *QI_UNUSED(session),
                                     const std::string &QI_UNUSED(serviceName));
    virtual void onServiceUnregistered(Session *QI_UNUSED(session),
                                       const std::string &QI_UNUSED(serviceName));

    void servicesEnd(qi::TransportSocket *client, qi::Message *msg,
                     qi::Promise<std::vector<qi::ServiceInfo> > &si);
    void serviceRegisterUnregisterEnd(int id, qi::Message *msg,  qi::FunctorResult promise);
    void serviceReady(unsigned int idx);


  public:
    qi::TransportSocket                *_serviceSocket;
    qi::NetworkThread                  *_networkThread;
    qi::Session                        *_self;
    std::vector<qi::SessionInterface *> _callbacks;
    boost::mutex                        _mutexCallback;

    boost::mutex                                                _mutexFuture;
    // Associate serviceRequest with the message id concerning it currently in
    // transit. It can be sd.service or service.metaobject call
    std::map<int, boost::shared_ptr<ServiceRequest> >           _futureService;
    // Associate serviceRequest with the connection currently connecting.
    std::map<void *, boost::shared_ptr<ServiceRequest> >        _futureConnect;
    std::map<int, qi::Promise<std::vector<qi::ServiceInfo> > >  _futureServices;
    std::map<int, qi::FunctorResult>                            _futureFunctor;
    boost::mutex                                                _mutexServiceReady;
    std::vector<unsigned int>                                   _serviceReady;
    boost::mutex                                                _watchedServicesMutex;
    std::map< std::string, std::pair<int, qi::Promise<void> > > _watchedServices;
  };
}


#endif  // _SRC_SESSION_P_HPP_
