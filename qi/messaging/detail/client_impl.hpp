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
#include <qi/messaging/detail/namelookup.hpp>
#include <qi/messaging/context.hpp>
#include <qi/serialization/serializeddata.hpp>
#include <qi/transport/client.hpp>
#include <qi/transport/detail/network/endpoint_context.hpp>

namespace qi {
  namespace detail {

    class ClientImpl {
    public:
      ClientImpl();
      virtual ~ClientImpl();

      ClientImpl(const std::string& clientName, const std::string& masterAddress);

      void call(const std::string &signature, const qi::serialization::SerializedData& callDef, qi::serialization::SerializedData &result);

      bool isInitialized() const;

    private:
      bool                        _isInitialized;
      std::string                 _clientName;
      std::string                 _masterAddress;
      qi::Context                 _qiContext;
      qi::detail::EndpointContext _endpointContext;

      MutexedNameLookup<std::string> _serviceCache;

      // map from address to Client
      NameLookup< boost::shared_ptr<qi::transport::Client> > _serverClients;

      void xInit();
      boost::shared_ptr<qi::transport::Client> xGetServerClient(const std::string& serverAddress);
      bool xCreateServerClient(const std::string& address);
      const std::string& xLocateService(const std::string& methodHash);

      void xRegisterSelfWithMaster();
      void xUnregisterSelfWithMaster();
    };

  }
}

#endif // __QI_MESSAGING_DETAIL_CLIENT_IMPL_HPP__
