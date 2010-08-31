#pragma once
/*
** Author(s):
**  - Chris Kilner <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/
#ifndef COMMON_SERVER_NODE_HPP_
#define COMMON_SERVER_NODE_HPP_

#include <string>
#include <alcommon-ng/common/client_node.hpp>
#include <alcommon-ng/messaging/messaging.hpp>

namespace AL {
  namespace Common {

    class ServerNode :
      AL::Messaging::DefaultMessageHandler,
      public AL::Messaging::DefaultServer {
    public:
      ServerNode();
      ServerNode(const std::string& nodeName,
        const std::string& nodeAddress,
        const std::string& masterAddress);

      boost::shared_ptr<AL::Messaging::ResultDefinition> onMessage(const AL::Messaging::CallDefinition& def);

      const NodeInfo& getNodeInfo() const;

      // todo should become bind, though addLocalService is a very clear name
      void addLocalService(const ServiceInfo& service);
      const ServiceInfo& getLocalService(const std::string& name);

    private:
      // TODO Hide implementation
      NodeInfo fInfo;
      ClientNode fClientNode;
      // should be map from hash to functor,
      // but we also need to be able to push these hashes to master
      // and ...
      // if would be good if we were capable of describing a mehtod
      MutexedNameLookup<ServiceInfo> fLocalServiceList;

      void xRegisterServiceWithMaster(const std::string& methodHash);
    };
  }
}

#endif  // COMMON_SERVER_NODE_HPP_

