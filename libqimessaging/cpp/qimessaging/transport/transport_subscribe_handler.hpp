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

#include <qimessaging/transport/buffer.hpp>
#include <qimessaging/api.hpp>

namespace qi {
  namespace transport {



    /// <summary> Subscriber Interface. You need to create a class that inherit TransportSubscribeHandler
    /// and implement subscribeHandler. Each time the subscriber receive a request, it will call subscribeHandler
    /// with the message received.</summary>
    /// \ingroup Transport
    class QIMESSAGING_API TransportSubscribeHandler {
    public:
      /// <summary> Generic subscribe handler. </summary>
      /// <param name="requestMessage"> The request </param>
      virtual void subscribeHandler(qi::transport::Buffer &requestMessage) = 0;
    };
  }
}

#endif  // _QI_TRANSPORT_TRANSPORT_SUBSCRIBE_HANDLER_HPP_
