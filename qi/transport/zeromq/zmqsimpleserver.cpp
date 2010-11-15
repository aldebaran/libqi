/*
** Author(s):
**  - Cedric GESTES      <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#include <qi/transport/zeromq/zmqsimpleserver.hpp>
#include <qi/transport/zeromq/zmq_connection_handler.hpp>

#include <boost/interprocess/streams/bufferstream.hpp>

#include <zmq.hpp>

//#include <pthread.h>
#include <qi/log.hpp>
#include <boost/thread/mutex.hpp>
//#include <alfile/alfilesystem.h>

namespace qi {
  namespace transport {

    //if you use the custom XREP code, activate the full async experience to use the thread pool
    //#define ZMQ_FULL_ASYNC

    ZMQSimpleServer::ZMQSimpleServer(const std::string &serverAddress)
      : Server(serverAddress),
        zctx(1),
        zsocket(zctx, ZMQ_REP)
    {
    }

    ZMQSimpleServer::~ZMQSimpleServer () {
    }

    void ZMQSimpleServer::wait () {
    }

    void ZMQSimpleServer::stop () {
    }

    //use only the number of thread we need
    void ZMQSimpleServer::run() {
      try {
        qisDebug << "Start ZMQServer on: " << _serverAddress << std::endl;
        zsocket.bind(_serverAddress.c_str());
      } catch(const std::exception& e) {
        qisError << "Failed to bind to address " << _serverAddress << " Reason: " << e.what() << std::endl;
        return;
      }

#ifdef ZMQ_FULL_ASYNC
      alsdebug << "ZMQ: entering the loop (REP + growing thread mode)";
#else
      qisDebug << "ZMQ: entering the loop (REP)" << std::endl;
#endif
      while (true) {
        zmq::message_t  msg;
        zsocket.recv(&msg);
        std::string data;
        data.assign((char *)msg.data(), msg.size());

#ifdef ZMQ_FULL_ASYNC
        handlersPool.pushTask(boost::shared_ptr<ZMQConnectionHandler>(new ZMQConnectionHandler(data, this->getDataHandler(), this, (void *)identity)));
#else
        ZMQConnectionHandler(data, this->getDataHandler(), this, (void *)0).run();
#endif
      }

    }

    void ZMQSimpleServer::serverResponseHandler(const std::string &result, void *data)
    {
      int                rc = 0;
      zmq::message_t     msg(result.size());

      memcpy(msg.data(), result.data(), result.size());
      rc = zsocket.send(msg);
      assert(rc > 0);
    }

  }
}

