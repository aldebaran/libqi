#pragma once
/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/

#ifndef _QI_TRANSPORT_DETAIL_CLIENT_IMPL_HPP_
#define _QI_TRANSPORT_DETAIL_CLIENT_IMPL_HPP_

#include <string>

namespace qi {
  namespace transport {
    namespace detail {

      template<typename BUFFER_TYPE = std::string>
      class ClientImpl {
      public:
        explicit ClientImpl(const std::string &serverAddress)
          : _serverAddress(serverAddress)
        {}

        virtual ~ClientImpl()
        {}

        virtual void send(const BUFFER_TYPE &tosend, BUFFER_TYPE &result) = 0;

      protected:
        std::string _serverAddress;
      };

    }
  }
}

#endif  // _QI_TRANSPORT_DETAIL_CLIENT_IMPL_HPP_
