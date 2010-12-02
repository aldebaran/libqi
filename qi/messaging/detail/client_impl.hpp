#pragma once
/*
** Author(s):
**  - Chris Kilner <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/
#ifndef   __QI_MESSAGING_DETAIL_CLIENT_IMPL_HPP__
#define   __QI_MESSAGING_DETAIL_CLIENT_IMPL_HPP__

#include <string>
#include <boost/shared_ptr.hpp>
#include <qi/messaging/detail/mutexednamelookup.hpp>
//#include <qi/messaging/detail/namelookup.hpp>
#include <qi/messaging/detail/master_client.hpp>
#include <qi/serialization/message.hpp>
//#include <qi/transport/subscribe_handler.hpp>
#include <qi/transport/client.hpp>
//#include <qi/functors/functor.hpp>
//#include <qi/messaging/serviceinfo.hpp>
//#include <qi/messaging/detail/subscriber_impl.hpp>

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
      MutexedNameLookup< boost::shared_ptr<qi::transport::Client> > _serverClients;

      boost::shared_ptr<qi::transport::Client> xGetServerClient(const std::string& serverAddress);
      bool xCreateServerClient(const std::string& address);
      const std::string& xLocateService(const std::string& methodHash);
    };

  }
}

#endif // __QI_MESSAGING_DETAIL_CLIENT_IMPL_HPP__
