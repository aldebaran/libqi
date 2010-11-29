/*
** Author(s):
**  - Chris Kilner <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#include <qi/messaging/detail/master_impl.hpp>
#include <qi/functors/makefunctor.hpp>
#include <qi/log.hpp>
#include <qi/functors/makefunctor.hpp>
#include <qi/transport/detail/network/endpoints.hpp>
#include <qi/signature.hpp>

#define MASTERIMPL_DEBUG_ENDPOINT_CONTEXT(msg, endpoint)           \
  qisDebug << "===" << msg << " ===" << std::endl;                 \
  qisDebug << "endpointID: " << endpoint.endpointID << std::endl;  \
  qisDebug << "machineID : " << endpoint.machineID  << std::endl;  \
  qisDebug << "name      : " << endpoint.name       << std::endl;  \
  qisDebug << "contextID : " << endpoint.contextID  << std::endl;  \
  qisDebug << "port      : " << endpoint.port       << std::endl;  \
  qisDebug << "============================"        << std::endl;  \

#define MASTERIMPL_DEBUG_MACHINE_CONTEXT(msg, machine)            \
  qisDebug << "===" << msg << " ===" << std::endl;                \
  qisDebug << "machineID : " << machine.machineID  << std::endl;  \
  qisDebug << "hostName  : " << machine.hostName   << std::endl;  \
  qisDebug << "platformID: " << machine.platformID << std::endl;  \
  qisDebug << "publicIP  : " << machine.publicIP   << std::endl;  \
  qisDebug << "============================"        << std::endl; \

namespace qi {
  namespace detail {

    MasterImpl::~MasterImpl() {
      if (!_server.isInitialized()) {
        return;
      }
      const EndpointContext& serverContext = _server.getEndpointContext();
      unregisterServer(serverContext.endpointID);
    }

    MasterImpl::MasterImpl(const std::string& masterAddress) :
        _name("master"),
        _address(masterAddress),
        _server(_name, _address)
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
      xRegisterServer(serverContext);

      // Bind methods
      xAddMasterMethod(serverContext.endpointID, "master.registerService",  this, &MasterImpl::registerService);
      xAddMasterMethod(serverContext.endpointID, "master.locateService",    this, &MasterImpl::locateService);
      xAddMasterMethod(serverContext.endpointID, "master.listServices",     this, &MasterImpl::listServices);
      xAddMasterMethod(serverContext.endpointID, "master.registerMachine",  this, &MasterImpl::registerMachine);
      xAddMasterMethod(serverContext.endpointID, "master.registerServer",   this, &MasterImpl::registerServer);
      xAddMasterMethod(serverContext.endpointID, "master.unregisterServer", this, &MasterImpl::unregisterServer);
      xAddMasterMethod(serverContext.endpointID, "master.registerClient",   this, &MasterImpl::registerClient);
      xAddMasterMethod(serverContext.endpointID, "master.unregisterClient", this, &MasterImpl::unregisterClient);
      xAddMasterMethod(serverContext.endpointID, "master.registerPublisher",   this, &MasterImpl::registerPublisher);
      xAddMasterMethod(serverContext.endpointID, "master.unregisterPublisher", this, &MasterImpl::unregisterPublisher);
      xAddMasterMethod(serverContext.endpointID, "master.topicExists",         this, &MasterImpl::topicExists);
      xAddMasterMethod(serverContext.endpointID, "master.registerTopic",       this, &MasterImpl::registerTopic);
      xAddMasterMethod(serverContext.endpointID, "master.getNewPort", &_addressManager, &AddressManager::getNewPort);
    }

    bool MasterImpl::isInitialized() const {
      return _server.isInitialized();
    }

    void MasterImpl::registerService(
       const std::string& methodSignature, const std::string& serverID) {

      const EndpointContext& serverContext = _knownServers.get(serverID);
      if(serverContext.name.empty()) {
        qisError << "Master::registerService Attempt to register the "
          "method of an unknown server, Please call registerService first"
          ": signature: " << methodSignature << " serverID " <<
            serverID << std::endl;
        return;
      }
      qisDebug << "Master::registerService " << serverContext.name << " " << methodSignature << std::endl;

      _knownServices.insert(methodSignature, serverID);
    }

    void MasterImpl::registerMachine(const std::string& hostName,
      const std::string& machineID, const std::string& publicIPAddress,
      const int& platformID)
    {
      MachineContext m(hostName, machineID, publicIPAddress, platformID);
      xRegisterMachine(m);
    }

    void MasterImpl::xRegisterMachine(const MachineContext& machine) {
      MASTERIMPL_DEBUG_MACHINE_CONTEXT("registerMachine", machine);
      _knownMachines.insert(machine.machineID, machine);
    }

    void MasterImpl::registerServer(const std::string& name,
      const std::string& endpointID, const std::string& contextID,
      const std::string& machineID, const int& port)
    {
      EndpointContext c(name, endpointID, contextID, machineID, 0, port);
      xRegisterServer(c);
    }

    void MasterImpl::xRegisterServer(const EndpointContext& endpoint) {
      MASTERIMPL_DEBUG_ENDPOINT_CONTEXT("Master::registerServer", endpoint);

      if ( ! _knownMachines.exists(endpoint.machineID)) {
        qisError << "Master::registerServer: Attempt to register a "
          "server for a machine that has not bee registered. machineID: " <<
          endpoint.machineID <<
          " Please call registerMachine first." << std::endl;
      } else {
        _knownServers.insert(endpoint.endpointID, endpoint);
      }
    }

    void MasterImpl::unregisterServer(const std::string& id) {

      const EndpointContext& c = _knownServers.get(id);
      MASTERIMPL_DEBUG_ENDPOINT_CONTEXT("Master::unregisterServer", c);
      // TODO remove associated services
      _knownServers.remove(id);
    }

    void MasterImpl::registerClient(const std::string& name,
      const std::string& clientID, const std::string& contextID,
      const std::string& machineID)
    {
      EndpointContext c(name, clientID, contextID, machineID, 0, 0);
      MASTERIMPL_DEBUG_ENDPOINT_CONTEXT("Master::registerClient", c);
      _knownClients.insert(c.endpointID, c);
    }

    void MasterImpl::unregisterClient(const std::string& id) {
      const EndpointContext& c = _knownClients.get(id);
      MASTERIMPL_DEBUG_ENDPOINT_CONTEXT("Master::unregisterClient", c);
      _knownClients.remove(id);
    }

    std::string MasterImpl::locateService(
      const std::string& methodSignature, const std::string& clientID)
    {
      const std::string& serverID = _knownServices.get(methodSignature);
      if (serverID.empty()) {
        qisDebug << "Master::locateService: Could not find server for method: " << methodSignature << std::endl;
        return "";
      }
      const EndpointContext& serverContext = _knownServers.get(serverID);
      if (serverContext.name.empty()) {
        qisDebug << "Master::locateService: Could not find server for serverID: " << serverID << std::endl;
        return "";
      }
      const EndpointContext& clientContext = _knownClients.get(clientID);
      if (clientContext.name.empty()) {
        qisDebug << "Master::locateService: Could not find client for clientID: " << clientID << std::endl;
        return "";
      }
      const MachineContext& serverMachineContext = _knownMachines.get(serverContext.machineID);
      if (serverMachineContext.machineID.empty()) {
        qisDebug << "Master::locateService: Could not find machine for serverID: " << serverID << std::endl;
        return "";
      }
      std::string endpoint = negotiateEndpoint(clientContext, serverContext, serverMachineContext);
      qisDebug << "Master::locateService: Resolved: " << methodSignature << " to " << endpoint << std::endl;
      return endpoint;
    }

    const std::map<std::string, std::string>& MasterImpl::listServices() {
      return _knownServices.getMap();
    }

    void MasterImpl::registerPublisher(const std::string& name,
      const std::string& endpointID,
      const std::string& contextID,
      const std::string& machineID,
      const int& port)
    {
      EndpointContext c(name, endpointID, contextID, machineID, 0, port);
      MASTERIMPL_DEBUG_ENDPOINT_CONTEXT("Master::registerPublisher", c)
      _knownPublishers.insert(endpointID, c);
    }

    void MasterImpl::unregisterPublisher(const std::string& endpointID) {
      const EndpointContext& c = _knownPublishers.get(endpointID);
      MASTERIMPL_DEBUG_ENDPOINT_CONTEXT("Master::unregisterPublisher", c);
      _knownPublishers.remove(endpointID);
    }

    void MasterImpl::registerTopic(const std::string& topicName, const std::string& endpointID) {
      //FIXME: check for existence
      qisDebug << "Register Topic " << topicName << " " << endpointID << std::endl;
      TopicInfo ti;
      ti.publishEndpointID = endpointID;
      ti.subscribeEndpointID = endpointID;
      _knownTopics.insert(topicName, ti);
    }

    bool MasterImpl::topicExists(const std::string& topicName) {
      return _knownTopics.exists(topicName);
    }

  }
}
