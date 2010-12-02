/*
** Author(s):
**  - Cedric GESTES      <gestes@aldebaran-robotics.com>
**  - Chris Kilner       <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#ifndef   __QI_TRANSPORT_MESSAGE_HANDLER_HPP__
#define   __QI_TRANSPORT_MESSAGE_HANDLER_HPP__

# include <qi/transport/buffer.hpp>

namespace qi {
  namespace transport {

    class SubscribeHandler {
    public:
      /// <summary> Generic subscribe handler. </summary>
      /// <param name="requestMessage"> The request </param>
      virtual void subscribeHandler(qi::transport::Buffer &requestMessage) = 0;
    };
  }
}

#endif // __QI_TRANSPORT_MESSAGE_HANDLER_HPP__
