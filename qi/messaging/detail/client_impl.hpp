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
#include <qi/serialization/serializeddata.hpp>
#include <qi/transport/client.hpp>

namespace qi {
  namespace detail {

    class ClientImpl {
    public:
      ClientImpl();
      virtual ~ClientImpl();

      ClientImpl(const std::string& clientName, const std::string& masterAddress);

      void call(const std::string &signature, const qi::serialization::SerializedData& callDef, qi::serialization::SerializedData &result);

      bool isInitialized;

    private:
      std::string fClientName;
      std::string fMasterAddress;

      MutexedNameLookup<std::string> fServiceCache;

      // map from address to Client
      NameLookup< boost::shared_ptr<qi::transport::Client> > fServerClients;

      void xInit();
      boost::shared_ptr<qi::transport::Client> xGetServerClient(const std::string& serverAddress);
      bool xCreateServerClient(const std::string& address);
      const std::string& xLocateService(const std::string& methodHash);
    };

  }
}

#endif // __QI_MESSAGING_DETAIL_CLIENT_IMPL_HPP__
