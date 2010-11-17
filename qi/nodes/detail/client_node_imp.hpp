#pragma once
/*
** Author(s):
**  - Chris Kilner <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/
#ifndef COMMON_CLIENT_NODE_IMP_HPP_
#define COMMON_CLIENT_NODE_IMP_HPP_

#include <string>
#include <boost/shared_ptr.hpp>
#include <qi/nodes/detail/mutexednamelookup.hpp>
#include <qi/nodes/detail/namelookup.hpp>
#include <qi/serialization/serializeddata.hpp>
#include <qi/transport/client.hpp>

namespace qi {
  namespace detail {

    class ClientNodeImp {
    public:
      ClientNodeImp();
      virtual ~ClientNodeImp();

      ClientNodeImp(const std::string& clientName, const std::string& masterAddress);

      void call(const std::string &signature, const qi::serialization::SerializedData& callDef, qi::serialization::SerializedData &result);

      bool initOK;

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

#endif  // COMMON_CLIENT_NODE_IMP_HPP_
