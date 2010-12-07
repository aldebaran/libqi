/*
*  Author(s):
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/

#include <qi/messaging/src/master_impl.hpp>
#include <qi/messaging/src/network/endpoints.hpp>
#include <qi/messaging/src/network/platform.hpp>
#include <qi/log.hpp>

#define MASTERIMPL_DEBUG_ENDPOINT_CONTEXT(msg, endpoint)            \
  qisDebug << "===" << msg << " ===" << std::endl;                  \
  qisDebug << "type      : " << endpointTypeAsString(endpoint.type) \
                                                     << std::endl;  \
  qisDebug << "endpointID: " << endpoint.endpointID << std::endl;   \
  qisDebug << "machineID : " << endpoint.machineID  << std::endl;   \
  qisDebug << "name      : " << endpoint.name       << std::endl;   \
  qisDebug << "contextID : " << endpoint.contextID  << std::endl;   \
  qisDebug << "processID : " << endpoint.processID  << std::endl;   \
  qisDebug << "port      : " << endpoint.port       << std::endl;   \
  qisDebug << "============================"        << std::endl;   \

#define MASTERIMPL_DEBUG_MACHINE_CONTEXT(msg, machine)               \
  qisDebug << "===" << msg << " ===" << std::endl;                   \
  qisDebug << "machineID : " << machine.machineID  << std::endl;     \
  qisDebug << "hostName  : " << machine.hostName   << std::endl;     \
  qisDebug << "platformID: " << platformAsString(machine.platformID) \
                                                      << std::endl;  \
  qisDebug << "publicIP  : " << machine.publicIP   << std::endl;     \
  qisDebug << "============================"        << std::endl;    \

namespace qi {
  namespace detail {

    MasterImpl::~MasterImpl() {
      if (!_server.isInitialized()) {
        return;
      }
      const EndpointContext& serverContext = _server.getEndpointContext();
      unregisterEndpoint(serverContext.endpointID);
    }

    MasterImpl::MasterImpl(const std::string& masterAddress) :
        _address(masterAddress),
        _server("master", _address)
    {
      xInit();
    }

    void MasterImpl::xInit() {
      if (!_server.isInitialized()) {
        return;
      }
      // Force an invalid Machine Context in the lookup
      MachineContext m;
      m.machineID = "";
      _knownMachines.setInvalidValue(m);

      // Register machine and server ( without network activity )
      xRegisterMachine(_server.getMachineContext());
      const EndpointContext& serverContext = _server.getEndpointContext();
      xRegisterEndpoint(serverContext);

      // Bind methods
      xAddMasterMethod(serverContext.endpointID, "master.registerService",    this, &MasterImpl::registerService);
      xAddMasterMethod(serverContext.endpointID, "master.locateService",      this, &MasterImpl::locateService);
      xAddMasterMethod(serverContext.endpointID, "master.listServices",       this, &MasterImpl::listServices);
      xAddMasterMethod(serverContext.endpointID, "master.listTopics",         this, &MasterImpl::listTopics);
      xAddMasterMethod(serverContext.endpointID, "master.listMachines",       this, &MasterImpl::listMachines);
      xAddMasterMethod(serverContext.endpointID, "master.listEndpoints",      this, &MasterImpl::listEndpoints);
      xAddMasterMethod(serverContext.endpointID, "master.listMachine",        this, &MasterImpl::listMachine);
      xAddMasterMethod(serverContext.endpointID, "master.listEndpoint",       this, &MasterImpl::listEndpoint);
      xAddMasterMethod(serverContext.endpointID, "master.registerMachine",    this, &MasterImpl::registerMachine);
      xAddMasterMethod(serverContext.endpointID, "master.registerEndpoint",   this, &MasterImpl::registerEndpoint);
      xAddMasterMethod(serverContext.endpointID, "master.unregisterEndpoint", this, &MasterImpl::unregisterEndpoint);
      xAddMasterMethod(serverContext.endpointID, "master.topicExists",        this, &MasterImpl::topicExists);
      xAddMasterMethod(serverContext.endpointID, "master.registerTopic",      this, &MasterImpl::registerTopic);
      xAddMasterMethod(serverContext.endpointID, "master.locateTopic",        this, &MasterImpl::locateTopic);
      xAddMasterMethod(serverContext.endpointID, "master.getNewPort", &_addressManager, &AddressManager::getNewPort);
    }

    bool MasterImpl::isInitialized() const {
      return _server.isInitialized();
    }

    void MasterImpl::registerService(
       const std::string& methodSignature, const std::string& serverID) {

      const EndpointContext& serverContext = _knownEndpoints.get(serverID);
      if(serverContext.name.empty()) {
        qisError << "Master::registerService Attempt to register the "
          "method of an unknown server, Please call registerService first"
          ": signature: " << qi::signatureToString(methodSignature) << " serverID " <<
            serverID << std::endl;
        return;
      }
      qisDebug << "Master::registerService " << serverContext.name << " " << qi::signatureToString(methodSignature) << std::endl;

      _knownServices.insert(methodSignature, serverID);
    }

    void MasterImpl::registerMachine(const std::string& hostName,
      const std::string& machineID, const std::string& publicIPAddress,
      const int& platformID)
    {
      if (!_knownMachines.exists(machineID)) {
        MachineContext m(hostName, machineID, publicIPAddress, (Platform)platformID);
        xRegisterMachine(m);
      }
    }

    void MasterImpl::xRegisterMachine(const MachineContext& machine) {
      MASTERIMPL_DEBUG_MACHINE_CONTEXT("registerMachine", machine);
      _knownMachines.insert(machine.machineID, machine);
    }

    void MasterImpl::registerEndpoint(
      const int& type, const std::string& name,
      const std::string& endpointID, const std::string& contextID,
      const std::string& machineID, const int& processID, const int& port)
    {
      EndpointContext c((EndpointType)type, name, endpointID, contextID, machineID, processID, port);
      xRegisterEndpoint(c);
    }

    void MasterImpl::xRegisterEndpoint(const EndpointContext& endpoint) {
      MASTERIMPL_DEBUG_ENDPOINT_CONTEXT("Master::registerEndpoint", endpoint);

      if ( ! _knownMachines.exists(endpoint.machineID)) {
        qisError << "Master::registerServer: Attempt to register a "
          "server for a machine that has not been registered. machineID: " <<
          endpoint.machineID <<
          " Please call registerMachine first." << std::endl;
      } else {
        _knownEndpoints.insert(endpoint.endpointID, endpoint);
      }
    }

    void MasterImpl::unregisterEndpoint(const std::string& id) {
      const EndpointContext& c = _knownEndpoints.get(id);
      MASTERIMPL_DEBUG_ENDPOINT_CONTEXT("Master::unregisterEndpoint", c);
      // TODO remove associated services
      _knownEndpoints.remove(id);
    }

    std::string MasterImpl::locateService(
      const std::string& methodSignature, const std::string& clientID)
    {
      const std::string& serverID = _knownServices.get(methodSignature);
      if (serverID.empty()) {
        qisDebug << "Master::locateService: Could not find server for method: " << qi::signatureToString(methodSignature) << std::endl;
        return "";
      }
      std::string endpoint = xNegotiateEndpoint(clientID, serverID);
      qisDebug << "Master::locateService: Resolved: " << qi::signatureToString(methodSignature) << " to " << endpoint << std::endl;
      return endpoint;
    }

    std::string MasterImpl::locateTopic(const std::string& topicName, const std::string& endpointID) {
      const std::string& publisherEndpointID = _knownTopics.get(topicName);
      if (publisherEndpointID.empty()) {
        qisDebug << "MasterImpl::locateTopic Could not find topic \"" << topicName << "\"" << std::endl;
        return "";
      }
      std::string endpoint = xNegotiateEndpoint(endpointID, publisherEndpointID);
      qisDebug << "Master::locateTopic: Resolved: " << topicName << " to " << endpoint << std::endl;
      return endpoint;
    }

    std::string MasterImpl::xNegotiateEndpoint(const std::string& clientEndpointID, const std::string& serverEndpointID) {
      const EndpointContext& serverContext = _knownEndpoints.get(serverEndpointID);
      if (serverContext.name.empty()) {
        qisDebug << "Master::xNegotiateEndpoint: Could not find server for serverID: " << serverEndpointID << std::endl;
        return "";
      }
      const EndpointContext& clientContext = _knownEndpoints.get(clientEndpointID);
      if (clientContext.name.empty()) {
        qisDebug << "Master::xNegotiateEndpoint: Could not find client for clientID: " << clientEndpointID << std::endl;
        return "";
      }
      const MachineContext& serverMachineContext = _knownMachines.get(serverContext.machineID);
      if (serverMachineContext.machineID.empty()) {
        qisDebug << "Master::xNegotiateEndpoint: Could not find machine for serverID: " << serverEndpointID << std::endl;
        return "";
      }
      std::string endpoint = negotiateEndpoint(clientContext, serverContext, serverMachineContext);
      return endpoint;
    }


    void MasterImpl::registerTopic(const std::string& topicName, const std::string& endpointID) {
      //FIXME: check for existence
      qisDebug << "Register Topic " << topicName << " " << endpointID << std::endl;
      _knownTopics.insert(topicName, endpointID);
    }

    bool MasterImpl::topicExists(const std::string& topicName) {
      return _knownTopics.exists(topicName);
    }

    const std::map<std::string, std::string>& MasterImpl::listServices() {
      return _knownServices.getMap();
    }

    const std::map<std::string, std::string>& MasterImpl::listTopics() {
      return _knownTopics.getMap();
    }

    const std::vector<std::string> MasterImpl::listMachines() {
      std::vector<std::string> v;
      const MachineMap& m = _knownMachines.getMap();
      MachineMapCIT it    = m.begin();
      MachineMapCIT end   = m.end();
      v.resize(m.size());
      for (int i=0; it != end; ++it) {
        v[i++] = it->first;
      }
      return v;
    }

    const std::vector<std::string> MasterImpl::listEndpoints() {
      std::vector<std::string> v;
      const EndpointMap& e = _knownEndpoints.getMap();
      EndpointMapCIT it    = e.begin();
      EndpointMapCIT end   = e.end();
      v.resize(e.size());
        for (int i=0; it != end; ++it) {
          v[i++] = it->first;
        }
        return v;
    }

    const std::map<std::string, std::string> MasterImpl::listMachine(const std::string& machineID) {
      const MachineContext& m = _knownMachines.get(machineID);
      std::map<std::string, std::string> ret;
      ret.insert(std::make_pair("machineID", m.machineID));
      ret.insert(std::make_pair("hostName", m.hostName));
      ret.insert(std::make_pair("publicIP", m.publicIP));
      ret.insert(std::make_pair("platform", platformAsString((Platform)m.platformID)));
      return ret;
    }

    const std::map<std::string, std::string> MasterImpl::listEndpoint(const std::string& endpointID) {
      const EndpointContext& e = _knownEndpoints.get(endpointID);
      std::map<std::string, std::string> ret;
      ret.insert(std::make_pair("endpointID", e.endpointID));
      ret.insert(std::make_pair("contextID", e.contextID));
      ret.insert(std::make_pair("machineID", e.machineID));
      ret.insert(std::make_pair("processID", "FIXME: ... master_impl.cpp"));
      ret.insert(std::make_pair("name", e.name));
      ret.insert(std::make_pair("type", endpointTypeAsString(e.type)));
      ret.insert(std::make_pair("port", "FIXME..master_impl.cpp line 258!"));
      return ret;
    }

  }
}
