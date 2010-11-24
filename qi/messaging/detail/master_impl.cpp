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
#include <qi/transport/detail/network/negotiate_endpoint.hpp>
#include <qi/signature.hpp>

#define MASTERIMPL_DEBUG_ENDPOINT(msg, endpoint)                   \
  qisDebug << "===" << msg << " ===" << std::endl;                 \
  qisDebug << "endpointID: " << endpoint.endpointID << std::endl;  \
  qisDebug << "machineID : " << endpoint.machineID  << std::endl;  \
  qisDebug << "hostName  : " << endpoint.hostName   << std::endl;  \
  qisDebug << "processID : " << endpoint.processID  << std::endl;  \
  qisDebug << "platformID: " << endpoint.platformID << std::endl;  \
  qisDebug << "publicIP  : " << endpoint.publicIP   << std::endl;  \
  qisDebug << "name      : " << endpoint.name       << std::endl;  \
  qisDebug << "contextID : " << endpoint.contextID  << std::endl;  \
  qisDebug << "port      : " << endpoint.port       << std::endl;  \
  qisDebug << "============================" << std::endl;         \

namespace qi {
  namespace detail {

    MasterImpl::~MasterImpl() {
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
      const EndpointContext& serverContext = _server.getEndpointContext();
      registerServer(serverContext.name,
        serverContext.endpointID,
        serverContext.contextID,
        serverContext.machineID,
        serverContext.platformID,
        serverContext.publicIP);

      xAddMasterMethod(serverContext.endpointID, "master.registerService",  &MasterImpl::registerService);
      xAddMasterMethod(serverContext.endpointID, "master.locateService",    &MasterImpl::locateService);
      xAddMasterMethod(serverContext.endpointID, "master.listServices",     &MasterImpl::listServices);
      xAddMasterMethod(serverContext.endpointID, "master.registerServer",   &MasterImpl::registerServer);
      xAddMasterMethod(serverContext.endpointID, "master.unregisterServer", &MasterImpl::unregisterServer);
      xAddMasterMethod(serverContext.endpointID, "master.registerClient",   &MasterImpl::registerClient);
      xAddMasterMethod(serverContext.endpointID, "master.unregisterClient", &MasterImpl::unregisterClient);
    }

    void MasterImpl::registerService(
       const std::string& methodSignature, const std::string& serverID) {

      const EndpointContext& serverContext = _knownServers.get(serverID);
      if(serverContext.name.empty()) {
        qisError << "Master::registerService Attempt to register the method of an unknown server, "
            " Please call registerService first: signature: " << methodSignature << " serverID " <<
            serverID << std::endl;
        return;
      }
      qisDebug << "Master::registerService " << serverContext.publicIP <<
         ":" << serverContext.port << " " << methodSignature << std::endl;

      _knownServices.insert(methodSignature, serverID);
    }

    int MasterImpl::registerServer(const std::string& name,
                                   const std::string& id,
                                   const std::string& contextID,
                                   const std::string& machineID,
                                   const int&         platformID,
                                   const std::string& publicIPAddress)
    {
      // Put into a context struct: TODO use a protobuf
      EndpointContext c;
      c.name       = name;
      c.endpointID = id;
      c.contextID  = contextID;
      c.machineID  = machineID;
      c.platformID = platformID;
      c.publicIP   = publicIPAddress;
      c.port = addressManager.getNewPort();
      MASTERIMPL_DEBUG_ENDPOINT("Master::registerServer", c);

      _knownServers.insert(c.endpointID, c);

      return c.port;
    }

    void MasterImpl::unregisterServer(const std::string& id) {

      const EndpointContext& c = _knownServers.get(id);
      MASTERIMPL_DEBUG_ENDPOINT("Master::unregisterServer", c);

      // TODO remove associated services
      _knownServers.remove(id);
    }

    void MasterImpl::registerClient(const std::string& name,
                                    const std::string& id,
                                    const std::string& contextID,
                                    const std::string& machineID,
                                    const int& platformID)
    {
      // Put into a context struct: TODO use a protobuf
      EndpointContext c;
      c.name       = name;
      c.endpointID = id;
      c.contextID  = contextID;
      c.machineID  = machineID;
      c.platformID = platformID;
      MASTERIMPL_DEBUG_ENDPOINT("Master::registerClient", c);

      _knownClients.insert(c.endpointID, c);
    }

    void MasterImpl::unregisterClient(const std::string& id) {
      const EndpointContext& c = _knownClients.get(id);
      MASTERIMPL_DEBUG_ENDPOINT("Master::unregisterClient", c);
      _knownClients.remove(id);
    }

    std::string MasterImpl::locateService(const std::string& methodSignature, const std::string& clientID) {
      // TODO error checking
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
      std::string endpoint = negotiateEndpoint(clientContext, serverContext);
      qisDebug << "Master::locateService: Resolved: " << methodSignature << " to " << endpoint << std::endl;
      return endpoint;
    }

    const std::map<std::string, std::string>& MasterImpl::listServices() {
      return _knownServices.getMap();
    }

  }
}
