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

#include <alcommon-ng/transport/shm/definition_type.hpp>

namespace AL {
  namespace Transport {

    ShmServer::ShmServer(const std::string & server_name)
      : Server(server_name),
        server_running(true),
        connector(server_name) {
      connector.connect();
    }

    ShmServer::~ShmServer () {
    }

    ResultHandler *ShmServer::getResultHandler () {
      return &resultHandler;
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
          handlersPool.pushTask(AL::ALPtr<ResultConnectionHandler> (new ResultConnectionHandler(invite, resultHandler)));
          continue;
        }

        handlersPool.pushTask(AL::ALPtr<ConnectionHandler> (new ConnectionHandler(invite, _dataHandler, resultHandler)));
      }
      // connector.disconnect done in BoostServerSharedSegmentConnector destructor
    }

  }
}
