#pragma once
/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/

#include <qi/messaging/publisher.hpp>
#include <qi/messaging/detail/publisher_impl.hpp>

namespace qi {
  Publisher::Publisher() {}

  Publisher::Publisher(
    const std::string& name,
    const std::string& masterAddress) :
    _impl(new qi::detail::PublisherImpl(name, masterAddress)) {}

  Publisher::~Publisher() {}

  void Publisher::xAdvertiseTopic(const std::string& signature) {
    _impl->advertiseTopic(signature);
  }

  void Publisher::xPublish(const std::string& message) {
    _impl->publish(message);
  }
}