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
#include <boost/thread.hpp>
#include <qimessaging/transport_socket.hpp>
#include <qimessaging/object.hpp>
#include <qimessaging/service_info.hpp>

namespace qi {

  struct ServiceRequest
  {
    qi::Promise<qi::Object *> promise;
    std::string               name;
    unsigned int              serviceId;
    qi::Url::Protocol         protocol;
  };

  class NetworkThread;
  class Session;
  class SessionInterface;
  class SessionPrivate : public qi::TransportSocketInterface {
  public:
    SessionPrivate(qi::Session *session);
    ~SessionPrivate();

    qi::TransportSocket* serviceSocket(const std::string &name,
                                       unsigned int      *idx,
                                       qi::Url::Protocol type);
    qi::Object* service(const std::string &service,
                        qi::Url::Protocol type);

    qi::Future<unsigned int>    registerService(const qi::ServiceInfo &si);
    qi::Future<void>            unregisterService(unsigned int idx);

    virtual void onSocketConnected(TransportSocket *client);
    virtual void onSocketConnectionError(TransportSocket *client);
    virtual void onSocketDisconnected(TransportSocket *client);
    virtual void onSocketReadyRead(TransportSocket *client, int id);

    void serviceRegisterUnregisterEnd(int id, qi::Message *msg,  qi::FunctorResult promise);


  public:
    qi::TransportSocket  *_serviceSocket;
    qi::NetworkThread    *_networkThread;
    qi::Session          *_self;
    qi::SessionInterface *_callbacks;

    boost::mutex                                               _mutexFuture;
    std::map<int, qi::Promise<std::vector<qi::ServiceInfo> > > _futureServices;
    std::map<int, qi::FunctorResult>                           _futureFunctor;
  };
}


#endif  // _SRC_SESSION_P_HPP_
