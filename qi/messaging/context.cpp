/*
*  Author(s):
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/

#include <qi/messaging/context.hpp>
#include <qi/transport/detail/network/uuid.hpp>

using qi::detail::getUUID;

namespace qi {
  Context::Context() :
      _contextID(getUUID()) {}

  Context::Context(const Context& rhs) {
    _contextID = rhs.getID();
  }

  Context::~Context() {}

  const std::string& Context::getID() const {
    return _contextID;
  }
}

