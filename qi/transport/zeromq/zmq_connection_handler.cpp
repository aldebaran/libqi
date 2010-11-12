/*
** Author(s):
**  - Cedric GESTES      <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#include <qi/transport/zeromq/zmq_connection_handler.hpp>

namespace qi {
  namespace transport {

    ZMQConnectionHandler::ZMQConnectionHandler(
      const std::string &msg,
      IDataHandler* dataHandler,
      Detail::IServerResponseHandler* responseDelegate,
      void *data)
      : fData(data),
      fMsg(msg),
      fDataHandler(dataHandler),
      fResponseDelegate(responseDelegate)
    {
      this->setTaskName("ZMQConnectionHandler");
    }

    ZMQConnectionHandler::~ZMQConnectionHandler () {
    }

    void ZMQConnectionHandler::run() {
      assert(fDataHandler);
      std::string result;

      fDataHandler->dataHandler(fMsg, result);
      if (fResponseDelegate)
        fResponseDelegate->serverResponseHandler(result, fData);
    }
  }
}
