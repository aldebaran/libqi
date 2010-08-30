#pragma once
/*
** Author(s):
**  - Chris Kilner <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/
#ifndef COMMON_CLIENT_NODE_HPP_
#define COMMON_CLIENT_NODE_HPP_

#include <string>
#include <alcommon-ng/messaging/messaging.hpp>
#include <alcommon-ng/common/namelookup.hpp>
#include <alcommon-ng/common/nodeinfo.hpp>
#include <alcommon-ng/common/serviceinfo.hpp>

namespace AL {
  namespace Common {

    // can handle calls to any service known by master
    class ClientNode {
    public:
      ClientNode(const std::string& clientName,
        const std::string& masterAddress);
      virtual ~ClientNode();
      
      // TODO use templated real calls ...
      AL::Messaging::ResultDefinition call(
        const AL::Messaging::CallDefinition& callDef);


    private:
      // TODO Hide implementation
      std::string fClientName;
      std::string fMasterAddress;

      NameLookup<NodeInfo> fServerList;
      NameLookup<ServiceInfo> fServiceCache;
      NameLookup<boost::shared_ptr<AL::Messaging::DefaultClient> > fServerClients;

      void xInit();
      void xUpdateServicesFromMaster();
      void xCreateServerClient(const NodeInfo& serverNodeInfo);

      const ServiceInfo& ClientNode::xGetService(const std::string& methodHash);

    protected:
      ServiceInfo fInvalidService;

    };
  }
}

#endif  // COMMON_CLIENT_NODE_HPP_

