#pragma once
/*
*  Author(s):
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*
*  Copyright (C) 2010, 2011 Aldebaran Robotics
*/


#ifndef _QI_TRANSPORT_SRC_FORWARDER_BACKEND_HPP_
#define _QI_TRANSPORT_SRC_FORWARDER_BACKEND_HPP_

#include <string>
#include <vector>
#include "src/transport/runnable.hpp"

namespace qi {
  namespace transport {
    namespace detail {

      class ForwarderBackend: public qi::detail::Runnable {
      public:
        ForwarderBackend(const ForwarderBackend& rhs) :
            _inAddresses(rhs._inAddresses),
            _outAddresses(rhs._outAddresses) {}

        ForwarderBackend(
          const std::vector<std::string>& inAddresses,
          const std::vector<std::string>& outAddresses) :
            _inAddresses(inAddresses),
            _outAddresses(outAddresses) {}

        virtual ~ForwarderBackend() {}
        virtual void bind() = 0;
        virtual void run()  = 0;

      protected:
        std::vector<std::string> _inAddresses;
        std::vector<std::string> _outAddresses;
      };
    }
  }
}

#endif  // _QI_TRANSPORT_SRC_FORWARDER_BACKEND_HPP_
