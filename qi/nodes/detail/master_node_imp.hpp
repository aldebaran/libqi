#pragma once
/*
** Author(s):
**  - Chris Kilner <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/
#ifndef QI_NODE_MASTER_NODE_IMP_HPP_
#define QI_NODE_MASTER_NODE_IMP_HPP_

#include <string>
#include <qi/nodes/server_node.hpp>  // could use imp
#include <qi/nodes/detail/mutexednamelookup.hpp>

namespace qi {
  namespace detail {

    class MasterNodeImp {
    public:
      explicit MasterNodeImp(const std::string& masterAddress);

      ~MasterNodeImp();

      void registerService(const std::string& nodeAddress,
        const std::string& methodSignature);

      void registerServerNode(const std::string& nodeName, const std::string& nodeAddress);
      void unregisterServerNode(const std::string& nodeName, const std::string& nodeAddress);

      void registerClientNode(const std::string& nodeName, const std::string& nodeAddress);
      void unregisterClientNode(const std::string& nodeName, const std::string& nodeAddress);

      const std::string locateService(const std::string& methodSignature);

      const std::map<std::string, std::string>& listServices();

    private:
      std::string fName;
      std::string fAddress;
      ServerNode fServerNode;

      void xInit();

      // map from methodSignature to nodeAddress
      MutexedNameLookup<std::string> fServiceCache;

      // Auditing: map from nodeName to nodeAddress
      MutexedNameLookup<std::string> fServerNodeCache;

      // Auditing: map from nodeName to nodeAddress
      MutexedNameLookup<std::string> fClientNodeCache;
    };
  }
}

#endif  // QI_NODE_MASTER_NODE_IMP_HPP_

