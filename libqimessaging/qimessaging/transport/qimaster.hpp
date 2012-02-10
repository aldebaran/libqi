/*
** Author(s):
**  - Herve Cuche <hcuche@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#ifndef QIMASTER_HPP_
# define QIMASTER_HPP_

#include <iostream>
#include <vector>
#include <map>

#include <qimessaging/transport.hpp>
#include <qimessaging/session.hpp>
#include <qimessaging/datastream.hpp>

namespace qi
{
  class ServiceDirectoryServer : public qi::TransportServerDelegate
  {
  public:
    ServiceDirectoryServer();
    virtual ~ServiceDirectoryServer();

    void start(const std::string &address);

    virtual void onConnected(const qi::Message &msg);
    virtual void onWrite(const qi::Message &msg);
    virtual void onRead(const qi::Message &msg);

  private:
    void services(const qi::Message &msg);
    void service(const qi::Message &msg);

    void registerEndpoint(const qi::Message &msg);
    void unregisterEndpoint(const qi::Message &msg);

  private:
    qi::NetworkThread                     *nthd;
    qi::TransportServer                   *ts;
    std::map<std::string, qi::ServiceInfo> connectedServices;

  }; // !ServiceDirectoryServer
}; // !qi

#endif // !QIMASTER_HPP_
