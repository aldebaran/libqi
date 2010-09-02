/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#ifndef   	ALIPPC_TRANSPORT_SHM_CLIENT_HPP_
# define   	ALIPPC_TRANSPORT_SHM_CLIENT_HPP_

# include <alcommon-ng/transport/client.hpp>
# include <alcommon-ng/transport/shm/server/shmserver.hpp>
# include <alcommon-ng/transport/shm/client/result_handler.hpp>
# include <alcommon-ng/transport/shm/client/shmconnection.hpp>

namespace AL {
  namespace Transport {

  class ShmClient : public Client {
  public:
    ShmClient(const std::string &servername, ResultHandler *resultHandler);

    virtual void send(const std::string &tosend, std::string &result);

  protected:
    ShmConnection  connection;
  };
}
}


#endif	    /* !ALIPPC_TRANSPORT_SHM_CLIENT_HPP_ */
