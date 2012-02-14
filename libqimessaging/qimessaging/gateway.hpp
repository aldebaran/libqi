/*
** Author(s):
**  - Herve Cuche <hcuche@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#ifndef	GATEWAY_HPP_
# define GATEWAY_HPP_

#include <string>
#include <qimessaging/transport/transport_server.hpp>

namespace qi
{
  class NetworkThread;
  class Object;
  class GatewayPrivate;

  class Gateway
  {
  public:
    Gateway();
    virtual ~Gateway();

    void start(const std::string &addr, unsigned short port, NetworkThread *base);
    void advertiseService(const std::string &name, qi::Object *obj);

  private:
    GatewayPrivate *_p;
  };
}


//namespace qi
//{
//  class Gateway : public qi::TransportServer
//  {
//  public:
//    Gateway();
//    virtual ~Gateway();

//    virtual void onConnected(const qi::Message &msg);
//    virtual void onWrite(const qi::Message &msg);
//    virtual void onRead(const qi::Message &msg);

//    void start(const std::string &address,
//               unsigned short port,
//               struct event_base *base);

//    void registerGateway(const std::string &masterAddress,
//                         const std::string &gatewayAddress);
//    void unregisterGateway(const std::string &gatewayAddress);

//  private:
//    void services(const qi::Message &msg);
//    void service(const qi::Message &msg);

//  private:
//    qi::Session                                 _session;
//    qi::NetworkThread                          *_nthd;
//    qi::TransportServer                        *_ts;
//    std::vector<qi::EndpointInfo>               _endpoints;
//    std::map<std::string, qi::TransportSocket*> _serviceConnection;

//  }; // !gateway
//}; // !qi

#endif // !GATEWAY_HPP_
