/*
** Author(s):
**  - Chris Kilner  <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#ifndef   __QI_TRANSPORT_SUBSCRIBE_HANDLER_USER_HPP__
#define   __QI_TRANSPORT_SUBSCRIBE_HANDLER_USER_HPP__

#include <qi/transport/subscribe_handler.hpp>

namespace qi {
  namespace transport {

    class SubscribeHandlerUser {
    public:
      SubscribeHandlerUser() {}
      virtual ~SubscribeHandlerUser() {}
      virtual void setSubscribeHandler(SubscribeHandler* callback) = 0;
      virtual SubscribeHandler* getSubscribeHandler() const = 0;
    };
  }
}

#endif // __QI_TRANSPORT_SUBSCRIBE_HANDLER_USER_HPP__
