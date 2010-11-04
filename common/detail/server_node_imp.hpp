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

// used to talk to master
#include <alcommon-ng/messaging/client.hpp>
#include <alcommon-ng/messaging/i_message_handler.hpp>
#include <alcommon-ng/messaging/server.hpp>
#include <alcommon-ng/common/serviceinfo.hpp>
#include <alcommon-ng/common/detail/mutexednamelookup.hpp>

namespace AL {
  namespace Common {

    class ServerNodeImp :
      AL::Messaging::IMessageHandler {
    public:
      ServerNodeImp();
      virtual ~ServerNodeImp();

      ServerNodeImp(const std::string nodeName,
        const std::string nodeAddress,
        const std::string masterAddress);

      const std::string& getName() const;

      const std::string& getAddress() const;

      void addService(const std::string& methodSignature, Functor* functor);

      bool initOK;

      // IMessageHandler Implementation -----------------
      void messageHandler(
        const AL::Messaging::CallDefinition& def,
              AL::Messaging::ResultDefinition& result);
      // -----------------------------------------------

    private:
      /// <summary> The friendly name of this server </summary>
      std::string fName;

      /// <summary> The address of the server </summary>
      std::string fAddress;

      /// <summary> The underlying messaging server </summary>
      AL::Messaging::Server fMessagingServer;

      /// <summary> The messaging client used to talk with the master </summary>
      AL::Messaging::Client fMessagingClient;

      /// <summary> true if this server belongs to the master </summary>
      bool fIsMasterServer;

      // should be map from hash to functor,
      // but we also need to be able to push these hashes to master
      // and ...
      // if would be good if we were capable of describing a mehtod
      MutexedNameLookup<ServiceInfo> fLocalServiceList;

      const ServiceInfo& xGetService(const std::string& methodHash);
      void xRegisterServiceWithMaster(const std::string& methodHash);
      void xRegisterSelfWithMaster();
      void xUnregisterSelfWithMaster();


    };
  }
}

#endif  // COMMON_SERVER_NODE_IMP_HPP_

