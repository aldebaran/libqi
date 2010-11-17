/*
** Author(s):
**  - Chris Kilner  <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#ifndef QI_TRANSPORT_SUBSCRIBER_HPP_
# define QI_TRANSPORT_SUBSCRIBER_HPP_

#include <string>
#include <qi/transport/subscribe_handler.hpp>

namespace qi {
  namespace transport {

    class Subscriber {
    public:
      explicit Subscriber(const std::string &publishAddress)
        : _publishAddress(publishAddress),
          _subscribeHandler(0) {}
      virtual ~Subscriber() {}

      virtual void setSubscribeHandler(SubscribeHandler* callback) {
        _subscribeHandler = callback;
      }

      virtual SubscribeHandler* getSubscribeHandler() {
        return _subscribeHandler;
      }

      virtual void subscribe() = 0;

    protected:
      std::string        _publishAddress;
      SubscribeHandler*  _subscribeHandler;
    };
  }
}

#endif  // QI_TRANSPORT_SUBSCRIBER_HPP_
