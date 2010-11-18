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
        fName("master"),
        fAddress(masterAddress),
        fServer(fName, masterAddress, masterAddress) {
      xInit();
    }

    void MasterImpl::xInit() {
      fServer.addService("master.registerService", this, &MasterImpl::registerService);
      registerService(fAddress, "master.registerService::v:ss");
      fServer.addService("master.locateService", this, &MasterImpl::locateService);
      registerService(fAddress, "master.locateService::s:s");
      fServer.addService("master.listServices", this, &MasterImpl::listServices);
      registerService(fAddress, "master.listServices::{ss}:");

      fServer.addService("master.registerServerNode", this, &MasterImpl::registerServerNode);
      registerService(fAddress, "master.registerServerNode::v:ss");
      fServer.addService("master.unregisterServerNode", this, &MasterImpl::unregisterServerNode);
      registerService(fAddress, "master.unregisterServerNode::v:ss");

      fServer.addService("master.registerClientNode", this, &MasterImpl::registerClientNode);
      registerService(fAddress, "master.registerClientNode::v:ss");
      fServer.addService("master.unregisterClientNode", this, &MasterImpl::unregisterClientNode);
      registerService(fAddress, "master.unregisterClientNode::v:ss");
    }

    void MasterImpl::registerService(
      const std::string& nodeAddress, const std::string& methodSignature) {
      qisInfo << "Master::registerService " << nodeAddress << " " << methodSignature << std::endl;
      fServiceCache.insert(methodSignature, nodeAddress);
    }

    void MasterImpl::registerServerNode(const std::string& nodeName, const std::string& nodeAddress) {
      qisInfo << "Master::registerServerNode " << nodeName << " " << nodeAddress << std::endl;
      fServerNodeCache.insert(nodeName, nodeAddress);
    }

    void MasterImpl::unregisterServerNode(const std::string& nodeName, const std::string& nodeAddress) {
      qisInfo << "Master::unregisterServerNode " << nodeName << " " << nodeAddress << std::endl;
      // todo remove associated services
      fServerNodeCache.remove(nodeName);
    }

    void MasterImpl::registerClientNode(const std::string& nodeName, const std::string& nodeAddress) {
      qisInfo << "Master::registerClientNode " << nodeName << " " << nodeAddress << std::endl;
      fClientNodeCache.insert(nodeName, nodeAddress);
    }

    void MasterImpl::unregisterClientNode(const std::string& nodeName, const std::string& nodeAddress) {
      qisInfo << "Master::unregisterClientNode " << nodeName << " " << nodeAddress << std::endl;
      fClientNodeCache.remove(nodeName);
    }

    //TODO: add a ref to the return value
    const std::string MasterImpl::locateService(const std::string& methodSignature) {
      // If not found, an empty string is returned
      // TODO friendly error describing closest service?
      return fServiceCache.get(methodSignature);
    }

    const std::map<std::string, std::string>& MasterImpl::listServices() {
      return fServiceCache.getMap();
    }

  }
}
