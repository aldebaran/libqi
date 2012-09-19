/*
** Author(s):
**  - Herve Cuche <hcuche@aldebaran-robotics.com>
**  - Laurent Lec <llec@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#pragma once
#ifndef _QIMESSAGING_GATEWAY_HPP_
#define _QIMESSAGING_GATEWAY_HPP_

# include <qimessaging/url.hpp>
# include <qimessaging/transportserver.hpp>
# include <qimessaging/transportsocket.hpp>

namespace qi
{
  class GatewayPrivate;

  class QIMESSAGING_API Gateway
  {
  public:
    Gateway();
    ~Gateway();

    bool attachToServiceDirectory(const qi::Url &address);
    bool listen(const qi::Url &address);

  private:
    GatewayPrivate *_p;
  };

  class QIMESSAGING_API RemoteGateway
  {
  public:
    RemoteGateway();
    ~RemoteGateway();

    bool listen(const qi::Url &address);
    void join();

  private:
    GatewayPrivate *_p;
  };

  class QIMESSAGING_API ReverseGateway
  {
  public:
    ReverseGateway();
    ~ReverseGateway();

    bool attachToServiceDirectory(const qi::Url &address);
    bool connect(const qi::Url &address);
    void join();

  private:
    GatewayPrivate *_p;
  };
}

#endif  // _QIMESSAGING_GATEWAY_HPP_
