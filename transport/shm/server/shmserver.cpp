/*
** Author(s):
**  - Jean-Charles DELAY <jdelay@aldebaran-robotics.com>
**  - Cedric GESTES      <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#include <alcommon-ng/transport/shm/server/shmserver.hpp>
#include <alcommon-ng/transport/shm/server/connection_handler.hpp>
#include <alcommon-ng/transport/shm/server/result_connection_handler.hpp>
#include <alcommon-ng/transport/shm/client/shmconnection.hpp>

#include <boost/bind.hpp>
#include <boost/interprocess/sync/interprocess_mutex.hpp>

#include <alcommon-ng/serialization/definition_type.hpp>

namespace AL {
  namespace Messaging {

ShmServer::ShmServer (const std::string & server_name)
  : ServerBase(server_name),
    server_running(true),
    connector(server_name) {
  connector.connect();
}

ShmServer::~ShmServer () {
 /* if (thread) {
    server_running = false;
    connector.broadcast();
    thread->join();
    delete thread;
    std::cout << "Server down !" << std::endl;
  }*/
}


ResultHandler *ShmServer::getResultHandler () {
  return &resultHandler;
}
/*
void Server::run () {
  if (thread)
    return;

  //thread = new boost::thread(boost::bind(&Server::_run, this));
  //fThread = AL::ALPtr<AL::ALThread> (new ALThread());
}*/

void ShmServer::wait () {
/*  if (thread)
          thread->join();*/
}

void ShmServer::stop () {
/*  if (thread) {
    server_running = false;
    connector.broadcast();
    thread->join();
    delete thread;
    thread = 0;
  }*/
}

void ShmServer::run () {
  boi::scoped_lock<boi::interprocess_mutex> lock(connector.getMutex<boi::interprocess_mutex>());

  while (1) {

    // Waiting for a connection
    std::string invite = connector.waitForRequest(lock, server_running);
    if (!server_running)
      break;

    // Got a connection ; Notifying the client it can connect the shm
    connector.notifyInit();

    // Waiting for client to write the first wave of data
    connector.resync(lock);

    if (connector.getType() == TypeResult || connector.getType() == TypeException) {
                  // Received a result ; Treating it !
      //std::cout << "receive result: " << invite << std::endl;
      //ResultConnectionHandler *hand  = new ResultConnectionHandler(invite, resultHandler);
      //hand->run();
      handlersPool.pushTask(AL::ALPtr<ResultConnectionHandler> (new ResultConnectionHandler(invite, resultHandler)));
      continue;
    }

    handlersPool.pushTask(AL::ALPtr<ConnectionHandler> (new ConnectionHandler(invite, command_delegate, this)));
  }
  // connector.disconnect done in BoostServerSharedSegmentConnector destructor
}

  void ShmServer::sendResponse(const CallDefinition &def, AL::ALPtr<ResultDefinition> result, void *data)
  {
    data = 0;
    try {
      ShmConnection connection(def.getSender(), *getResultHandler());
      connection.send(*result);
    } catch (const std::exception & e)
    {
      std::cerr << "FATAL: Failed to send back the result for call: " << e.what() << std::endl;
    }
  }

}
}
