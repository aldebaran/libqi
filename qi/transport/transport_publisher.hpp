#pragma once
/*
*  Author(s):
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/


#ifndef _QI_TRANSPORT_PUBLISHER_HPP_
#define _QI_TRANSPORT_PUBLISHER_HPP_

#include <string>
#include <vector>

namespace qi {
  namespace transport {

    namespace detail {
      class PublisherBackend;
    }

    class TransportPublisher {
    public:
      TransportPublisher();
      ~TransportPublisher();
      virtual void connect(const std::string& serverAddresses);
      virtual void bind(const std::string& serverAddress);
      virtual void bind(const std::vector<std::string>& serverAddresses);
      virtual void publish(const std::string& tosend);

    protected:
      qi::transport::detail::PublisherBackend *_publisher;
    };
  }
}

#endif  // _QI_TRANSPORT_PUBLISHER_HPP_
