#pragma once
/*
** Author(s):
**  - Chris Kilner <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/
#ifndef   __QI_MESSAGING_DETAIL_MASTER_IMPL_HPP__
#define   __QI_MESSAGING_DETAIL_MASTER_IMPL_HPP__

#include <string>
#include <qi/messaging/detail/server_impl.hpp>
#include <qi/messaging/detail/mutexednamelookup.hpp>
#include <qi/messaging/detail/address_manager.hpp>
#include <qi/messaging/context.hpp>
#include <qi/transport/detail/network/endpoint_context.hpp>
#include <qi/transport/detail/network/machine_context.hpp>

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

      void registerServer(const std::string& name,
                          const std::string& endpointID,
                          const std::string& contextID,
                          const std::string& machineID,
                          const int& port);

      void unregisterServer(const std::string& id);

      void registerClient(const std::string& name,
                          const std::string& clientID,
                          const std::string& contextID,
                          const std::string& machineID);

      void unregisterClient(const std::string& id);

      std::string locateService(const std::string& methodSignature, const std::string& clientID);

      const std::map<std::string, std::string>& listServices();

      bool isInitialized() const;

    private:
      std::string _name;
      std::string _address;
      ServerImpl  _server;

      void xInit();
      void xRegisterServer(const EndpointContext& endpoint);
      void xRegisterMachine(const MachineContext& machine);

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

      // map from methodSignature to nodeAddress
      MutexedNameLookup<std::string> _knownServices;

      // map from id to EndpointContext
      MutexedNameLookup<qi::detail::EndpointContext> _knownServers;

      // map from id to EndpointContext
      MutexedNameLookup<qi::detail::EndpointContext> _knownClients;

      // map from id to MachineContext
      MutexedNameLookup<qi::detail::MachineContext> _knownMachines;

      AddressManager _addressManager;
    };
  }
}

#endif // __QI_MESSAGING_DETAIL_MASTER_IMPL_HPP__

