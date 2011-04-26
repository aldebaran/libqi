/*
*  Author(s):
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/

#include <cassert>
#include "src/transport/zmq/zmq_connection_handler.hpp"

namespace qi {
  namespace transport {
    namespace detail {
      ZMQConnectionHandler::ZMQConnectionHandler(const qi::transport::Buffer    &msg,
                                                 TransportMessageHandler        *dataHandler,
                                                 detail::ServerResponseHandler  *responseDelegate,
                                                 void                           *data)
        : _data(data),
          _msg(msg),
          _dataHandler(dataHandler),
          _responseDelegate(responseDelegate)
      {
        //this->setTaskName("ZMQConnectionHandler");
      }

      ZMQConnectionHandler::~ZMQConnectionHandler () {
      }

      void ZMQConnectionHandler::run() {
        assert(_dataHandler);
        qi::transport::Buffer result;

        _dataHandler->messageHandler(_msg, result);
        if (_responseDelegate)
          _responseDelegate->serverResponseHandler(result, _data);
      }
    }
  }
}
