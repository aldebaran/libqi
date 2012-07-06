/*
** Author(s):
**  - Herve Cuche <hcuche@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#pragma once
#ifndef _QIMESSAGING_GATEWAY_HPP_
#define _QIMESSAGING_GATEWAY_HPP_

# include <qimessaging/url.hpp>

namespace qi
{
  class GatewayPrivate;

  class QIMESSAGING_API Gateway
  {
  public:
    Gateway();
    ~Gateway();

    bool listen(const qi::Url &listenAddress);
    bool listen(const qi::Url &listenAddress, const qi::Url &serviceDirectoryURL);
    void join();
    bool connect(const qi::Url &connectURL);

  private:
    GatewayPrivate *_p;
  }; // !Gateway
}; // !qi

#endif  // _QIMESSAGING_GATEWAY_HPP_
