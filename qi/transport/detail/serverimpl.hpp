/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**  - Chris Kilner  <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#ifndef  QI_TRANSPORT_DETAIL_SERVERIMPL_HPP_
# define QI_TRANSPORT_DETAIL_SERVERIMPL_HPP_

#include <string>
#include <qi/core/runnable.hpp>
#include <qi/transport/message_handler.hpp>
#include <qi/transport/detail/server_response_handler.hpp>

namespace qi {
  namespace transport {
    namespace detail {

      class ServerImpl: public qi::Runnable {
      public:
        explicit ServerImpl(const std::string &_serverAddress)
          : _serverAddress(_serverAddress),
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
        std::string      _serverAddress;
        MessageHandler  *_dataHandler;
      };
    }
  }
}

#endif  // QI_TRANSPORT_SERVER_HPP_
