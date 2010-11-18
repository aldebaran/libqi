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

    class MessageHandler {
    public:
      /// <summary> Generic message handler. </summary>
      /// <param name="requestMessage"> The request </param>
      /// <param name="responseMessage"> [in,out] The reply </param>
      virtual void messageHandler(qi::transport::Buffer &requestMessage, qi::transport::Buffer& responseMessage) = 0;
    };
  }
}

#endif // __QI_TRANSPORT_MESSAGE_HANDLER_HPP__
