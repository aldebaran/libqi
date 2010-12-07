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
      explicit MasterImpl(const std::string& masterAddress);

      ~MasterImpl();

      void registerService(const std::string& methodSignature,
                           const std::string& serverID);
      void unregisterService(const std::string& methodSignature);

      void registerMachine(const std::string& hostName,
                           const std::string& machineID,
                           const std::string& publicIPAddress,
                           const int&         platformID);

      void registerEndpoint(
        const int& type, const std::string& name,
        const std::string& endpointID, const std::string& contextID,
        const std::string& machineID, const int& processID, const int& port);

      void unregisterEndpoint(const std::string& id);

      std::string locateService(const std::string& methodSignature, const std::string& clientID);

      const std::map<std::string, std::string>& listServices();
      std::vector<std::string> listTopics();
      std::map<std::string, std::string> getTopic(const std::string& topicID);
      const std::vector<std::string> listMachines();
      const std::vector<std::string> listEndpoints();
      const std::map<std::string, std::string> getMachine(const std::string& machineID);
      const std::map<std::string, std::string> getEndpoint(const std::string& endpointID);

      std::string locateTopic(const std::string& methodSignature, const std::string& clientID);

      void registerTopic(const std::string& topicName, const std::string& endpointID);
      void unregisterTopic(const std::string& topicName);

      bool topicExists(const std::string& topicName);

      bool isInitialized() const;

    private:
      std::string _address;
      ServerImpl  _server;

      void xInit();
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
        std::string signature = makeSignature(methodName, method);
        _server.addService(signature, makeFunctor(obj, method));
        registerService(signature, endpointID);
      }

      /// map from methodSignature to endpointID
      MutexedNameLookup<std::string> _knownServices;

      /// map from machine to MachineContext
      MutexedNameLookup<qi::detail::MachineContext> _knownMachines;

      /// map from endpointID to EndpointContext
      MutexedNameLookup<qi::detail::EndpointContext> _knownEndpoints;

      /// map from topicSignature to topic
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

