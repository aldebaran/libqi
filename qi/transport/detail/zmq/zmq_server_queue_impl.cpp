/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/

#include <qi/transport/detail/zmq/zmq_server_queue_impl.hpp>
#include <qi/transport/detail/zmq/zmq_connection_handler.hpp>

#include <boost/interprocess/streams/bufferstream.hpp>

//#include <qi/serialization/definition_type.hpp>

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

      ZMQServerQueueImpl::ZMQServerQueueImpl (const std::vector<std::string> &serverAddresses)
        : detail::ServerImpl(serverAddresses),
        zctx(1),
        zsocketworkers(zctx, ZMQ_XREQ),
        zsocket(zctx, ZMQ_XREP)
      {
      }

      ZMQServerQueueImpl::~ZMQServerQueueImpl () {
      }

      void ZMQServerQueueImpl::wait () {
      }

      void ZMQServerQueueImpl::stop () {
      }

      void *worker_routine(void *arg)
      {
        int              rc = 0;
        ZMQServerImpl   *zserv = (ZMQServerImpl *)(arg);
        zmq::message_t   msg;
        zmq::socket_t    s(zserv->zctx, ZMQ_REP);

        s.connect(gWorkersAddress);
        // alsdebug << "ZMQ:entering worker loop";
        while (true) {
          rc = s.recv(&msg);
          assert(rc > 0);
          std::string data;
          data.assign((char *)msg.data(), msg.size());
          ZMQConnectionHandler(data, zserv->getDataHandler(), zserv, &s).run();
        }
      }

      void ZMQServerQueueImpl::run() {
        for(unsigned int i=0; i< _serverAddresses.size(); ++i) {
          qisDebug << "Start ZMQServer on: " << _serverAddresses[i] << std::endl;
          zsocket.bind(_serverAddresses[i].c_str());
        }
        zsocketworkers.bind(gWorkersAddress);

        // alsdebug << "ZMQ workers binded";

        pthread_t worker[gWorkersThreadsCount];
        for (int i = 0; i < gWorkersThreadsCount; ++i)
          pthread_create(&worker[i], NULL, worker_routine, (void*) this);
        //sleep(2);
        // alsdebug << "ZMQ: start queue_device";
        //zmq::device(ZMQ_QUEUE, workers, zsocket);
        zmq::device(ZMQ_QUEUE, zsocket, zsocketworkers);
        std::cout << "quit server" << std::endl;
      }

      void ZMQServerQueueImpl::serverResponseHandler(const std::string &result, void *data)
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
