/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#ifndef   	ALIPPC_TRANSPORT_SHM_CLIENT_HPP_
# define   	ALIPPC_TRANSPORT_SHM_CLIENT_HPP_

# include <alcommon-ng/transport/clientbase.hpp>
# include <alcommon-ng/transport/shm/server/shmserver.hpp>
# include <alcommon-ng/transport/shm/client/result_handler.hpp>
# include <alcommon-ng/transport/shm/client/shmconnection.hpp>

namespace AL {
  namespace Messaging {

  class ShmClient : public ClientBase {
  public:
    ShmClient(const std::string &servername, ResultHandler *resultHandler);

    virtual AL::ALPtr<ResultDefinition> send(CallDefinition &def);

  protected:
    ShmConnection  connection;
  };
}
}


#endif	    /* !ALIPPC_TRANSPORT_SHM_CLIENT_HPP_ */
