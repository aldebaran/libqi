#pragma once
/*
*  Author(s):
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/


#ifndef _QI_TRANSPORT_DETAIL_SERVER_RESPONSE_HANDLER_HPP_
#define _QI_TRANSPORT_DETAIL_SERVER_RESPONSE_HANDLER_HPP_

#include <string>

/** Delegate that a server should implement to send a response
* this is mostly internal
*/
namespace qi {
  namespace transport {
    namespace detail
    {
      class ServerResponseHandler
      {
      public:
        virtual void serverResponseHandler(const std::string &result, void *data = 0) = 0;
      };
    }
  }
}


#endif  // _QI_TRANSPORT_DETAIL_SERVER_RESPONSE_HANDLER_HPP_
