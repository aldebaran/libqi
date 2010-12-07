#pragma once
/*
*  Author(s):
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/


#ifndef _QI_TRANSPORT_DETAIL_CLIENT_IMPL_HPP_
#define _QI_TRANSPORT_DETAIL_CLIENT_IMPL_HPP_

#include <string>

namespace qi {
  namespace transport {
    namespace detail {

      class ClientBackend {
      public:
        explicit ClientBackend(const std::string &serverAddress)
          : _serverAddress(serverAddress)
        {}

        virtual ~ClientBackend()
        {}

        virtual void send(const qi::transport::Buffer &tosend, qi::transport::Buffer &result) = 0;

      protected:
        std::string _serverAddress;
      };

    }
  }
}

#endif  // _QI_TRANSPORT_DETAIL_CLIENT_IMPL_HPP_
