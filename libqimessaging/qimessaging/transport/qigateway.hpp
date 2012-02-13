/*
** Author(s):
**  - Herve Cuche <hcuche@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#ifndef	QIGATEWAY_HPP_
# define QIGATEWAY_HPP_

# include <vector>
# include <map>
# include <string>

# include <qimessaging/transport.hpp>
# include <qimessaging/datastream.hpp>
# include <qimessaging/session.hpp>

namespace qi
{
  class Gateway : public qi::TransportServer
  {
  public:
    Gateway();
    virtual ~Gateway();

    virtual void onConnected(const qi::Message &msg);
    virtual void onWrite(const qi::Message &msg);
    virtual void onRead(const qi::Message &msg);

    void start(const std::string &address,
               unsigned short port,
               struct event_base *base);

    void registerGateway(const std::string &masterAddress,
                         const std::string &gatewayAddress);
    void unregisterGateway(const std::string &gatewayAddress);

  private:
    void services(const qi::Message &msg);
    void service(const qi::Message &msg);

  private:
    qi::Session                                 _session;
    qi::NetworkThread                          *_nthd;
    qi::TransportServer                        *_ts;
    std::vector<qi::EndpointInfo>               _endpoints;
    std::map<std::string, qi::TransportSocket*> _serviceConnection;

  }; // !gateway
}; // !qi

#endif // !QIGATEWAY_HPP_
