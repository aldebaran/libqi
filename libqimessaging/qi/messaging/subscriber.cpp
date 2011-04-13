/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/

#include <qi/messaging/subscriber.hpp>
#include <qi/messaging/context.hpp>
#include <qi/messaging/src/subscriber_impl.hpp>

namespace qi {

  Subscriber::~Subscriber() {

  }

  Subscriber::Subscriber(const std::string& name, Context *ctx)
    : _impl(new detail::SubscriberImpl(name, ctx))
  {
  }

  void Subscriber::connect(const std::string &masterAddress) {
    _impl->connect(masterAddress);
  }

  void Subscriber::xSubscribe(const std::string& signature, qi::Functor* f) {
    _impl->subscribe(signature, f);
  }

  void Subscriber::xUnsubscribe(const std::string& signature) {
    _impl->unsubscribe(signature);
  }

  bool Subscriber::isInitialized() const {
    return _impl->isInitialized();
  }


}
