/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#pragma once
#ifndef _QIMESSAGING_SESSION_HPP_
#define _QIMESSAGING_SESSION_HPP_

#include <qimessaging/transport_socket.hpp>
#include <qimessaging/service_info.hpp>

#include <vector>
#include <string>

namespace qi {

  class NetworkThread;
  class Object;

  class Session : public qi::TransportSocketInterface {
  public:
    Session();
    virtual ~Session();

    void onConnected(TransportSocket *client);
    void onDisconnected(TransportSocket *client);
    void onWriteDone(TransportSocket *client);
    void onReadyRead(TransportSocket *client, qi::Message &msg);


    void connect(const std::string &masterAddress);
    bool disconnect();
    void join();
    bool waitForConnected(int msecs = 30000);
    bool waitForDisconnected(int msecs = 30000);

    std::vector<ServiceInfo> services();

    qi::TransportSocket* serviceSocket(const std::string &name,
                                       unsigned int      *idx,
                                       qi::Url::Protocol  type = qi::Url::Protocol_Tcp);

    qi::Object* service(const std::string &service,
                        qi::Url::Protocol  type = qi::Url::Protocol_Tcp);

    qi::TransportSocket *tc;
    qi::NetworkThread   *_nthd;
  };
}


#endif  // _QIMESSAGING_SESSION_HPP_
