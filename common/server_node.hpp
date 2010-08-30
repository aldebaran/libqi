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
      AL::Common::ClientNode,
      AL::Messaging::DefaultMessageHandler,
      public AL::Messaging::DefaultServer {
    public:
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
      // should become functors
      MutexedNameLookup<ServiceInfo> fLocalServiceList;
    };
  }
}

#endif  // COMMON_SERVER_NODE_HPP_

