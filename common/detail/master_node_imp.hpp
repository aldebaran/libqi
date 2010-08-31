#pragma once
/*
** Author(s):
**  - Chris Kilner <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/
#ifndef AL_COMMON_MASTER_NODE_IMP_HPP_
#define AL_COMMON_MASTER_NODE_IMP_HPP_

#include <string>
#include <alcommon-ng/common/detail/nodeinfo.hpp>
#include <alcommon-ng/common/server_node.hpp> // could use imp
#include <alcommon-ng/common/detail/mutexednamelookup.hpp>

namespace AL {
  namespace Common {

    class MasterNodeImp {
    public:
      MasterNodeImp(const std::string& masterAddress);

      void registerService(const std::string& nodeAddress, const std::string& methodHash);
      const std::string& locateService(const std::string& methodHash);
      const std::map<std::string, std::string>& listServices();

    private:
      NodeInfo fNodeInfo;
      ServerNode fServerNode;

      void xInit();

      // map from methodHash to nodeAddress
      MutexedNameLookup<std::string> fServiceCache;
    };
  }
}

#endif  // AL_COMMON_MASTER_NODE_IMP_HPP_

