/*
*  Author(s):
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/

#include <qi/transport/src/zmq/zmq_simple_server.hpp>
#include <qi/transport/src/zmq/zmq_connection_handler.hpp>

#include <boost/interprocess/streams/bufferstream.hpp>

#include <zmq.hpp>

#include <qi/log.hpp>
#include <boost/thread/mutex.hpp>

namespace qi {
  namespace transport {
    namespace detail {
      //if you use the custom XREP code, activate the full async experience to use the thread pool
      //#define ZMQ_FULL_ASYNC

      ZMQSimpleServerBackend::ZMQSimpleServerBackend(const std::vector<std::string> &serverAddresses, zmq::context_t &context)
        : ServerBackend(serverAddresses),
          _zcontext(context),
          _zsocket(_zcontext, ZMQ_REP)
      {
        int linger = 0;
        _zsocket.setsockopt(ZMQ_LINGER, &linger, sizeof(linger));
      }

      ZMQSimpleServerBackend::~ZMQSimpleServerBackend () {
      }

      void ZMQSimpleServerBackend::wait () {
      }

      void ZMQSimpleServerBackend::stop () {
      }

      //use only the number of thread we need
      void ZMQSimpleServerBackend::run() {
        std::string currentAddress;
        try {
          for(unsigned int i = 0; i < _serverAddresses.size(); ++i) {
            currentAddress = _serverAddresses[i];
            qisDebug << "Binding Server to: " << currentAddress << std::endl;
            _zsocket.bind(currentAddress.c_str());
          }
        } catch(const std::exception& e) {
          qisError << "Bind Server Failed to: " << currentAddress <<
            " Reason: " << e.what() << std::endl;
          // TODO throw here ?
          return;
        }

#ifdef ZMQ_FULL_ASYNC
        alsdebug << "ZMQ: entering the loop (REP + growing thread mode)";
#else
        qisDebug << "ZMQ: entering the loop (REP)" << std::endl;
#endif
        while (true) {
          zmq::message_t  msg;
          _zsocket.recv(&msg);
          std::string data;
          data.assign((char *)msg.data(), msg.size());

#ifdef ZMQ_FULL_ASYNC
          handlersPool.pushTask(boost::shared_ptr<ZMQConnectionHandler>(new ZMQConnectionHandler(data, this->getMessageHandler(), this, (void *)identity)));
#else
          ZMQConnectionHandler(data, this->getMessageHandler(), this, (void *)0).run();
#endif
        }
      }

      void ZMQSimpleServerBackend::serverResponseHandler(const std::string &result, void *data)
      {
        int                rc = 0;
        zmq::message_t     msg(result.size());

        memcpy(msg.data(), result.data(), result.size());
        rc = _zsocket.send(msg);
        assert(rc > 0);
      }
    }
  }
}
