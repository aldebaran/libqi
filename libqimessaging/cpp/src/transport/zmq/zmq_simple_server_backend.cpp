/*
*  Author(s):
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/

#include "src/transport/zmq/zmq_simple_server_backend.hpp"
#include "src/transport/zmq/zmq_connection_handler.hpp"
#include <boost/interprocess/streams/bufferstream.hpp>

#include <zmq.hpp>

#include <qi/os.hpp>
#include <qi/log.hpp>
#include <boost/thread/mutex.hpp>

namespace qi {
  namespace transport {
    namespace detail {
      //if you use the custom XREP code, activate the full async experience to use the thread pool
      //#define ZMQ_FULL_ASYNC

      ZMQSimpleServerBackend::ZMQSimpleServerBackend(const std::vector<std::string> &serverAddresses, zmq::context_t &context)
        : _running(false),
          ServerBackend(serverAddresses),
          _zcontext(context),
          _zsocket(_zcontext, ZMQ_REP)
      {
        int linger = 0;
#ifdef ZMQ_LINGER
        _zsocket.setsockopt(ZMQ_LINGER, &linger, sizeof(linger));
#endif
      }

      ZMQSimpleServerBackend::~ZMQSimpleServerBackend () {
        _running = false;
        qi::os::sleep(1); // allow the polling loop to terminate before killing the socket :(
      }

      void ZMQSimpleServerBackend::wait () {
      }

      void ZMQSimpleServerBackend::stop () {
      }

      bool ZMQSimpleServerBackend::poll(long timeout) {
        int             rc = 0;
        zmq_pollitem_t  items[1];

        items[0].socket  = _zsocket;
        items[0].fd      = 0;
        items[0].events  = ZMQ_POLLIN;
        items[0].revents = 0;

        // unfortunately there is an assert in getsockopt
        rc = zmq::poll(&items[0], 1, timeout);
        return (items[0].revents & ZMQ_POLLIN);
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
        _running = true;
        while (_running) {
          zmq::message_t  msg;

          try {
            bool isMessageReceived = false;
            while (! isMessageReceived) {
              isMessageReceived = poll(500 * 1000); // 0.5s in us
              if (!_running) {
                return;
              }
            }
            _zsocket.recv(&msg);
            std::string data;
            data.assign((char *)msg.data(), msg.size());

#ifdef ZMQ_FULL_ASYNC
            handlersPool.pushTask(boost::shared_ptr<ZMQConnectionHandler>(new ZMQConnectionHandler(data, this->getMessageHandler(), this, (void *)identity)));
#else
            ZMQConnectionHandler(data, this->getMessageHandler(), this, (void *)0).run();
#endif
          } catch(const zmq::error_t& e) {
            _running = false;
            qisError << "ZMQSimpleServerBackend::run Fatal error, stopping. Reason: " << e.what() << std::endl;
          }
        }
      }

      void ZMQSimpleServerBackend::serverResponseHandler(const std::string &result, void *data)
      {
        if (!_running) {
          return;
        }
        int            rc = 0;
        zmq::message_t msg(result.size());

        memcpy(msg.data(), result.data(), result.size());
        rc = _zsocket.send(msg);
        assert(rc > 0);
      }
    }
  }
}
