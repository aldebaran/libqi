/*
** Author(s):
**  - Chris Kilner <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#include <alcommon-ng/common/master_node.hpp>
#include <alcommon-ng/functor/makefunctor.hpp>

namespace AL {
  namespace Common {

    MasterNode::MasterNode(
      const std::string& masterName,
      const std::string& masterAddress) :
      fServerNode(masterName, masterAddress, masterAddress) {

      fServerNode.addLocalService(ServiceInfo(masterName, masterName, "registerService", makeFunctor(this, &MasterNode::registerService)));
      registerService(masterAddress, "master.registerService");
      fServerNode.addLocalService(ServiceInfo(masterName, masterName, "locateService", makeFunctor(this, &MasterNode::locateService)));
      registerService(masterAddress, "master.locateService");
      fServerNode.addLocalService(ServiceInfo(masterName, masterName, "listServices", makeFunctor(this, &MasterNode::listServices)));
      registerService(masterAddress, "master.listServices");
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
