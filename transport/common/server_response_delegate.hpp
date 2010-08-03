/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#ifndef   	SERVER_RESPONSE_DELEGATE_HPP_
# define   	SERVER_RESPONSE_DELEGATE_HPP_

#include <alcore/alptr.h>

/** Delegate that a server should implement to send a response
  * this is mostly internal
  */
namespace AL {
  namespace Messaging {

  class ConnectionHandler;
  class ResultDefinition;
  class CallDefinition;

  namespace internal
  {
    class ServerResponseDelegate
    {
    public:
      virtual void sendResponse(const CallDefinition &def, AL::ALPtr<ResultDefinition> result, void *data = 0) = 0;
    };
  }

}
}


#endif	    /* !SERVER_RESPONSE_DELEGATE_PP_ */
