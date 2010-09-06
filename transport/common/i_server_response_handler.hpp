/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**  - Chris Kilner  <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#ifndef  AL_TRANSPORT_I_SERVER_RESPONSE_HANDLER_HPP_
# define AL_TRANSPORT_I_SERVER_RESPONSE_HANDLER_HPP_

/** Delegate that a server should implement to send a response
* this is mostly internal
*/
namespace AL {
  namespace Transport {

    namespace Detail
    {
      class IServerResponseHandler
      {
      public:
        virtual void serverResponseHandler(const std::string &result, void *data = 0) = 0;
      };
    }
  }
}


#endif  // AL_TRANSPORT_I_SERVER_RESPONSE_HANDLER_HPP_
