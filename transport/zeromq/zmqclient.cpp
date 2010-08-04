/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Cedric GESTES
*/

#include <alcommon-ng/transport/zeromq/zmqclient.hpp>
#include <alcommon-ng/transport/shm/client/result_handler.hpp>
#include <alcommon-ng/serialization/iarchive.hpp>
#include <alcommon-ng/serialization/oarchive.hpp>
#include <sstream>
#include <boost/interprocess/streams/bufferstream.hpp>
#include <alfile/alfilesystem.h>

namespace AL {
  namespace Transport {

    /** Accepted address:
     *   - ipc:///tmp/naoqi/paf
     *   - tcp://127.0.0.1:5555
     */

  ZMQClient::ZMQClient(const std::string &servername)
    : Client(servername),
      context(1),
      socket(context, ZMQ_REQ)
  {
    connect();
  }

  void ZMQClient::connect()
  {
    socket.connect(_serverAddress.c_str());
  }

  void ZMQClient::send(const std::string &tosend, std::string &result)
  {
    //TODO: could we avoid more copy?
    zmq::message_t msg(tosend.size());
    //TODO?
    memcpy(msg.data(), tosend.data(), tosend.size());
    socket.send(msg);
    socket.recv(&msg);
    result.assign((char *)msg.data(), msg.size());
  }

}
}

