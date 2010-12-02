#pragma once
/*
** Author(s):
**  - Chris Kilner <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/
#ifndef   __QI_MESSAGING_DETAIL_MASTER_IMPL_HPP__
#define   __QI_MESSAGING_DETAIL_MASTER_IMPL_HPP__

#include <qi/messaging/detail/server_impl.hpp>
#include <qi/messaging/detail/mutexednamelookup.hpp>
#include <qi/messaging/detail/address_manager.hpp>
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

      std::string locateTopic(const std::string& methodSignature, const std::string& clientID);

      void registerTopic(const std::string& topicName, const std::string& endpointID);

      bool topicExists(const std::string& topicName);

      bool isInitialized() const;

    private:
      std::string _address;
      ServerImpl  _server;

      void xInit();
      void xRegisterEndpoint(const EndpointContext& endpoint);
      void xRegisterMachine(const MachineContext& machine);
      std::string xNegotiateEndpoint(const std::string& clientEndpointID, const std::string& serverEndpointID);

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

      /// map from topicSignature to endpointID
      MutexedNameLookup<std::string>                 _knownTopics;

      AddressManager _addressManager;
    };
  }
}

#endif // __QI_MESSAGING_DETAIL_MASTER_IMPL_HPP__

