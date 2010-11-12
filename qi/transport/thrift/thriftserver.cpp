/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#include <allog/allog.h>
#include <qi/transport/thrift/thriftserver.hpp>

namespace qi {
  namespace transport {

    ThriftServer::ThriftServer(const std::string &server_name)
      : Server(server_name)
    {
    }

    ThriftServer::~ThriftServer () {
    }

    void ThriftServer::wait () {
    }

    void ThriftServer::stop () {
    }

    //use only the number of thread we need
    void ThriftServer::run() {
      alsdebug << "Start ThriftServer on: " << _serverAddress;
      //zsocket.bind(_serverAddress.c_str());

      while (true) {

//#ifdef ZMQ_FULL_ASYNC
//        handlersPool.pushTask(boost::shared_ptr<ZMQConnectionHandler>(new ZMQConnectionHandler(data, this->getDataHandler(), this, (void *)identity)));
//#else
//        ZMQConnectionHandler(data, this->getDataHandler(), this, (void *)0).run();
//#endif
      }
    }

    void ThriftServer::sendResponse(const std::string &result, void *data)
    {

    }


  }
}
