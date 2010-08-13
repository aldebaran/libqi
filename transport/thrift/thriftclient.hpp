/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Cedric GESTES
*/

#ifndef         AL_MESSAGING_TRANSPORT_THRIFTCLIENT_HPP_
# define        AL_MESSAGING_TRANSPORT_THRIFTCLIENT_HPP_

#include <alcommon-ng/transport/client.hpp>
#include <transport/TTransport.h>

namespace AL {
  namespace Transport {

    class ResultHandler;
    class ThriftClient : public Client {
    public:
      ThriftClient(const std::string &servername, ResultHandler *resultHandler);
      ThriftClient(const std::string &servername);

      virtual void send(const std::string &tosend, std::string &result);

    protected:
      void connect();

    protected:
      apache::thrift::transport::TTransport *transport;
    };
  }
}


#endif
