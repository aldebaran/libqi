/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#ifndef QI_MESSAGING_MESSAGING_CLIENT_HPP
#define QI_MESSAGING_MESSAGING_CLIENT_HPP

#include <qi/messaging/generic_client.hpp>
#include <string>

namespace qi {
  namespace messaging {
    typedef GenericClient<std::string, std::string> Client;
  }
}

#endif  // QI_MESSAGING_MESSAGING_CLIENT_HPP
