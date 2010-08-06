/*
** Author(s):
**  - Cedric GESTES      <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#include <alcommon-ng/transport/zeromq/zmqserverqueue.hpp>
#include <alcommon-ng/transport/zeromq/zmq_connection_handler.hpp>

#include <boost/interprocess/streams/bufferstream.hpp>

#include <alcommon-ng/serialization/definition_type.hpp>

#include <zmq.hpp>

#include <pthread.h>
#include <allog/allog.h>
#include <boost/thread/mutex.hpp>
#include <alfile/alfilesystem.h>

namespace AL {
  namespace Transport {

    static const char *gWorkersAddress      = "inproc://workers";
    static const int   gWorkersThreadsCount = 10;

    ZMQServerQueue::ZMQServerQueue (const std::string &server_name)
      : Server(server_name),
        zctx(1),
        zsocketworkers(zctx, ZMQ_XREQ),
        zsocket(zctx, ZMQ_XREP)
    {
      ALPath mypath = ALFileSystem::getTmpPath();
      mypath /= server_name;
#ifdef WIN32
      // ck horrid hack so that naoqi excecutes
      server_path = "tcp://127.0.0.1:5555";
#else
      server_path = "ipc://" + mypath.string();
#endif
    }

    ZMQServerQueue::~ZMQServerQueue () {
    }

    void ZMQServerQueue::wait () {
    }

    void ZMQServerQueue::stop () {
    }

    void *worker_routine(void *arg)
    {
      int              rc = 0;
      ZMQServer       *zserv = (ZMQServer *)(arg);
      zmq::message_t   msg;
      zmq::socket_t    s(zserv->zctx, ZMQ_REP);

      s.connect(gWorkersAddress);
      alsdebug << "ZMQ:entering worker loop";
      while (true) {
        rc = s.recv(&msg);
        assert(rc > 0);
        std::string data;
        data.assign((char *)msg.data(), msg.size());
        ZMQConnectionHandler(data, zserv->getDataHandler(), zserv, &s).run();
      }
    }

    void ZMQServerQueue::run() {
      alsdebug << "Start ZMQServer on: " << _serverAddress;
      zsocket.bind(_serverAddress.c_str());
      zsocketworkers.bind(gWorkersAddress);

      alsdebug << "ZMQ workers binded";

      pthread_t worker[gWorkersThreadsCount];
      for (int i = 0; i < gWorkersThreadsCount; ++i)
        pthread_create(&worker[i], NULL, worker_routine, (void*) this);
      //sleep(2);
      alsdebug << "ZMQ: start queue_device";
      //zmq::device(ZMQ_QUEUE, workers, zsocket);
      zmq::device(ZMQ_QUEUE, zsocket, zsocketworkers);
      std::cout << "quit server" << std::endl;
    }

    void ZMQServerQueue::sendResponse(const std::string &result, void *data)
    {
      int                rc = 0;
      zmq::message_t     msg(result.size());

      memcpy(msg.data(), result.data(), result.size());
      zmq::socket_t     *sock = static_cast<zmq::socket_t *>(data);

      assert(data);
      alsdebug << "ZMQ: send response";
      rc = sock->send(msg);
      assert(rc > 0);
    }
  }
}

//PLAYGROUND
namespace AL {
  namespace Messaging {
    /*
     //use ZMQ_REP
     //single threaded version (just for test)
      void ZMQServer::run-single-thread() {
        std::cout << "enterring DA LOOZ" << std::endl;
        while (true) {

          rc = zsocket.recv(&msg);
          std::cout << "Receive(size=" << msg.size() << ")" << std::endl;

          for (int i = 0; i < msg.size(); ++i)
            printf("0x%.2x\n", ((char *)msg.data())[i]);
          boost::shared_ptr<ippc::CallDefinition>   def(new ippc::CallDefinition());
          boost::interprocess::bufferstream bstream((char *)msg.data(), msg.size());
          ippc::IArchive                    archive(bstream);

          //unmarshall the message
          archive >> *def;
          //handlersPool.pushTask(boost::shared_ptr<ippc::ZMQConnectionHandler>(new ippc::ZMQConnectionHandler(*def, this->getCommandDelegate(), this)));
          ippc::ZMQConnectionHandler(*def, this->getCommandDelegate(), this).run();
        }
      }
    */
  }
}




