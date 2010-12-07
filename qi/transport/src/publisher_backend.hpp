#pragma once
/*
*  Author(s):
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/


#ifndef _QI_TRANSPORT_SRC_PUBLISHER_BACKEND_HPP_
#define _QI_TRANSPORT_SRC_PUBLISHER_BACKEND_HPP_

#include <string>
#include <vector>

namespace qi {
  namespace transport {

    namespace detail {
      class PublisherBackend {
      public:
        virtual void connect(const std::string& serverAddresses) = 0;
        virtual void bind(const std::string& serverAddress) = 0;
        virtual void bind(const std::vector<std::string>& serverAddresses) = 0;
        virtual void publish(const std::string& tosend) = 0;
      };
    }

  }
}

#endif  // _QI_TRANSPORT_SRC_PUBLISHER_BACKEND_HPP_
