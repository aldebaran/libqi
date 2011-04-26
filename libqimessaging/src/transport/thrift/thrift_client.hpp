#pragma once
/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/


#ifndef _QI_TRANSPORT_SRC_THRIFT_THRIFT_CLIENT_HPP_
#define _QI_TRANSPORT_SRC_THRIFT_THRIFT_CLIENT_HPP_

#include <qimessaging/transport/client.hpp>
#include <transport/TTransport.h>

namespace qi {
  namespace transport {

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


#endif  // _QI_TRANSPORT_SRC_THRIFT_THRIFT_CLIENT_HPP_
