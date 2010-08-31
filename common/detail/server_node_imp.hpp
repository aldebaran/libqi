#pragma once
/*
** Author(s):
**  - Chris Kilner <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/
#ifndef COMMON_SERVER_NODE_IMP_HPP_
#define COMMON_SERVER_NODE_IMP_HPP_

#include <string>

// TODO(chris) should be just a client, not a node
#include <alcommon-ng/common/client_node.hpp>

#include <alcommon-ng/messaging/messagehandler.hpp>
#include <alcommon-ng/messaging/server.hpp>
#include <alcommon-ng/common/detail/nodeinfo.hpp>
#include <alcommon-ng/common/serviceinfo.hpp>
#include <alcommon-ng/common/detail/mutexednamelookup.hpp>

namespace AL {
  namespace Common {

    class ServerNodeImp :
      AL::Messaging::MessageHandler,
      public AL::Messaging::Server {
    public:
      ServerNodeImp();
      ServerNodeImp(const std::string& nodeName,
        const std::string& nodeAddress,
        const std::string& masterAddress);

      boost::shared_ptr<AL::Messaging::ResultDefinition> onMessage(
        const AL::Messaging::CallDefinition& def);

      const NodeInfo& getNodeInfo() const;

      // best if we didn't use a ServiceInfo here ...
      // should hide some of bind
      void addLocalService(const ServiceInfo& service);
      
    private:
      NodeInfo fInfo;

      // in fact we only need a single messaging client for talking
      // with the master
      // TODO(chris) replace with messaging client.
      ClientNode fClientNode;

      // should be map from hash to functor,
      // but we also need to be able to push these hashes to master
      // and ...
      // if would be good if we were capable of describing a mehtod
      MutexedNameLookup<ServiceInfo> fLocalServiceList;

      const ServiceInfo& xGetLocalService(const std::string& name);
      void xRegisterServiceWithMaster(const std::string& methodHash);
    };
  }
}

#endif  // COMMON_SERVER_NODE_IMP_HPP_

