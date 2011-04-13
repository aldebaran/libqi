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

  void Publisher::connect(const std::string &masterAddress) {
    _impl->connect(masterAddress);
  }

  void Publisher::xAdvertiseTopic(const std::string& topicSignature,
    const bool& isManyToMany) {
    _impl->advertiseTopic(topicSignature, isManyToMany);
  }

  void Publisher::xUnadvertiseTopic(const std::string& topicSignature) {
    _impl->unadvertiseTopic(topicSignature);
  }

  void Publisher::xPublish(const std::string& topicSignature, const std::string& message) {
    _impl->publish(topicSignature, message);
  }
}
