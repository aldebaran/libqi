/*
*  Author(s):
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/

#include <cassert>
#include <qi/transport/src/zmq/zmq_connection_handler.hpp>

namespace qi {
  namespace transport {
    namespace detail {
      ZMQConnectionHandler::ZMQConnectionHandler(
        const qi::transport::Buffer    &msg,
         TransportMessageHandler        *dataHandler,
         detail::ServerResponseHandler  *responseDelegate,
         void                           *data)
           : fData(data),
             fMsg(msg),
             fDataHandler(dataHandler),
             fResponseDelegate(responseDelegate)
      {
        //this->setTaskName("ZMQConnectionHandler");
      }

      ZMQConnectionHandler::~ZMQConnectionHandler () {
      }

      void ZMQConnectionHandler::run() {
        assert(fDataHandler);
        qi::transport::Buffer result;

        fDataHandler->messageHandler(fMsg, result);
        if (fResponseDelegate)
          fResponseDelegate->serverResponseHandler(result, fData);
      }
    }
  }
}
