/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#include <alcommon-ng/transport/shm/client/shmclient.hpp>

namespace AL {
  namespace Messaging {

  ShmClient::ShmClient(const std::string &servername, ResultHandler *resultHandler)
    : ClientBase(servername),
      connection(servername, *resultHandler)
  {
  }

  AL::ALPtr<ResultDefinition> ShmClient::send(CallDefinition &def)
  {
    AL::ALPtr<ResultDefinition>     res(new ResultDefinition());

    connection.send(def, *res);
    return res;
  }

}
}