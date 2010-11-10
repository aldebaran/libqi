/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**  - Chris Kilner <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#ifndef QI_MESSAGING_i_message_handler_HPP
#define QI_MESSAGING_i_message_handler_HPP

#include <qi/messaging/i_generic_message_handler.hpp>
#include <qi/messaging/call_definition.hpp>
#include <qi/messaging/result_definition.hpp>

namespace qi {
  namespace Messaging {
    /// <summary>
    /// Define the IMessageHandler interface as being a specialization
    /// of the IGenericMessageHandler using a CallDefinition as the
    /// request and the a ResultDefinition as the response
    /// </summary>
    typedef IGenericMessageHandler<
      qi::Messaging::CallDefinition,
      qi::Messaging::ResultDefinition
    > IMessageHandler;
  }
}

#endif  // QI_MESSAGING_i_message_handler_HPP
