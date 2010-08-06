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
#include "namelookup.h"
#include "nodeinfo.h"
#include "serviceinfo.h"
#include <alcommon-ng/messaging/messagehandler.hpp>
#include <alcommon-ng/messaging/server.hpp>


namespace AL {

  namespace Messaging {
    class Client;
  }

  namespace Common {

    class Node : AL::Messaging::MessageHandler, public AL::Messaging::Server {
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
      NameLookup<boost::shared_ptr<AL::Messaging::Client> > fNodeClients;

      void xCreateNodeClient(const NodeInfo& node);

      // publisherlist

    };
  }  // namespace Common
}  // namespace AL

#endif  // COMMON_NODE_H_

