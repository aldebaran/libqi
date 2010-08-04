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
    : msg(msg),
      callbackdelegate(callbackdelegate),
      data(data),
      responsedelegate(responsedelegate)
  {
    this->setTaskName("ZMQConnectionHandler");
  }

  ZMQConnectionHandler::~ZMQConnectionHandler () {
  }

  void ZMQConnectionHandler::run() {
    assert(callbackdelegate);
    std::string result;
    callbackdelegate->onData(msg, result);
    if (responsedelegate)
      responsedelegate->sendResponse(result, data);
  }
}
}
