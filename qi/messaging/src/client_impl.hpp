#pragma once
/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/


#ifndef _QI_MESSAGING_DETAIL_CLIENT_IMPL_HPP_
#define _QI_MESSAGING_DETAIL_CLIENT_IMPL_HPP_

#include <string>
#include <boost/shared_ptr.hpp>
#include <qi/messaging/src/mutexednamelookup.hpp>
#include <qi/messaging/src/master_client.hpp>
#include <qi/serialization/message.hpp>
#include <qi/transport/transport_client.hpp>

namespace qi {
  namespace detail {

    class ClientImpl : public MasterClient {
    public:
      ClientImpl();
      virtual ~ClientImpl();

      ClientImpl(const std::string& clientName, const std::string& masterAddress);

      void call(const std::string &signature, const qi::serialization::Message& callDef,
        qi::serialization::Message &result);

    private:
      MutexedNameLookup<std::string> _serviceCache;

      // map from address to Client
      MutexedNameLookup< boost::shared_ptr<qi::transport::TransportClient> > _serverClients;

      boost::shared_ptr<qi::transport::TransportClient> xGetServerClient(
        const std::string& serverAddress);
      bool xCreateServerClient(const std::string& address);
      const std::string& xLocateService(const std::string& methodHash);
    };

  }
}

#endif  // _QI_MESSAGING_DETAIL_CLIENT_IMPL_HPP_
