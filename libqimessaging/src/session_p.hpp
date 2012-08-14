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
#include <qimessaging/transport_server.hpp>
#include <qimessaging/object.hpp>
#include <qimessaging/service_info.hpp>
#include <qimessaging/session.hpp>

namespace qi {

  class NetworkThread;
  class Object;
  class ServerPrivate;

  /// <summary> Used to advertise named services. Advertised Services are
  /// registered with the service directory so that clients can find them. The exact
  /// signature of your method is composed of the methods name and the
  /// return and argument types of your handler.</summary>
  /// \b Advertise a Service
  /// \include example_qi_server.cpp
  /// \ingroup Messaging
  class QIMESSAGING_API Server {
  public:

    /// <summary> Constructs a Server object that can be used to
    /// advertise services to clients. </summary>
    /// <param name="name"> The name you want to give to the server. </param>
    /// <param name="context">
    /// An optional context that can be used to group or separate
    /// transport resources.
    /// </param>
    Server();
    virtual ~Server();

    /// <summary> Listen on the given address. </summary>
    /// <param name="session" the service directory session on which to advertise.</param>
    /// <param name="address"> the url to listen to. You can use
    /// a port value of 0 to let the system pick an available port.</param>
    bool listen(qi::Session *session, const std::string &address);
    void close();

    qi::Future<unsigned int> registerService(const std::string &name,
                                             qi::Object *obj);
    qi::Future<void>         unregisterService(unsigned int idx);

    std::vector<qi::ServiceInfo>  registeredServices();
    qi::ServiceInfo               registeredService(const std::string &service);
    qi::Object                   *registeredServiceObject(const std::string &service);

    qi::Url                       listenUrl() const;

  private:
    ServerPrivate *_p;
  };

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
    qi::Server                          _server;

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
