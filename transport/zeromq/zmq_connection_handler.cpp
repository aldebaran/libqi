/*
** Author(s):
**  - Cedric GESTES      <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#include <alcommon-ng/transport/zeromq/zmq_connection_handler.hpp>

namespace AL {
  namespace Transport {

    ZMQConnectionHandler::ZMQConnectionHandler(const std::string &msg, OnDataDelegate *callbackdelegate, internal::ServerResponseDelegate *responsedelegate, void *data)
    : _msg(msg),
      _callbackdelegate(callbackdelegate),
      _data(data),
      _responsedelegate(responsedelegate)
  {
    this->setTaskName("ZMQConnectionHandler");
  }

  ZMQConnectionHandler::~ZMQConnectionHandler () {
  }

  void ZMQConnectionHandler::run() {
    assert(_callbackdelegate);
    std::string result;

    _callbackdelegate->onData(_msg, result);
    if (_responsedelegate)
      _responsedelegate->sendResponse(result, _data);
  }
}
}
