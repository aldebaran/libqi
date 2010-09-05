/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#ifndef  AL_TRANSPORT_SERVER_RESPONSE_DELEGATE_HPP_
# define AL_TRANSPORT_SERVER_RESPONSE_DELEGATE_HPP_

/** Delegate that a server should implement to send a response
* this is mostly internal
*/
namespace AL {
  namespace Transport {

    namespace internal
    {
      class ServerResponseDelegate
      {
      public:
        virtual void sendResponse(const std::string &result, void *data = 0) = 0;
      };
    }
  }
}


#endif  // AL_TRANSPORT_SERVER_RESPONSE_DELEGATE_HPP_
