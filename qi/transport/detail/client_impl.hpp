/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#ifndef  QI_TRANSPORT_DETAIL_CLIENT_IMPL_HPP_
# define QI_TRANSPORT_DETAIL_CLIENT_IMPL_HPP_

#include <string>

namespace qi {
  namespace transport {
    namespace detail {

      template<typename Buffer = std::string>
      class ClientImpl {
      public:
        explicit ClientImpl(const std::string &serverAddress)
          : _serverAddress(serverAddress)
        {}

        virtual ~ClientImpl()
        {}

        virtual void send(const Buffer &tosend, Buffer &result) = 0;

      protected:
        std::string _serverAddress;
      };

    }
  }
}

#endif  // QI_TRANSPORT_CLIENT_HPP_
