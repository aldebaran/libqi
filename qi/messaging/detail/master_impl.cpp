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
        _server(_name, _address, _address) {
      xInit();
    }

    void MasterImpl::xInit() {
      _server.addService("master.registerService", this, &MasterImpl::registerService);
      registerService(_address, "master.registerService::v:ss");
      _server.addService("master.locateService", this, &MasterImpl::locateService);
      registerService(_address, "master.locateService::s:s");
      _server.addService("master.listServices", this, &MasterImpl::listServices);
      registerService(_address, "master.listServices::{ss}:");

      _server.addService("master.registerServerNode", this, &MasterImpl::registerServerNode);
      registerService(_address, "master.registerServerNode::v:ss");
      _server.addService("master.unregisterServerNode", this, &MasterImpl::unregisterServerNode);
      registerService(_address, "master.unregisterServerNode::v:ss");

      _server.addService("master.registerClientNode", this, &MasterImpl::registerClientNode);
      registerService(_address, "master.registerClientNode::v:ss");
      _server.addService("master.unregisterClientNode", this, &MasterImpl::unregisterClientNode);
      registerService(_address, "master.unregisterClientNode::v:ss");
    }

    void MasterImpl::registerService(
      const std::string& nodeAddress, const std::string& methodSignature) {
      qisInfo << "Master::registerService " << nodeAddress << " " << methodSignature << std::endl;
      _knownServices.insert(methodSignature, nodeAddress);
    }

    void MasterImpl::registerServerNode(const std::string& nodeName, const std::string& nodeAddress) {
      qisInfo << "Master::registerServerNode " << nodeName << " " << nodeAddress << std::endl;
      _knownServers.insert(nodeName, nodeAddress);
    }

    void MasterImpl::unregisterServerNode(const std::string& nodeName, const std::string& nodeAddress) {
      qisInfo << "Master::unregisterServerNode " << nodeName << " " << nodeAddress << std::endl;
      // todo remove associated services
      _knownServers.remove(nodeName);
    }

    void MasterImpl::registerClientNode(const std::string& nodeName, const std::string& nodeAddress) {
      qisInfo << "Master::registerClientNode " << nodeName << " " << nodeAddress << std::endl;
      _knownClients.insert(nodeName, nodeAddress);
    }

    void MasterImpl::unregisterClientNode(const std::string& nodeName, const std::string& nodeAddress) {
      qisInfo << "Master::unregisterClientNode " << nodeName << " " << nodeAddress << std::endl;
      _knownClients.remove(nodeName);
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
