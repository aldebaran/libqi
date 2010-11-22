/*
** Author(s):
**  - Chris Kilner <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#include <qi/messaging/detail/master_impl.hpp>
#include <qi/functors/makefunctor.hpp>
#include <qi/log.hpp>

namespace qi {
  namespace detail {

    MasterImpl::~MasterImpl() {}

    MasterImpl::MasterImpl(const std::string& masterAddress) :
        _name("master"),
        _address(masterAddress),
        _server(_name, _address)
    {
      _endpointContext.name = _name;
      _endpointContext.contextID = _qiContext.getID();
      xInit();
    }

    void MasterImpl::xInit() {
      _server.addService("master.registerService", this, &MasterImpl::registerService);
      registerService(_address, "master.registerService::v:ss");
      _server.addService("master.locateService", this, &MasterImpl::locateService);
      registerService(_address, "master.locateService::s:s");
      _server.addService("master.listServices", this, &MasterImpl::listServices);
      registerService(_address, "master.listServices::{ss}:");

      _server.addService("master.registerServer", this, &MasterImpl::registerServer);
      registerService(_address, "master.registerServer::i:ssssis");
      _server.addService("master.unregisterServer", this, &MasterImpl::unregisterServer);
      registerService(_address, "master.unregisterServer::v:s");

      _server.addService("master.registerClient", this, &MasterImpl::registerClient);
      registerService(_address, "master.registerClient::v:ssssi");
      _server.addService("master.unregisterClient", this, &MasterImpl::unregisterClient);
      registerService(_address, "master.unregisterClient::v:s");
    }

    void MasterImpl::registerService(
      const std::string& nodeAddress, const std::string& methodSignature) {
      qisInfo << "Master::registerService " << nodeAddress << " " << methodSignature << std::endl;
      _knownServices.insert(methodSignature, nodeAddress);
    }

    int MasterImpl::registerServer(const std::string& name,
                                   const std::string& id,
                                   const std::string& contextID,
                                   const std::string& machineID,
                                   const int&         platformID,
                                   const std::string& publicIPAddress)
    {
      // Put into a context struct: TODO use a protobuf
      qi::detail::EndpointContext c;
      c.name       = name;
      c.endpointID = id;
      c.contextID  = contextID;
      c.machineID  = machineID;
      c.platformID = platformID;
      c.publicIP   = publicIPAddress;
      c.port = addressManager.getNewPort();

      qisInfo << "Master::registerServer ===" << std::endl;
      qisInfo << "endpointID: " << c.endpointID << std::endl;
      qisInfo << "machineID : " << c.machineID  << std::endl;
      qisInfo << "hostName  : " << c.hostName   << std::endl;
      qisInfo << "processID : " << c.processID  << std::endl;
      qisInfo << "platformID: " << c.platformID << std::endl;
      qisInfo << "publicIP  : " << c.publicIP   << std::endl;
      qisInfo << "name      : " << c.name       << std::endl;
      qisInfo << "contextID : " << c.contextID  << std::endl;
      qisInfo << "port      : " << c.port       << std::endl;
      qisInfo << "==========================" << std::endl;

      _knownServers.insert(c.endpointID, c);

      return c.port;
    }

    void MasterImpl::unregisterServer(const std::string& id) {

      const EndpointContext& c = _knownServers.get(id);
      qisInfo << "Master::unregisterServer =" << std::endl;
      qisInfo << "endpointID: " << c.endpointID << std::endl;
      qisInfo << "machineID : " << c.machineID  << std::endl;
      qisInfo << "hostName  : " << c.hostName   << std::endl;
      qisInfo << "processID : " << c.processID  << std::endl;
      qisInfo << "platformID: " << c.platformID << std::endl;
      qisInfo << "publicIP  : " << c.publicIP   << std::endl;
      qisInfo << "name      : " << c.name       << std::endl;
      qisInfo << "contextID : " << c.contextID  << std::endl;
      qisInfo << "port      : " << c.port       << std::endl;
      qisInfo << "==========================" << std::endl;

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
      qi::detail::EndpointContext c;
      c.name       = name;
      c.endpointID = id;
      c.contextID  = contextID;
      c.machineID  = machineID;
      c.platformID = platformID;

      qisInfo << "Master::registerClient ===" << std::endl;
      qisInfo << "endpointID: " << c.endpointID << std::endl;
      qisInfo << "machineID : " << c.machineID  << std::endl;
      qisInfo << "hostName  : " << c.hostName   << std::endl;
      qisInfo << "processID : " << c.processID  << std::endl;
      qisInfo << "platformID: " << c.platformID << std::endl;
      qisInfo << "publicIP  : " << c.publicIP   << std::endl;
      qisInfo << "name      : " << c.name       << std::endl;
      qisInfo << "contextID : " << c.contextID  << std::endl;
      qisInfo << "port      : " << c.port       << std::endl;
      qisInfo << "==========================" << std::endl;

      _knownClients.insert(c.endpointID, c);
    }

    void MasterImpl::unregisterClient(const std::string& id) {
      const EndpointContext& c = _knownClients.get(id);
      qisInfo << "Master::unregisterClient =" << std::endl;
      qisInfo << "endpointID: " << c.endpointID << std::endl;
      qisInfo << "machineID : " << c.machineID  << std::endl;
      qisInfo << "hostName  : " << c.hostName   << std::endl;
      qisInfo << "processID : " << c.processID  << std::endl;
      qisInfo << "platformID: " << c.platformID << std::endl;
      qisInfo << "publicIP  : " << c.publicIP   << std::endl;
      qisInfo << "name      : " << c.name       << std::endl;
      qisInfo << "contextID : " << c.contextID  << std::endl;
      qisInfo << "port      : " << c.port       << std::endl;
      qisInfo << "==========================" << std::endl;

      _knownClients.remove(id);
    }

    //TODO: add a ref to the return value
    const std::string MasterImpl::locateService(const std::string& methodSignature) {
      // If not found, an empty string is returned
      // TODO friendly error describing closest service?
      return _knownServices.get(methodSignature);
    }

    const std::map<std::string, std::string>& MasterImpl::listServices() {
      return _knownServices.getMap();
    }

  }
}
