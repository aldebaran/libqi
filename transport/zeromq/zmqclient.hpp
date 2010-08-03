/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#ifndef   	ALIPPC_TRANSPORT_ZMQCLIENT_HPP_
# define   	ALIPPC_TRANSPORT_ZMQCLIENT_HPP_

# include <alcommon-ng/transport/clientbase.hpp>
# include <alcommon-ng/serialization/call_definition.hpp>
# include <zmq.hpp>

namespace AL {
  namespace Messaging {

  class ResultHandler;
  class ZMQClient : public ClientBase {
  public:
    ZMQClient(const std::string &servername, ResultHandler *resultHandler);
    ZMQClient(const std::string &servername);

    virtual AL::ALPtr<ResultDefinition> send(CallDefinition &def);

  protected:
    void connect();

  protected:
    zmq::context_t context;
    zmq::socket_t  socket;
  };
}
}

#endif	    /* !ALIPPC_TRANSPORT_ZMQCLIENT_HPP_ */
