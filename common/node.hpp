#pragma once
/*
** Author(s):
**  - Chris Kilner <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/
#ifndef COMMON_NODE_H_
#define COMMON_NODE_H_

#include <string>
#include <alcommon-ng/common/namelookup.hpp>
#include <alcommon-ng/common/nodeinfo.hpp>
#include <alcommon-ng/common/serviceinfo.hpp>
#include <alcommon-ng/messaging/messaging.hpp>

namespace AL {
  namespace Common {

    class Node :
      AL::Messaging::DefaultMessageHandler,
      public AL::Messaging::DefaultServer {
    public:
      Node(const std::string& name, const std::string &address);

      boost::shared_ptr<AL::Messaging::ResultDefinition> onMessage(const AL::Messaging::CallDefinition &def);

      const NodeInfo& getNodeInfo() const;

      void addNode(const NodeInfo& node);
      const NodeInfo& getNode(const std::string& name) const;

      void addService(const ServiceInfo& service);
      const ServiceInfo& getService(const std::string& name) const;

    private:
      NodeInfo fNodeInfo;
      NameLookup<NodeInfo> fNodeList;
      NameLookup<ServiceInfo> fServiceList;
      NameLookup<boost::shared_ptr<AL::Messaging::DefaultClient> > fNodeClients;

      void xCreateNodeClient(const NodeInfo& node);

      // publisherlist

    };
  }  // namespace Common
}  // namespace AL

#endif  // COMMON_NODE_H_

