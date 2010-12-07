#pragma once
/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/


#ifndef _QI_TRANSPORT_TRANSPORT_SUBSCRIBE_HANDLER_HPP_
#define _QI_TRANSPORT_TRANSPORT_SUBSCRIBE_HANDLER_HPP_

#include <qi/transport/buffer.hpp>

namespace qi {
  namespace transport {

    class TransportSubscribeHandler {
    public:
      /// <summary> Generic subscribe handler. </summary>
      /// <param name="requestMessage"> The request </param>
      virtual void subscribeHandler(qi::transport::Buffer &requestMessage) = 0;
    };
  }
}

#endif  // _QI_TRANSPORT_TRANSPORT_SUBSCRIBE_HANDLER_HPP_
