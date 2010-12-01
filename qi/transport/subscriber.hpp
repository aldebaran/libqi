/*
** Author(s):
**  - Chris Kilner  <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#ifndef   __QI_TRANSPORT_SUBSCRIBER_HPP__
#define   __QI_TRANSPORT_SUBSCRIBER_HPP__

#include <string>
#include <qi/transport/subscribe_handler_user.hpp>

namespace qi {
  namespace transport {

    class Subscriber : SubscribeHandlerUser {
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
      SubscribeHandler* _subscribeHandler;
    };
  }
}

#endif // __QI_TRANSPORT_SUBSCRIBER_HPP__
