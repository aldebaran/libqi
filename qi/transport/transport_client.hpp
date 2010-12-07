#pragma once
/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/


#ifndef _QI_TRANSPORT_TRANSPORT_CLIENT_HPP_
#define _QI_TRANSPORT_TRANSPORT_CLIENT_HPP_

#include <string>

namespace qi {
  namespace transport {

    namespace detail {
      class ClientBackend;
    }

    //TODO: bad!
    typedef std::string Buffer;

    class TransportClient {
    public:
      TransportClient();

      bool connect(const std::string &address);

      void send(const qi::transport::Buffer &def, qi::transport::Buffer &result);

    protected:
     bool                                  _isInitialized;
     qi::transport::detail::ClientBackend *_client;
    };

  }
}

#endif  // _QI_TRANSPORT_TRANSPORT_CLIENT_HPP_
