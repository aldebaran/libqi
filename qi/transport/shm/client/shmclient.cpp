/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#include <alcommon-ng/transport/shm/client/shmclient.hpp>

namespace AL {
  namespace Transport {

  ShmClient::ShmClient(const std::string &servername, ResultHandler *resultHandler)
    : Client(servername),
      _resultHandler(resultHandler)
  {
  }

  void ShmClient::send(const std::string &tosend, std::string &result)
  {
    ShmConnection  connection(_serverAddress, *_resultHandler);
    connection.send(tosend, result);
  }

}
}
