/*
** Author(s):
**  - Herve Cuche <hcuche@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#ifndef	GATEWAY_HPP_
# define GATEWAY_HPP_

# include <string>
# include <qimessaging/object.hpp>
# include <qimessaging/session.hpp>

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

    void start(qi::Session *session, const std::string &addr);
    void advertiseService(const std::string &name, qi::Object *obj);

  private:
    GatewayPrivate *_p;
  };
}

#endif // !GATEWAY_HPP_
