#pragma once
/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/


#ifndef _QI_MESSAGING_SRC_MASTER_IMPL_HPP_
#define _QI_MESSAGING_SRC_MASTER_IMPL_HPP_

#include <qi/messaging/src/server_impl.hpp>
#include <qi/messaging/src/mutexednamelookup.hpp>
#include <qi/messaging/src/address_manager.hpp>
#include <qi/messaging/src/topic.hpp>
#include <qi/functors/makefunctor.hpp>
#include <qi/signature.hpp>

namespace qi {
  namespace detail {
    class MasterImpl {
    public:
      MasterImpl(const std::string& masterAddress, Context *ctx = NULL);

      ~MasterImpl();

      void run();

      /// <summary>Registers the service. </summary>
      /// <param name="methodSignature">The method signature.</param>
      /// <param name="serverID">Identifier for the server.</param>
      void registerService(const std::string& methodSignature,
                           const std::string& serverID);

      /// <summary>Unregisters the service described by methodSignature. </summary>
      /// <param name="methodSignature">The method signature.</param>
      void unregisterService(const std::string& methodSignature);

      /// <summary>Registers the machine. </summary>
      /// <param name="hostName">Name of the host.</param>
      /// <param name="machineID">Identifier for the machine.</param>
      /// <param name="publicIPAddress">The public ip address.</param>
      /// <param name="platformID">Identifier for the platform.</param>
      void registerMachine(const std::string& hostName,
                           const std::string& machineID,
                           const std::string& publicIPAddress,
                           const int&         platformID);

      /// <summary>Registers the endpoint. </summary>
      /// <param name="type">The type of the endpoint</param>
      /// <param name="name">The name of the endpoint</param>
      /// <param name="endpointID">Identifier for the endpoint.</param>
      /// <param name="contextID">Identifier for the context.</param>
      /// <param name="machineID">Identifier for the machine.</param>
      /// <param name="processID">Identifier for the process.</param>
      /// <param name="port">The port.</param>
      void registerEndpoint(
        const int& type, const std::string& name,
        const std::string& endpointID, const std::string& contextID,
        const std::string& machineID, const int& processID, const int& port);

      /// <summary>Unregisters the endpoint described by endpointID. </summary>
      /// <param name="endpointID">Identifier for the endpoint.</param>
      void unregisterEndpoint(const std::string& endpointID);

      /// <summary>Locates a service. </summary>
      /// <param name="methodSignature">The method signature.</param>
      /// <param name="clientID">Identifier for the client.</param>
      /// <returns>The fully qualified endpoint for the service.</returns>
      std::string locateService(const std::string& methodSignature, const std::string& clientID);

      const std::map<std::string, std::string>& listServices();
      std::vector<std::string> listTopics();
      std::map<std::string, std::string> getTopic(const std::string& topicID);
      const std::vector<std::string> listMachines();
      const std::vector<std::string> listEndpoints();
      const std::map<std::string, std::string> getMachine(const std::string& machineID);
      const std::map<std::string, std::string> getEndpoint(const std::string& endpointID);


      void registerTopicParticipant(const std::string& topicName,
        const std::string& endpointID);

      /// <summary>Locates a topic. </summary>
      /// <param name="methodSignature">The method signature.</param>
      /// <param name="clientID">Identifier for the client.</param>
      /// <returns>The fully qualified endpoint for the service.</returns>
      std::string locateTopic(const std::string& methodSignature, const std::string& clientID);

      /// <summary>Registers the topic. </summary>
      /// <param name="topicName">Name of the topic.</param>
      /// <param name="endpointID">Identifier for the endpoint.</param>
      void registerTopic(const std::string& topicName, const std::string& endpointID);

      /// <summary>Unregisters the topic described by topicName. </summary>
      /// <param name="topicName">Name of the topic.</param>
      void unregisterTopic(const std::string& topicName);

      /// <summary>Queries if a given topic exists. </summary>
      /// <param name="topicName">Name of the topic.</param>
      /// <returns>true if it exists, false otherwise.</returns>
      bool topicExists(const std::string& topicName);

      bool isInitialized() const;

    private:
      std::string _address;

      /// <summary> The Master's server that is used by clients to call methods</summary>
      ServerImpl  _server;

      void xRegisterEndpoint(const EndpointContext& endpoint);
      void xRegisterMachine(const MachineContext& machine);
      std::string xNegotiateEndpoint(const std::string& clientEndpointID, const std::string& serverEndpointID);
      std::vector<std::string> xListServicesForEndpoint(const std::string& endpointID);
      std::vector<std::string> xListTopicsForEndpoint(const std::string& endpointID);

      // Helper method
      template <typename OBJECT_TYPE, typename METHOD_TYPE>
      void xAddMasterMethod(
        const std::string& endpointID,
        const std::string& methodName,
        OBJECT_TYPE obj,
        METHOD_TYPE method)
      {
        std::string signature = makeFunctionSignature(methodName, method);
        _server.advertiseService(signature, makeFunctor(obj, method));
        registerService(signature, endpointID);
      }

      /// <summary> A map from service signatures to endpointIDs </summary>
      MutexedNameLookup<std::string> _knownServices;

      /// <summary> A map from machineIDs to their complete contexts </summary>
      MutexedNameLookup<qi::detail::MachineContext> _knownMachines;

      /// <summary> A map from endpointIDs to their complete contexts </summary>
      MutexedNameLookup<qi::detail::EndpointContext> _knownEndpoints;

      /// <summary> A map from topic signatures to their Topic structure </summary>
      MutexedNameLookup<qi::detail::Topic>           _knownTopics;

      AddressManager _addressManager;

      // Helpful typedefs
      typedef std::map<std::string, MachineContext>                  MachineMap;
      typedef std::map<std::string, MachineContext>::const_iterator  MachineMapCIT;
      typedef std::map<std::string, EndpointContext>                 EndpointMap;
      typedef std::map<std::string, EndpointContext>::const_iterator EndpointMapCIT;
      typedef std::map<std::string, Topic>                           TopicMap;
      typedef std::map<std::string, Topic>::const_iterator           TopicMapCIT;
    };
  }
}

#endif  // _QI_MESSAGING_SRC_MASTER_IMPL_HPP_

