/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/

#include <qi/transport/src/zmq/zmq_server_backend.hpp>
#include <qi/transport/src/zmq/zmq_connection_handler.hpp>
#include <boost/interprocess/streams/bufferstream.hpp>

#include <zmq.hpp>
#include <qi/log.hpp>
#include <boost/thread/mutex.hpp>

namespace qi {
  namespace transport {
    namespace detail {
      //if you use the custom XREP code, activate the full async experience to use the thread pool
#define _QI_ZMQ_FULL_ASYNC

      /// <summary> Constructor. </summary>
      /// <param name="serverAddresses"> The server addresses. </param>
      ZMQServerBackend::ZMQServerBackend(const std::vector<std::string> &serverAddresses, zmq::context_t &context)
        : ServerBackend(serverAddresses),
          _zcontext(context),
          _zsocket(_zcontext, ZMQ_XREP)
      {
        int linger = 0;
        _zsocket.setsockopt(ZMQ_LINGER, &linger, sizeof(linger));
      }

      ZMQServerBackend::~ZMQServerBackend () {
      }

      void ZMQServerBackend::wait () {
      }

      void ZMQServerBackend::stop () {
      }

      void ZMQServerBackend::poll() {
        int             rc = 0;
        zmq_pollitem_t  items[1];

        items[0].socket  = _zsocket;
        items[0].fd      = 0;
        items[0].events  = ZMQ_POLLIN;
        items[0].revents = 0;

        rc = zmq_poll(&items[0], 1, -1);
        assert(rc > 0);
        assert(items[0].revents & ZMQ_POLLIN);
      }

      //receive the message in parameter, return the identity
      zmq::message_t *ZMQServerBackend::recv(zmq::message_t &msg) {
        int             rc = 0;
        boost::int64_t  more;
        size_t          moresz   = sizeof(more);
        zmq::message_t *identity = new zmq::message_t();

        // alsdebug << "ZMQ: waiting for a message";
        poll();
        boost::mutex::scoped_lock lock(_socketMutex);
        {
          rc = _zsocket.recv(identity);
          assert(rc > 0);
          _zsocket.getsockopt(ZMQ_RCVMORE, &more, &moresz);
          //first msg we want an identity
          assert(more);
          // alsdebug << "ZMQ: identity received";

          //TODO mettre une condition de sortie
          while (true)
          {
            rc = _zsocket.recv(&msg);
            assert(rc > 0);
            _zsocket.getsockopt(ZMQ_RCVMORE, &more, &moresz);
            if (!more)
            {
              // alsdebug << "ZMQ: Receive complete, msg size:" << msg.size();
              break;
            }
            // alsdebug << "ZMQ: Receive(need more): " << msg.size();
          }
        }
        return identity;
      }


      //use only the number of thread we need
      void ZMQServerBackend::run() {
        for(unsigned int i=0; i< _serverAddresses.size(); ++i) {
          qisDebug << "Start ZMQServer on: " << _serverAddresses[i];
          _zsocket.bind(_serverAddresses[i].c_str());
        }

#ifdef _QI_ZMQ_FULL_ASYNC
        // alsdebug << "ZMQ: entering the loop (XREP + growing thread mode)";
#else
        alsdebug << "ZMQ: entering the loop (XREP)";
#endif
        while (true) {
          zmq::message_t  msg;
          zmq::message_t *identity;
          identity = recv(msg);
          std::string data;
          data.assign((char *)msg.data(), msg.size());

#ifdef _QI_ZMQ_FULL_ASYNC
          _handlersPool.pushTask(boost::shared_ptr<ZMQConnectionHandler>(new ZMQConnectionHandler(data, this->getMessageHandler(), this, (void *)identity)));
#else
          ZMQConnectionHandler(data, this->getMessageHandler(), this, (void *)identity).run();
#endif
        }
      }

      void ZMQServerBackend::serverResponseHandler(const std::string &result, void *data)
      {
        int                rc = 0;
        zmq::message_t     msg(result.size());
        zmq::message_t     emptymsg(0);
        zmq::message_t    *identity = static_cast<zmq::message_t *>(data);

        memcpy(msg.data(), result.data(), result.size());

        assert(identity);

        // alsdebug << "ZMQ: send response";

        boost::mutex::scoped_lock lock(_socketMutex);
        {
          //send identity (and an empty msg for xrep to be happy)
          rc = _zsocket.send(*identity, ZMQ_SNDMORE);
          assert(rc > 0);
          //delete identity;
          rc = _zsocket.send(emptymsg, ZMQ_SNDMORE);
          assert(rc > 0);

          //send the message
          // alsdebug << "ZMQ: response size: " << msg.size();
          rc = _zsocket.send(msg);
          assert(rc > 0);
        }
      }

    }
  }
}
