/*
** Author(s):
**  - Chris Kilner <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#include <alcommon-ng/common/master_node.hpp>
#include <alcommon-ng/functor/makefunctor.hpp>
#include <allog/allog.h>

namespace AL {
  namespace Common {

    MasterNode::MasterNode(
      const std::string& masterAddress) :
    fNodeInfo("master", masterAddress),
    fServerNode("master", masterAddress, masterAddress) {
      xInit();
    }

    void MasterNode::xInit() {
      fServerNode.addLocalService(ServiceInfo(fNodeInfo.name, fNodeInfo.name, "registerService", makeFunctor(this, &MasterNode::registerService)));
      registerService(fNodeInfo.address, "master.registerService");
      fServerNode.addLocalService(ServiceInfo(fNodeInfo.name, fNodeInfo.name, "locateService", makeFunctor(this, &MasterNode::locateService)));
      registerService(fNodeInfo.address, "master.locateService");
      fServerNode.addLocalService(ServiceInfo(fNodeInfo.name, fNodeInfo.name, "listServices", makeFunctor(this, &MasterNode::listServices)));
      registerService(fNodeInfo.address, "master.listServices");
    }

    void MasterNode::registerService(const std::string& nodeAddress, const std::string& methodHash) {
      fServiceCache.insert(methodHash, nodeAddress);
    }

    const std::string& MasterNode::locateService(const std::string& methodHash) {
      return fServiceCache.get(methodHash);
    }

    const std::map<std::string, std::string>& MasterNode::listServices() {
      return fServiceCache.getMap();
    }
  }
}
