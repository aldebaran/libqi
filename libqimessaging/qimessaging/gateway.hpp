/*
** Author(s):
**  - Herve Cuche <hcuche@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#pragma once
#ifndef _QIMESSAGING_GATEWAY_HPP_
#define _QIMESSAGING_GATEWAY_HPP_

# include <string>
# include <qimessaging/api.hpp>
# include <qimessaging/object.hpp>
# include <qimessaging/session.hpp>

namespace qi
{
  class NetworkThread;
  class Object;
  class GatewayPrivate;

  class QIMESSAGING_API Gateway
  {
  public:
    Gateway();
    virtual ~Gateway();

    void listen(qi::Session *session, const std::string &addr);
    void advertiseService(const std::string &name, qi::Object *obj);

  private:
    GatewayPrivate *_p;
  };
}

#endif  // _QIMESSAGING_GATEWAY_HPP_
