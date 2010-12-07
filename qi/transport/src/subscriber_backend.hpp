#pragma once
/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/


#ifndef _QI_TRANSPORT_SRC_SUBSCRIBER_BACKEND_HPP_
#define _QI_TRANSPORT_SRC_SUBSCRIBER_BACKEND_HPP_

#include <string>
#include <qi/transport/transport_subscribe_handler.hpp>

namespace qi {
  namespace transport {
    namespace detail {

      class SubscriberBackend {
      public:
        SubscriberBackend() : _subscribeHandler(NULL) {}

        virtual ~SubscriberBackend() {}

        virtual void setSubscribeHandler(TransportSubscribeHandler* callback) {
          _subscribeHandler = callback;
        }

        virtual TransportSubscribeHandler* getSubscribeHandler() const {
          return _subscribeHandler;
        }

        virtual void connect(const std::string &publishAddress) = 0;

        virtual void subscribe() = 0;

      protected:
        TransportSubscribeHandler  *_subscribeHandler;
      };
    }
  }
}

#endif  // _QI_TRANSPORT_SRC_SUBSCRIBER_BACKEND_HPP_
