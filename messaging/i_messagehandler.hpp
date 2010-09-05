/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**  - Chris Kilner <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#ifndef AL_MESSAGING_I_MESSAGEHANDLER_HPP
#define AL_MESSAGING_I_MESSAGEHANDLER_HPP

#include <alcommon-ng/messaging/i_generic_messagehandler.hpp>
#include <alcommon-ng/messaging/call_definition.hpp>
#include <alcommon-ng/messaging/result_definition.hpp>

namespace AL {
  namespace Messaging {
    /// <summary>
    /// Define the IMessageHandler interface as being a specialization
    /// of the IGenericMessageHandler using a CallDefinition as the
    /// request and the a ResultDefinition as the response
    /// </summary>
    typedef IGenericMessageHandler<
      AL::Messaging::CallDefinition,
      AL::Messaging::ResultDefinition
    > IMessageHandler;
  }
}

#endif  // AL_MESSAGING_I_MESSAGEHANDLER_HPP
