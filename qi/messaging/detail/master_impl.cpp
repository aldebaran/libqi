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

namespace qi {
  namespace detail {

    MasterImpl::~MasterImpl() {}

    MasterImpl::MasterImpl(const std::string& masterAddress) :
        _name("master"),
        _address(masterAddress),
        _server(_name, _address)
    {
      xInit();
    }


    void MasterImpl::xInit() {
      xAddMasterMethod("master.registerService",  &MasterImpl::registerService);
      xAddMasterMethod("master.locateService",    &MasterImpl::locateService);
      xAddMasterMethod("master.listServices",     &MasterImpl::listServices);
      xAddMasterMethod("master.registerServer",   &MasterImpl::registerServer);
      xAddMasterMethod("master.unregisterServer", &MasterImpl::unregisterServer);
      xAddMasterMethod("master.registerClient",   &MasterImpl::registerClient);
      xAddMasterMethod("master.unregisterClient", &MasterImpl::unregisterClient);
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
      qisInfo << "Master::registerService " << serverContext.publicIP <<
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

      qisInfo << "Master::registerServer =====" << std::endl;
      qisInfo << "endpointID: " << c.endpointID << std::endl;
      qisInfo << "machineID : " << c.machineID  << std::endl;
      qisInfo << "hostName  : " << c.hostName   << std::endl;
      qisInfo << "processID : " << c.processID  << std::endl;
      qisInfo << "platformID: " << c.platformID << std::endl;
      qisInfo << "publicIP  : " << c.publicIP   << std::endl;
      qisInfo << "name      : " << c.name       << std::endl;
      qisInfo << "contextID : " << c.contextID  << std::endl;
      qisInfo << "port      : " << c.port       << std::endl;
      qisInfo << "============================" << std::endl;

      _knownServers.insert(c.endpointID, c);

      return c.port;
    }

    void MasterImpl::unregisterServer(const std::string& id) {

      const EndpointContext& c = _knownServers.get(id);
      qisInfo << "Master::unregisterServer ===" << std::endl;
      qisInfo << "endpointID: " << c.endpointID << std::endl;
      qisInfo << "machineID : " << c.machineID  << std::endl;
      qisInfo << "hostName  : " << c.hostName   << std::endl;
      qisInfo << "processID : " << c.processID  << std::endl;
      qisInfo << "platformID: " << c.platformID << std::endl;
      qisInfo << "publicIP  : " << c.publicIP   << std::endl;
      qisInfo << "name      : " << c.name       << std::endl;
      qisInfo << "contextID : " << c.contextID  << std::endl;
      qisInfo << "port      : " << c.port       << std::endl;
      qisInfo << "============================" << std::endl;

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

      qisInfo << "Master::registerClient =====" << std::endl;
      qisInfo << "endpointID: " << c.endpointID << std::endl;
      qisInfo << "machineID : " << c.machineID  << std::endl;
      qisInfo << "hostName  : " << c.hostName   << std::endl;
      qisInfo << "processID : " << c.processID  << std::endl;
      qisInfo << "platformID: " << c.platformID << std::endl;
      qisInfo << "publicIP  : " << c.publicIP   << std::endl;
      qisInfo << "name      : " << c.name       << std::endl;
      qisInfo << "contextID : " << c.contextID  << std::endl;
      qisInfo << "port      : " << c.port       << std::endl;
      qisInfo << "============================" << std::endl;

      _knownClients.insert(c.endpointID, c);
    }

    void MasterImpl::unregisterClient(const std::string& id) {
      const EndpointContext& c = _knownClients.get(id);
      qisInfo << "Master::unregisterClient ===" << std::endl;
      qisInfo << "endpointID: " << c.endpointID << std::endl;
      qisInfo << "machineID : " << c.machineID  << std::endl;
      qisInfo << "hostName  : " << c.hostName   << std::endl;
      qisInfo << "processID : " << c.processID  << std::endl;
      qisInfo << "platformID: " << c.platformID << std::endl;
      qisInfo << "publicIP  : " << c.publicIP   << std::endl;
      qisInfo << "name      : " << c.name       << std::endl;
      qisInfo << "contextID : " << c.contextID  << std::endl;
      qisInfo << "port      : " << c.port       << std::endl;
      qisInfo << "============================" << std::endl;

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
