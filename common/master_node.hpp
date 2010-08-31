#pragma once
/*
** Author(s):
**  - Chris Kilner <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/
#ifndef AL_COMMON_MASTER_NODE_H_
#define AL_COMMON_MASTER_NODE_H_

#include <string>
#include <alcommon-ng/common/nodeinfo.hpp>
#include <alcommon-ng/common/server_node.hpp>
#include <alcommon-ng/common/mutexednamelookup.hpp>

namespace AL {
  namespace Common {

    class MasterNode {
    public:
      MasterNode(const std::string& masterAddress);

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

#endif  // AL_COMMON_MASTER_NODE_H_

