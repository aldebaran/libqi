/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#include <iostream>
#include <qi/transport/thrift/thriftclient.hpp>
#include <transport/TSocket.h>
#include <transport/TBufferTransports.h>

namespace qi {
  namespace transport {

    ThriftClient::ThriftClient(const std::string &servername, ResultHandler *resultHandler)
      : Client(servername)
    {
      transport = new apache::thrift::transport::TSocket("127.0.0.1", 5555);
    }

    ThriftClient::ThriftClient(const std::string &servername)
      : Client(servername)
    {

    }

    void ThriftClient::send(const std::string &tosend, std::string &result)
    {
      transport->read((uint8_t*)tosend.c_str(), tosend.size());
      //transport->read();
      //transport->write();
    }
  }
}
//shared_ptr<TTransport> socket(new TSocket("localhost", 9090));
//shared_ptr<TTransport> transport(new TBufferedTransport(socket));
