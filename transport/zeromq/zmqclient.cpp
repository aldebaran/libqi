/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Cedric GESTES
*/

#include <alippc/transport/zeromq/zmqclient.hpp>
#include <alippc/transport/shm/client/result_handler.hpp>
#include <alippc/serialization/iarchive.hpp>
#include <alippc/serialization/oarchive.hpp>
#include <sstream>
#include <boost/interprocess/streams/bufferstream.hpp>
#include <alfile/alfilesystem.h>

namespace AL {
  namespace Messaging {

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

  AL::ALPtr<ResultDefinition> ZMQClient::send(CallDefinition &def)
  {
    AL::ALPtr<ResultDefinition> ret(new ResultDefinition());

    //boost::interprocess::bufferstream bstream((char *)msg.data(), msg.size());
    std::stringstream           strstream;
    OArchive                    oarchive(strstream);
    oarchive << def;

    zmq::message_t                    request(strstream.str().size());
    memcpy(request.data(), strstream.str().data(), strstream.str().size());
    socket.send(request);
    std::cout << "ZMQClient::send" << std::endl;

    //  Get the reply.
    zmq::message_t reply;
    socket.recv(&reply);
    std::cout << "ZMQClient::recv" << std::endl;
    boost::interprocess::bufferstream bstream((char *)reply.data(), reply.size());
    IArchive archive(bstream);
    archive >> (*ret);
    return ret;
  }

}
}

