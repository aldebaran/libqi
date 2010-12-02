#pragma once
/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/

#ifndef _QI_TRANSPORT_SUBSCRIBER_HPP_
#define _QI_TRANSPORT_SUBSCRIBER_HPP_

#include <string>
#include <qi/transport/subscribe_handler.hpp>

namespace qi {
  namespace transport {


    class Subscriber {
    public:
      Subscriber() : _subscribeHandler(NULL) {}
      explicit Subscriber(const Subscriber& rhs) : _subscribeHandler(rhs.getSubscribeHandler()) {}

      virtual ~Subscriber() {}

      virtual void setSubscribeHandler(SubscribeHandler* callback) {
        _subscribeHandler = callback;
      }

      virtual SubscribeHandler* getSubscribeHandler() const {
        return _subscribeHandler;
      }

      virtual void connect(const std::string &publishAddress) = 0;

      virtual void subscribe() = 0;

    protected:
      SubscribeHandler  *_subscribeHandler;
    };
  }
}

#endif  // _QI_TRANSPORT_SUBSCRIBER_HPP_
