/*
** Author(s):
**  - Chris Kilner <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#include <alcommon-ng/common/detail/master_node_imp.hpp>
#include <map>
#include <string>
#include <alcommon-ng/functor/makefunctor.hpp>
#include <allog/allog.h>

namespace AL {
  namespace Common {

    MasterNodeImp::MasterNodeImp(
      const std::string& masterAddress) :
    fNodeInfo("master", masterAddress),
    fServerNode("master", masterAddress, masterAddress) {
      xInit();
    }

    void MasterNodeImp::xInit() {
      fServerNode.addLocalService(ServiceInfo(fNodeInfo.name, fNodeInfo.name, "registerService", makeFunctor(this, &MasterNodeImp::registerService)));
      registerService(fNodeInfo.address, "master.registerService");
      fServerNode.addLocalService(ServiceInfo(fNodeInfo.name, fNodeInfo.name, "locateService", makeFunctor(this, &MasterNodeImp::locateService)));
      registerService(fNodeInfo.address, "master.locateService");
      fServerNode.addLocalService(ServiceInfo(fNodeInfo.name, fNodeInfo.name, "listServices", makeFunctor(this, &MasterNodeImp::listServices)));
      registerService(fNodeInfo.address, "master.listServices");
    }

    void MasterNodeImp::registerService(
      const std::string& nodeAddress, const std::string& methodHash) {
      fServiceCache.insert(methodHash, nodeAddress);
    }

    const std::string& MasterNodeImp::locateService(const std::string& methodHash) {
      return fServiceCache.get(methodHash);
    }

    const std::map<std::string, std::string>& MasterNodeImp::listServices() {
      return fServiceCache.getMap();
    }
  }
}
