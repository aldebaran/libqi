/*
** Author(s):
**  - Cedric GESTES      <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#include <alcommon-ng/transport/zeromq/zmqserver.hpp>
#include <alcommon-ng/transport/zeromq/zmq_connection_handler.hpp>

#include <boost/interprocess/streams/bufferstream.hpp>

#include <alcommon-ng/serialization/definition_type.hpp>

#include <zmq.hpp>

#include <alcommon-ng/serialization/iarchive.hpp>
#include <alcommon-ng/serialization/oarchive.hpp>

#include <pthread.h>
#include <allog/allog.h>
#include <boost/thread/mutex.hpp>
#include <alfile/alfilesystem.h>




namespace AL {
  namespace Messaging {



    //If you want to use a zmqqueue device and a fixed number of workers threads define ZMQ_WORKERS_THREADS
    //else you get custom XREP code (that could be used with a thread pool)
    //#define ZMQ_WORKERS_THREADS
    static const char *gWorkersAddress      = "inproc://workers";
    static const int   gWorkersThreadsCount = 10;

    //if you use the custom XREP code, activate the full async experience to use the thread pool
    #define ZMQ_FULL_ASYNC


    void marshallResult(AL::ALPtr<ResultDefinition> result, zmq::message_t &msg)
    {
      std::stringstream  outstream;
      OArchive           oarchive(outstream);

      oarchive << (*result);

      //copy the message content
      msg.rebuild(outstream.str().size());
      memcpy(msg.data(), outstream.str().data(), outstream.str().size());
    }

    void marshallCall(AL::ALPtr<CallDefinition> def, zmq::message_t &msg)
    {
      std::stringstream  outstream;
      OArchive           oarchive(outstream);

      oarchive << (*def);

      //copy the message content
      msg.rebuild(outstream.str().size());
      memcpy(msg.data(), outstream.str().data(), outstream.str().size());
    }

    AL::ALPtr<CallDefinition> unmarshallCall(zmq::message_t &msg)
    {
      AL::ALPtr<CallDefinition>         def(new CallDefinition());
      boost::interprocess::bufferstream bstream((char *)msg.data(), msg.size());
      IArchive                          archive(bstream);

      archive >> *def;
      return def;
    }


    ZMQServer::ZMQServer (const std::string &server_name)
      : ServerBase(server_name),
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

    ZMQServer::~ZMQServer () {
    }

    void ZMQServer::wait () {
    }

    void ZMQServer::stop () {
    }

    void ZMQServer::poll() {
      int             rc = 0;
      zmq_pollitem_t  items[1];

      items[0].socket  = zsocket;
      items[0].fd      = 0;
      items[0].events  = ZMQ_POLLIN;
      items[0].revents = 0;

      rc = zmq_poll(&items[0], 1, -1);
      assert(rc > 0);
      assert(items[0].revents & ZMQ_POLLIN);
    }

    //receive the message in parameter, return the identity
    zmq::message_t *ZMQServer::recv(zmq::message_t &msg) {
      int             rc = 0;
      boost::int64_t  more;
      size_t          moresz   = sizeof(more);
      zmq::message_t *identity = new zmq::message_t();

      alsdebug << "ZMQ: waiting for a message";
      poll();
      boost::mutex::scoped_lock lock(socketMutex);
      {
        rc = zsocket.recv(identity);
        assert(rc > 0);
        zsocket.getsockopt(ZMQ_RCVMORE, &more, &moresz);
        //first msg we want an identity
        assert(more);
        alsdebug << "ZMQ: identity received";

        //TODO mettre une condition de sortie
        while (true)
        {
          rc = zsocket.recv(&msg);
          assert(rc > 0);
          zsocket.getsockopt(ZMQ_RCVMORE, &more, &moresz);
          if (!more)
          {
            alsdebug << "ZMQ: Receive complete, msg size:" << msg.size();
            break;
          }
          alsdebug << "ZMQ: Receive(need more): " << msg.size();
        }
      }
      return identity;
    }




#ifdef ZMQ_WORKERS_THREADS

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
        AL::ALPtr<CallDefinition> def = unmarshallCall(msg);
        ZMQConnectionHandler(def, zserv->getCommandDelegate(), zserv, &s).run();
      }
    }

    void ZMQServer::run() {
      alsdebug << "Start ZMQServer on: " << server_path;
      zsocket.bind(server_path.c_str());

      //zmq::socket_t workers (zctx, ZMQ_XREQ);
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

    void ZMQServer::sendResponse(const CallDefinition &def, AL::ALPtr<ResultDefinition> result, void *data) {
      int                rc = 0;
      zmq::message_t     msg;
      zmq::socket_t     *sock = static_cast<zmq::socket_t *>(data);

      assert(data && result);
      alsdebug << "ZMQ: send response";
      marshallResult(result, msg);
      rc = sock->send(msg);
      assert(rc > 0);
    }

#else
    //use only the number of thread we need
     void ZMQServer::run() {
       alsdebug << "Start ZMQServer on: " << server_path;
       zsocket.bind(server_path.c_str());

       #ifdef ZMQ_FULL_ASYNC
       alsdebug << "ZMQ: entering the loop (XREP + growing thread mode)";
       #else
       alsdebug << "ZMQ: entering the loop (XREP)";
       #endif
       while (true) {
         zmq::message_t  msg;
         zmq::message_t *identity;

         identity = recv(msg);

         AL::ALPtr<CallDefinition>          def = unmarshallCall(msg);
#ifdef ZMQ_FULL_ASYNC
         handlersPool.pushTask(AL::ALPtr<ZMQConnectionHandler>(new ZMQConnectionHandler(def, this->getCommandDelegate(), this, (void *)identity)));
#else
         ZMQConnectionHandler(def, this->getCommandDelegate(), this, (void *)identity).run();
#endif
       }
     }

    void ZMQServer::sendResponse(const CallDefinition &def, AL::ALPtr<ResultDefinition> result, void *data) {
      int                rc = 0;
      zmq::message_t     msg;
      zmq::message_t     emptymsg(0);
      zmq::message_t    *identity = static_cast<zmq::message_t *>(data);

      assert(identity);
      assert(result);

      alsdebug << "ZMQ: send response";
      marshallResult(result, msg);

      boost::mutex::scoped_lock lock(socketMutex);
      {
        //send identity (and an empty msg for xrep to be happy)
        rc = zsocket.send(*identity, ZMQ_SNDMORE);
        assert(rc > 0);
        //delete identity;
        rc = zsocket.send(emptymsg, ZMQ_SNDMORE);
        assert(rc > 0);

        //send the message
        alsdebug << "ZMQ: response size: " << msg.size();
        rc = zsocket.send(msg);
        assert(rc > 0);
      }
    }
#endif

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
          AL::ALPtr<ippc::CallDefinition>   def(new ippc::CallDefinition());
          boost::interprocess::bufferstream bstream((char *)msg.data(), msg.size());
          ippc::IArchive                    archive(bstream);

          //unmarshall the message
          archive >> *def;
          //handlersPool.pushTask(AL::ALPtr<ippc::ZMQConnectionHandler>(new ippc::ZMQConnectionHandler(*def, this->getCommandDelegate(), this)));
          ippc::ZMQConnectionHandler(*def, this->getCommandDelegate(), this).run();
        }
      }
    */


  }
}




