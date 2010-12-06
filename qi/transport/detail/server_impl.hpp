#pragma once
/*
*  Author(s):
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/


#ifndef _QI_TRANSPORT_DETAIL_SERVER_IMPL_HPP_
#define _QI_TRANSPORT_DETAIL_SERVER_IMPL_HPP_

#include <string>
#include <vector>
#include <qi/core/runnable.hpp>
#include <qi/transport/message_handler.hpp>
#include <qi/transport/detail/server_response_handler.hpp>

namespace qi {
  namespace transport {
    namespace detail {

      class ServerImpl: public qi::detail::Runnable {
      public:
        explicit ServerImpl(const std::vector<std::string> &_serverAddresses)
          : _serverAddresses(_serverAddresses),
            _dataHandler(0) {}

        virtual ~ServerImpl() {}

        virtual void run() = 0;

        virtual void setDataHandler(MessageHandler* callback) {
          _dataHandler = callback;
        }

        virtual MessageHandler *getDataHandler() {
          return _dataHandler;
        }

      protected:
        std::vector<std::string>      _serverAddresses;
        MessageHandler  *_dataHandler;
      };
    }
  }
}

#endif  // _QI_TRANSPORT_DETAIL_SERVER_IMPL_HPP_
