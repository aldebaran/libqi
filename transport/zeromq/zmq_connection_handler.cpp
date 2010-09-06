/*
** Author(s):
**  - Cedric GESTES      <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#include <alcommon-ng/transport/zeromq/zmq_connection_handler.hpp>

namespace AL {
  namespace Transport {

    ZMQConnectionHandler::ZMQConnectionHandler(
      const std::string &msg,
      IDataHandler* dataHandler,
      internal::IServerResponseHandler* responseDelegate,
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
        fResponseDelegate->responseHandler(result, fData);
    }
  }
}
