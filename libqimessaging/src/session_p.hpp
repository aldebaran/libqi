/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#pragma once
#ifndef _SRC_SESSION_P_HPP_
#define _SRC_SESSION_P_HPP_

#include <qimessaging/object.hpp>
#include <qimessaging/service_info.hpp>

namespace qi {

  class NetworkThread;
  class Session;
  class SessionInterface;
  class SessionPrivate : public qi::TransportSocketInterface {
  public:
    SessionPrivate(qi::Session *session);
    ~SessionPrivate();

    bool connect(const qi::Url &serviceDirectoryURL);
    qi::TransportSocket* serviceSocket(const std::string &name,
                                       unsigned int      *idx,
                                       qi::Url::Protocol type);
    qi::Object* service(const std::string &service,
                        qi::Url::Protocol type);
    std::vector<ServiceInfo> services();

    virtual void onSocketConnected(TransportSocket *client);
    virtual void onSocketConnectionError(TransportSocket *client);
    virtual void onSocketDisconnected(TransportSocket *client);

  public:
    qi::TransportSocket  *_serviceSocket;
    qi::NetworkThread    *_networkThread;
    qi::Session          *_self;
    qi::SessionInterface *_callbacks;
  };

}


#endif  // _SRC_SESSION_P_HPP_
