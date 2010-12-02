/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/

#include <qi/messaging/subscriber.hpp>
#include <qi/messaging/detail/subscriber_impl.hpp>

namespace qi {
  Subscriber::Subscriber() {}

  Subscriber::~Subscriber() {}

  Subscriber::Subscriber(const std::string& name,
                         const std::string& masterAddress)
    : _impl(new detail::SubscriberImpl(name, masterAddress))
  {}

  void Subscriber::xSubscribe(const std::string& topicName, qi::Functor* f) {
    return _impl->subscribe(topicName, f);
  }

  bool Subscriber::isInitialized() const {
    return _impl->isInitialized();
  }
}
