#pragma once
/*
*  Author(s):
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/


#ifndef _QI_TRANSPORT_SRC_SERVER_BACKEND_HPP_
#define _QI_TRANSPORT_SRC_SERVER_BACKEND_HPP_

#include <string>
#include <vector>
#include <qi/core/runnable.hpp>
#include <qi/transport/transport_message_handler.hpp>
#include <qi/transport/src/server_response_handler.hpp>

namespace qi {
  namespace transport {
    namespace detail {


      class ServerBackend: public qi::detail::Runnable {
      public:
        explicit ServerBackend(const std::vector<std::string> &_serverAddresses)
          : _serverAddresses(_serverAddresses),
            _dataHandler(0) {}

        virtual ~ServerBackend() {}

        virtual void run() = 0;

        virtual void setDataHandler(TransportMessageHandler* callback) {
          _dataHandler = callback;
        }

        virtual TransportMessageHandler *getDataHandler() {
          return _dataHandler;
        }

      protected:
        std::vector<std::string>      _serverAddresses;
        TransportMessageHandler      *_dataHandler;
      };
    }
  }
}

#endif  // _QI_TRANSPORT_SRC_SERVER_BACKEND_HPP_
