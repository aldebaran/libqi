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

  class NetworkThread;
  class Session;
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

  public:
    qi::TransportSocket *_serviceSocket;
    qi::NetworkThread   *_networkThread;
    qi::Session         *_self;
  };

}


#endif  // _SRC_SESSION_P_HPP_
