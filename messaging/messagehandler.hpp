/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#ifndef AL_MESSAGING_MESSAGEHANDLER_HPP
#define AL_MESSAGING_MESSAGEHANDLER_HPP

#include <alcommon-ng/messaging/generic_messagehandler.hpp>
#include <alcommon-ng/messaging/call_definition.hpp>
#include <alcommon-ng/messaging/result_definition.hpp>

namespace AL {
  namespace Messaging {
    typedef GenericMessageHandler<AL::Messaging::CallDefinition, AL::Messaging::ResultDefinition> MessageHandler;
  }
}

#endif  // AL_MESSAGING_MESSAGEHANDLER_HPP
