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
#include <qi/perf/to_string.hpp> // wrong place, sorry
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

    MasterImpl::MasterImpl(const std::string& masterAddress, Context *ctx) :
    _address(masterAddress),
      _server("master", ctx)
    {
    }

    void MasterImpl::run() {
      _server.connect(_address);
      if (!_server.isInitialized()) {
        return;
      }
      // Force an invalid Machine Context in the lookup
      MachineContext m;
      m.machineID = "";
      _knownMachines.setInvalidValue(m);

      // Register machine and server ( without network activity )
      xRegisterMachine(_server.getMachineContext());
      const EndpointContext& e = _server.getEndpointContext();
      _addressManager.setMasterPort(e.port);
      xRegisterEndpoint(e);

      // Bind methods
      xAddMasterMethod(e.endpointID, "master.registerService",    this, &MasterImpl::registerService);
      xAddMasterMethod(e.endpointID, "master.unregisterService",  this, &MasterImpl::unregisterService);
      xAddMasterMethod(e.endpointID, "master.locateService",      this, &MasterImpl::locateService);
      xAddMasterMethod(e.endpointID, "master.listServices",       this, &MasterImpl::listServices);
      xAddMasterMethod(e.endpointID, "master.listTopics",         this, &MasterImpl::listTopics);
      xAddMasterMethod(e.endpointID, "master.listMachines",       this, &MasterImpl::listMachines);
      xAddMasterMethod(e.endpointID, "master.listEndpoints",      this, &MasterImpl::listEndpoints);
      xAddMasterMethod(e.endpointID, "master.getMachine",         this, &MasterImpl::getMachine);
      xAddMasterMethod(e.endpointID, "master.getEndpoint",        this, &MasterImpl::getEndpoint);
      xAddMasterMethod(e.endpointID, "master.registerMachine",    this, &MasterImpl::registerMachine);
      xAddMasterMethod(e.endpointID, "master.registerEndpoint",   this, &MasterImpl::registerEndpoint);
      xAddMasterMethod(e.endpointID, "master.unregisterEndpoint", this, &MasterImpl::unregisterEndpoint);
      xAddMasterMethod(e.endpointID, "master.topicExists",        this, &MasterImpl::topicExists);
      xAddMasterMethod(e.endpointID, "master.registerTopic",      this, &MasterImpl::registerTopic);
      xAddMasterMethod(e.endpointID, "master.unregisterTopic",    this, &MasterImpl::unregisterTopic);
      xAddMasterMethod(e.endpointID, "master.locateTopic",        this, &MasterImpl::locateTopic);
      xAddMasterMethod(e.endpointID, "master.getNewPort", &_addressManager, &AddressManager::getNewPort);
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

    void MasterImpl::unregisterService(const std::string& methodSignature) {
      _knownServices.remove(methodSignature);
      qisDebug << "Master::unregisterService " << qi::signatureToString(methodSignature) << std::endl;
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
      MASTERIMPL_DEBUG_MACHINE_CONTEXT("Master::registerMachine", machine);
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

    void MasterImpl::unregisterEndpoint(const std::string& endpointID) {
      std::vector<std::string>::const_iterator it;

      // remove services
      std::vector<std::string> servicesToDelete = xListServicesForEndpoint(endpointID);
      if (! servicesToDelete.empty()) {
        for (it = servicesToDelete.begin(); it != servicesToDelete.end(); ++it) {
          unregisterService(*it);
        }
      }

      // remove topics
      std::vector<std::string> topicsToDelete = xListTopicsForEndpoint(endpointID);
      if (! topicsToDelete.empty()) {
        for (it = topicsToDelete.begin(); it != topicsToDelete.end(); ++it) {
          unregisterTopic(*it);
        }
      }

      const EndpointContext& c = _knownEndpoints.get(endpointID);
      MASTERIMPL_DEBUG_ENDPOINT_CONTEXT("Master::unregisterEndpoint", c);
      _knownEndpoints.remove(endpointID);
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

    void MasterImpl::registerTopicParticipant(const std::string& topicName, const std::string& endpointID) {
      Topic& t = _knownTopics.getEditable(topicName);
      if (t.topicName.empty()) {
        qisDebug << "MasterImpl::registerTopicParticipant Could not find topic \"" << topicName << "\"" << std::endl;
        return;
      }

      const EndpointContext& e = _knownEndpoints.get(endpointID);
      switch(e.type) {
        case PUBLISHER_ENDPOINT:
          t.publisherIDs.push_back(e.endpointID);
          qisDebug << "Master::registerTopicParticipant: Added " <<
            "Publisher for \"" <<
            topicName << "\" : " << e.endpointID << std::endl;
          break;
        case SUBSCRIBER_ENDPOINT:
          t.subscriberIDs.push_back(e.endpointID);
          qisDebug << "Master::registerTopicParticipant: Added " <<
            "Subscriber for \"" <<
            topicName << "\" : " << e.endpointID << std::endl;
          break;
        default:
          qisWarning << "Master::registerTopicParticipant: Invalid " <<
            "attempt to register a topic participant for \"" <<
            topicName << "\" that is neither a publisher, nor " <<
            "a subscriber: " << e.endpointID << std::endl;
          break;
      }
    }

    std::string MasterImpl::locateTopic(const std::string& topicName, const std::string& endpointID) {
      const Topic& t = _knownTopics.get(topicName);
      if (t.subscribeEndpointID.empty()) {
        qisDebug << "MasterImpl::locateTopic Could not find topic \"" << topicName << "\"" << std::endl;
        return "";
      }
      std::string ret;
      const EndpointContext& e = _knownEndpoints.get(endpointID);
      if (e.type == PUBLISHER_ENDPOINT) {
          ret = xNegotiateEndpoint(endpointID, t.publishEndpointID);
      } else {
          ret = xNegotiateEndpoint(endpointID, t.subscribeEndpointID);
      }
      qisDebug << "Master::locateTopic: Resolved: " << topicName << " to " << ret << std::endl;
      return ret;
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
      qisDebug << "Master::registerTopic " << topicName << " " << endpointID << std::endl;
      Topic t;
      t.publishEndpointID = endpointID;
      t.subscribeEndpointID = endpointID;
      t.publisherIDs.push_back(endpointID);
      _knownTopics.insert(topicName, t);
    }

    void MasterImpl::unregisterTopic(const std::string& topicName) {
      qisDebug << "Master::unregisterTopic " << topicName << std::endl;
      _knownTopics.remove(topicName);
    }


    bool MasterImpl::topicExists(const std::string& topicName) {
      return _knownTopics.exists(topicName);
    }

    const std::map<std::string, std::string>& MasterImpl::listServices() {
      return _knownServices.getMap();
    }

    std::vector<std::string> MasterImpl::listTopics() {
      return _knownTopics.getKeys();
    }

    std::map<std::string, std::string> MasterImpl::getTopic(const std::string& topicID) {
      std::map<std::string, std::string> result;
      const Topic& t = _knownTopics.get(topicID);
      result.insert(std::make_pair("topicName", t.topicName));
      result.insert(std::make_pair("publishEndpointID", t.publishEndpointID));
      result.insert(std::make_pair("subscribeEndpointID", t.subscribeEndpointID));
      // FIXME ignore publishers and subscribers for now
      return result;
    }

    const std::vector<std::string> MasterImpl::listMachines() {
      return _knownMachines.getKeys();
    }

    const std::vector<std::string> MasterImpl::listEndpoints() {
      return _knownEndpoints.getKeys();
    }

    const std::map<std::string, std::string> MasterImpl::getMachine(const std::string& machineID) {
      const MachineContext& m = _knownMachines.get(machineID);
      std::map<std::string, std::string> ret;
      ret.insert(std::make_pair("machineID", m.machineID));
      ret.insert(std::make_pair("hostName", m.hostName));
      ret.insert(std::make_pair("publicIP", m.publicIP));
      ret.insert(std::make_pair("platform", platformAsString((Platform)m.platformID)));
      return ret;
    }

    const std::map<std::string, std::string> MasterImpl::getEndpoint(const std::string& endpointID) {
      const EndpointContext& e = _knownEndpoints.get(endpointID);
      std::map<std::string, std::string> ret;
      ret.insert(std::make_pair("endpointID", e.endpointID));
      ret.insert(std::make_pair("contextID", e.contextID));
      ret.insert(std::make_pair("machineID", e.machineID));
      ret.insert(std::make_pair("processID", toString(e.processID)));

      ret.insert(std::make_pair("name", e.name));
      ret.insert(std::make_pair("type", endpointTypeAsString(e.type)));
      ret.insert(std::make_pair("port", toString(e.port)));
      return ret;
    }

    std::vector<std::string> MasterImpl::xListServicesForEndpoint(const std::string& endpointID) {
      return _knownServices.getKeysWhereValueEquals(endpointID);
    }

    std::vector<std::string> MasterImpl::xListTopicsForEndpoint(const std::string& endpointID) {
      std::vector<std::string> result;
      const TopicMap& topics = _knownTopics.getMap();
      TopicMapCIT it = topics.begin();
      TopicMapCIT end = topics.end();
      for (; it != end; ++it) {
        if (((it->second).publishEndpointID == endpointID) ||
          ((it->second).subscribeEndpointID == endpointID)) {
            result.push_back(it->first);
        }
      }
      return result;
    }

  }
}
