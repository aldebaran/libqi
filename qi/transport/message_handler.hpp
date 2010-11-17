/*
** Author(s):
**  - Cedric GESTES      <gestes@aldebaran-robotics.com>
**  - Chris Kilner       <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#ifndef QI_TRANSPORT_I_MESSAGE_HANDLER_HPP_
# define QI_TRANSPORT_I_MESSAGE_HANDLER_HPP_

# include <qi/transport/buffer.hpp>

namespace qi {
  namespace transport {

    class MessageHandler {
    public:
      /// <summary> Generic message handler. </summary>
      /// <param name="requestMessage"> The request </param>
      /// <param name="responseMessage"> [in,out] The reply </param>
      virtual void messageHandler(const qi::transport::Buffer &request, qi::transport::Buffer& responseMessage) = 0;
    };
  }
}

#endif  // QI_MESSAGING_I_GENERIC_MESSAGE_HANDLER_HPP_
