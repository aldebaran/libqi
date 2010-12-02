#pragma once
/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/

#ifndef _QI_TRANSPORT_CLIENT_HPP_
#define _QI_TRANSPORT_CLIENT_HPP_

#include <qi/transport/detail/client_impl.hpp>
#include <qi/transport/detail/zmq/zmq_client_impl.hpp>
#include <qi/log.hpp>

namespace qi {
  namespace transport {

    template<typename TRANSPORT_TYPE, typename BUFFER_TYPE>
    class GenericTransportClient {
    public:
      GenericTransportClient() : isInitialized(false) {}

      bool connect(const std::string &address) {
        try {
          //qisDebug << "* GenericTransportClient:connect " << address << std::endl;
          _client = new TRANSPORT_TYPE(address);
          isInitialized = true;
        } catch(const std::exception& e) {
          qisDebug << "GenericClient failed to create client for address \""
            << address << "\" Reason: " << e.what() << std::endl;
        }
         return isInitialized;
      }

      void send(const BUFFER_TYPE &def, BUFFER_TYPE &result)
      {
        if (!isInitialized) {
          qisError << "Attempt to use an unitialized client." << std::endl;
        }
        _client->send(def, result);
      }

      bool isInitialized;
    protected:
      qi::transport::detail::ClientImpl<BUFFER_TYPE> *_client;
    };

    typedef GenericTransportClient<
      qi::transport::detail::ZMQClientImpl,
      qi::transport::Buffer> ZMQTransportClient;

    typedef ZMQTransportClient Client;
  }
}

#endif  // _QI_TRANSPORT_CLIENT_HPP_
