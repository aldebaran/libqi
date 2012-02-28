/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#pragma once
#ifndef _SRC_SESSION_P_HPP_
#define _SRC_SESSION_P_HPP_

namespace qi {

  class SessionPrivate : public qi::TransportSocketInterface {
  public:
    SessionPrivate();
    ~SessionPrivate();

    bool connect(const std::string &masterAddress);
    qi::TransportSocket* serviceSocket(const std::string &name,
                                       unsigned int      *idx,
                                       qi::Url::Protocol type);
    qi::Object* service(const std::string &service,
                        qi::Url::Protocol type);
    std::vector<ServiceInfo> services();

    virtual void onConnected(TransportSocket *client);
    virtual void onDisconnected(TransportSocket *client);
    virtual void onWriteDone(TransportSocket *client);
    virtual void onReadyRead(TransportSocket *client, int id);

  public:
    qi::TransportSocket *_serviceSocket;
    qi::NetworkThread   *_networkThread;
  };

}


#endif  // _SRC_SESSION_P_HPP_
