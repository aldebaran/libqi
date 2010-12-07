/*
*  Author(s):
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/

#include <qi/messaging/publisher.hpp>
#include <qi/messaging/src/publisher_impl.hpp>

namespace qi {

  Publisher::Publisher(const std::string& name, Context *ctx)
    : _impl(new qi::detail::PublisherImpl(name, ctx)) {}

  Publisher::~Publisher() {}

  void Publisher::xAdvertiseTopic(const std::string& signature) {
    _impl->advertiseTopic(signature);
  }

  void Publisher::xPublish(const std::string& message) {
    _impl->publish(message);
  }
}
