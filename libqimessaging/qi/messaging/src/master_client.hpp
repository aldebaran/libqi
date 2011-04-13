#pragma once
/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/


#ifndef _QI_MESSAGING_SRC_MASTER_CLIENT_HPP_
#define _QI_MESSAGING_SRC_MASTER_CLIENT_HPP_

#include <qi/transport/transport_client.hpp>
#include <qi/messaging/src/network/endpoint_context.hpp>
#include <qi/messaging/src/network/machine_context.hpp>
#include <qi/messaging/context.hpp>

namespace qi {
  namespace detail {
    class MasterClient {
    public:
      MasterClient(qi::Context *ctx = NULL);
      virtual ~MasterClient();

      void connect(const std::string masterAddress);

      const std::string& getMasterAddress() const;

      int getNewPort(const std::string& machineID);

      void registerMachine(const qi::detail::MachineContext& m);

      void registerEndpoint(const qi::detail::EndpointContext& e);

      void unregisterEndpoint(const qi::detail::EndpointContext& e);

      std::string locateService(const std::string& methodSignature,
                                const qi::detail::EndpointContext& e);

      void registerService(const std::string& methodSignature,
                           const qi::detail::EndpointContext& e);

      void unregisterService(const std::string& methodSignature);

      std::string locateTopic(const std::string& methodSignature,
                              const qi::detail::EndpointContext& e);

      bool topicExists(const std::string& topicSignature);

      void registerTopic(const std::string& topicSignature,
                         const bool& isManyToMany,
                         const qi::detail::EndpointContext& e);

      void registerTopicParticipant(const std::string& signature,
        const std::string& endpointID);

      void unregisterTopic(const std::string& topicSignature,
        const qi::detail::EndpointContext& e);

      qi::Context* getQiContextPtr() const {
        return _qiContextPtr;
      }

      bool isInitialized() const;

    protected:
      std::string _masterAddress;

      bool _isInitialized;

      /// <summary> The qi Context</summary>
      qi::Context*                _qiContextPtr;

      /// <summary> The transport client used to talk with the master </summary>
      qi::transport::TransportClient _transportClient;
    };
  }
}

#endif  // _QI_MESSAGING_SRC_MASTER_CLIENT_HPP_

