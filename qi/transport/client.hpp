/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#ifndef QI_TRANSPORT_CLIENT_HPP_
# define QI_TRANSPORT_CLIENT_HPP_

#include <qi/log.hpp>
#include <qi/transport/buffer.hpp>
#include <qi/transport/detail/clientimpl.hpp>
#include <qi/transport/zeromq/zmqclientimpl.hpp>

namespace qi {
  namespace transport {

    template<typename Transport, typename Buffer>
    class _Client {
    public:
      _Client()
        : initOK(false) {
      }

      bool connect(const std::string &address) {
        try {
          _client = new Transport(address);
          initOK = true;
        } catch(const std::exception& e) {
          qisDebug << "GenericClient failed to create client for address \"" << address << "\" Reason: " << e.what() << std::endl;
        }
         return initOK;
      }

      void send(const Buffer &def, Buffer &result)
      {
        if (!initOK) {
          qisError << "Attempt to use an unitialized client." << std::endl;
        }
        _client->send(def, result);
      }

      bool initOK;
    protected:
      qi::transport::detail::ClientImpl<Buffer> *_client;
    };

    typedef _Client<qi::transport::detail::ZMQClientImpl, qi::transport::Buffer> ZMQClient;
    typedef ZMQClient Client;

  }
}

#endif  // QI_TRANSPORT_CLIENT_HPP_
