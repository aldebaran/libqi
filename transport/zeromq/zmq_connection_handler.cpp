/*
** Author(s):
**  - Cedric GESTES      <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#include <alippc/transport/zeromq/zmq_connection_handler.hpp>

namespace AL {
  namespace Messaging {

    ZMQConnectionHandler::ZMQConnectionHandler(AL::ALPtr<CallDefinition> def, ServerCommandDelegate *callbackdelegate, internal::ServerResponseDelegate *responsedelegate, void *data)
    : def(def),
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
    AL::ALPtr<ResultDefinition> result = callbackdelegate->ippcCallback(*def);
    if (result && responsedelegate)
      responsedelegate->sendResponse(*def, result, data);
  }
}
}
