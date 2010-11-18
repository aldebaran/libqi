#pragma once
/*
** Author(s):
**  - Chris Kilner <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/
#ifndef   __QI_MESSAGING_DETAIL_MASTER_IMPL_HPP__
#define   __QI_MESSAGING_DETAIL_MASTER_IMPL_HPP__

#include <string>
#include <qi/messaging/server.hpp>  // could use imp
#include <qi/messaging/detail/mutexednamelookup.hpp>

namespace qi {
  namespace detail {

    class MasterImpl {
    public:
      explicit MasterImpl(const std::string& masterAddress);

      ~MasterImpl();

      void registerService(const std::string& nodeAddress,
        const std::string& methodSignature);

      void registerServerNode(const std::string& nodeName, const std::string& nodeAddress);
      void unregisterServerNode(const std::string& nodeName, const std::string& nodeAddress);

      void registerClientNode(const std::string& nodeName, const std::string& nodeAddress);
      void unregisterClientNode(const std::string& nodeName, const std::string& nodeAddress);

      const std::string locateService(const std::string& methodSignature);

      const std::map<std::string, std::string>& listServices();

    private:
      std::string _name;
      std::string _address;
      Server      _server;

      void xInit();

      // map from methodSignature to nodeAddress
      MutexedNameLookup<std::string> _knownServices;

      // Auditing: map from nodeName to nodeAddress
      MutexedNameLookup<std::string> _knownServers;

      // Auditing: map from nodeName to nodeAddress
      MutexedNameLookup<std::string> _knownClients;
    };
  }
}

#endif // __QI_MESSAGING_DETAIL_MASTER_IMPL_HPP__

