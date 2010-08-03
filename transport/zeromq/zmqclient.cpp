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

    //static const std::string gAddress = "ipc:///tmp/naoqi/";
    //static const std::string gAddress = "tcp://127.0.0.1:5555";

  ZMQClient::ZMQClient(const std::string &servername)
    : ClientBase(servername),
      context(1),
      socket(context, ZMQ_REQ)
  {
    connect();
  }



  //TODO: useless constructor for compat with shm ATM (should be removed)
  ZMQClient::ZMQClient(const std::string &servername, ResultHandler *resultHandler)
    : ClientBase(servername),
      context(1),
      socket(context, ZMQ_REQ)
  {
    connect();
  }

  void ZMQClient::connect()
  {
    ALPath mypath = ALFileSystem::getTmpPath();
    mypath /= server_name;
    std::string address;
#ifdef WIN32
    // ck horrid hack so that naoqi excecutes
    address = "tcp://127.0.0.1:5555";
#else
    address = "ipc://" + mypath.string();
#endif
    socket.connect(address.c_str());
  }

  void ZMQClient::send(const std::string &tosend, std::string &result)
  {
    //TODO: could we avoid more copy?
    zmq::message_t msg(tosend.data(), tosend.size());
    //TODO?
    //memcpy(request.data(), strstream.str().data(), strstream.str().size());
    socket.send(msg);
    socket.recv(&msg);
    result.copy(msg.data(), msg.size());
  }

}
}

