/*
** Author(s):
**  - Cedric GESTES      <gestes@aldebaran-robotics.com>
**  - Chris Kilner       <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#ifndef AL_MESSAGING_i_generic_message_handler_HPP_
#define AL_MESSAGING_i_generic_message_handler_HPP_

namespace AL {
  namespace Messaging {
    template<typename T, typename R>
    class IGenericMessageHandler {
    public:
      /// <summary> Generic message handler. </summary>
      /// <param name="requestMessage"> The request </param>
      /// <param name="responseMessage"> [in,out] The reply </param>
      virtual void messageHandler(const T &requestMessage, R& responseMessage) = 0;
    };
  }
}

#endif  // AL_MESSAGING_i_generic_message_handler_HPP_
