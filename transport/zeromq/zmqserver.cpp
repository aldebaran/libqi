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
  namespace Transport {

    //if you use the custom XREP code, activate the full async experience to use the thread pool
    #define ZMQ_FULL_ASYNC

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

