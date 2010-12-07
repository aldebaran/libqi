/*
*  Author(s):
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/

#include <qi/transport/detail/zmq/zmq_simple_server.hpp>
#include <qi/transport/detail/zmq/zmq_connection_handler.hpp>

#include <boost/interprocess/streams/bufferstream.hpp>

#include <zmq.hpp>

//#include <pthread.h>
#include <qi/log.hpp>
#include <boost/thread/mutex.hpp>
//#include <alfile/alfilesystem.h>

namespace qi {
  namespace transport {
    namespace detail {
      //if you use the custom XREP code, activate the full async experience to use the thread pool
      //#define ZMQ_FULL_ASYNC

      ZMQSimpleServerImpl::ZMQSimpleServerImpl(const std::vector<std::string> &serverAddresses)
        : ServerImpl(serverAddresses),
        zctx(1),
        zsocket(zctx, ZMQ_REP)
      {
      }

      ZMQSimpleServerImpl::~ZMQSimpleServerImpl () {
      }

      void ZMQSimpleServerImpl::wait () {
      }

      void ZMQSimpleServerImpl::stop () {
      }

      //use only the number of thread we need
      void ZMQSimpleServerImpl::run() {
        try {
          for(unsigned int i = 0; i < _serverAddresses.size(); ++i) {
            qisDebug << "Binding Server to: " << _serverAddresses[i] << std::endl;
            zsocket.bind(_serverAddresses[i].c_str());
          }
        } catch(const std::exception& e) {
          qisError << "Bind Server Failed to: " << std::endl;
          for(unsigned int i = 0; i < _serverAddresses.size(); ++i) {
            qisError << _serverAddresses[i].c_str() << std::endl;
          }
          qisError << " Reason: " << e.what() << std::endl;
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

      void ZMQSimpleServerImpl::serverResponseHandler(const std::string &result, void *data)
      {
        int                rc = 0;
        zmq::message_t     msg(result.size());

        memcpy(msg.data(), result.data(), result.size());
        rc = zsocket.send(msg);
        assert(rc > 0);
      }
    }
  }
}
