/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#ifndef QI_MESSAGING_SERVER_HPP
#define QI_MESSAGING_SERVER_HPP

#include <qi/messaging/generic_server.hpp>
#include <string>

namespace qi {
  namespace messaging {
    typedef GenericServer<std::string, std::string> Server;
  }
}

#endif // QI_MESSAGING_SERVER_HPP
