/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/

#include "src/transport/zmq/zmq_server_queue_backend.hpp"
#include "src/transport/zmq/zmq_connection_handler.hpp"

#include <boost/interprocess/streams/bufferstream.hpp>

//#include <qimessaging/serialization/definition_type.hpp>

#include <zmq.hpp>

#include <pthread.h>
#include <qi/log.hpp>
#include <boost/thread/mutex.hpp>
#include <iostream>
//#include <alfile/alfilesystem.h>

namespace qi {
  namespace transport {
    namespace detail {
      static const char *gWorkersAddress      = "inproc://workers";
      static const int   gWorkersThreadsCount = 10;

      ZMQServerQueueBackend::ZMQServerQueueBackend(const std::vector<std::string> &serverAddresses, zmq::context_t &context)
        : detail::ServerBackend(serverAddresses),
          _zcontext(context),
          _zsocketworkers(_zcontext, ZMQ_XREQ),
          _zsocket(_zcontext, ZMQ_XREP)
      {
        int linger = 0;
#ifdef ZMQ_LINGER
        _zsocket.setsockopt(ZMQ_LINGER, &linger, sizeof(linger));
#endif
      }

      ZMQServerQueueBackend::~ZMQServerQueueBackend () {
      }

      void ZMQServerQueueBackend::wait () {
      }

      void ZMQServerQueueBackend::stop () {
      }

      void *worker_routine(void *arg)
      {
        int                   rc = 0;
        ZMQServerQueueBackend *zserv = (ZMQServerQueueBackend *)(arg);
        zmq::message_t        msg;
        zmq::socket_t         s(zserv->_zcontext, ZMQ_REP);

        s.connect(gWorkersAddress);
        // alsdebug << "ZMQ:entering worker loop";
        while (true) {
          rc = s.recv(&msg);
          assert(rc > 0);
          std::string data;
          data.assign((char *)msg.data(), msg.size());
          ZMQConnectionHandler(data, zserv->getMessageHandler(), zserv, &s).run();
        }
      }

      void ZMQServerQueueBackend::run() {
        for(unsigned int i=0; i< _serverAddresses.size(); ++i) {
          qisDebug << "Start ZMQServer on: " << _serverAddresses[i] << std::endl;
          _zsocket.bind(_serverAddresses[i].c_str());
        }
        _zsocketworkers.bind(gWorkersAddress);

        // alsdebug << "ZMQ workers binded";

        pthread_t worker[gWorkersThreadsCount];
        for (int i = 0; i < gWorkersThreadsCount; ++i)
          pthread_create(&worker[i], NULL, worker_routine, (void*) this);
        //sleep(2);
        // alsdebug << "ZMQ: start queue_device";
        //zmq::device(ZMQ_QUEUE, workers, zsocket);
        zmq::device(ZMQ_QUEUE, _zsocket, _zsocketworkers);
        std::cout << "quit server" << std::endl;
      }

      void ZMQServerQueueBackend::serverResponseHandler(const std::string &result, void *data)
      {
        int                rc = 0;
        zmq::message_t     msg(result.size());

        memcpy(msg.data(), result.data(), result.size());
        zmq::socket_t     *sock = static_cast<zmq::socket_t *>(data);

        assert(data);
        // alsdebug << "ZMQ: send response";
        rc = sock->send(msg);
        assert(rc > 0);
      }
    }
  }
}
