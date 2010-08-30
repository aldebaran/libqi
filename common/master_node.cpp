/*
** Author(s):
**  - Chris Kilner <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#include <alcommon-ng/common/master_node.hpp>

namespace AL {
  namespace Common {

    // TODO create a client only node as a base class
    MasterNode::MasterNode(
      const std::string& masterName,
      const std::string& masterAddress) :
        ServerNode(masterName, masterAddress, masterAddress) {

      // just testing ... should be a bind
      //addLocalService(ServiceInfo(masterName, masterName, "listServices"));
     // addLocalService(ServiceInfo(masterName, masterName, "addService"));
     // addLocalService(ServiceInfo(masterName, masterName, "getService"));

    }
  }
}
