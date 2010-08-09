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
      DataHandler *callbackdelegate,
      internal::ServerResponseDelegate *responsedelegate,
      void *data)
    : _data(data),
      _msg(msg),
      _dataHandler(callbackdelegate),
      _responsedelegate(responsedelegate)
  {
    this->setTaskName("ZMQConnectionHandler");
  }

  ZMQConnectionHandler::~ZMQConnectionHandler () {
  }

  void ZMQConnectionHandler::run() {
    assert(_dataHandler);
    std::string result;

    _dataHandler->onData(_msg, result);
    if (_responsedelegate)
      _responsedelegate->sendResponse(result, _data);
  }
}
}
