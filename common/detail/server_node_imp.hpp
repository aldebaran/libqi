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
#include <alcommon-ng/common/client_node.hpp>  // TODO(chris) just a client
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

      // todo should become bind, though addLocalService is a very clear name
      void addLocalService(const ServiceInfo& service);
      const ServiceInfo& getLocalService(const std::string& name);

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

      void xRegisterServiceWithMaster(const std::string& methodHash);
    };
  }
}

#endif  // COMMON_SERVER_NODE_IMP_HPP_

