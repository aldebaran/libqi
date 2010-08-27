/*
** Author(s):
**  - Chris Kilner <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#include "node.hpp"
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <alcommon-ng/messaging/client.hpp>

namespace AL {
  using namespace Messaging;
  namespace Common {

    // dodgy?
    static NodeInfo fInvalidNode;
    static ServiceInfo fInvalidService;

    // TODO create a client only node as a base class
    Node::Node(const std::string& name, const std::string &address) : Server(address) {
      fNodeInfo.name = name;
      fNodeInfo.address = address;

      setMessageHandler(this);

      // just testing
      addService(ServiceInfo(name,name,"addNode"));

      boost::thread serverThread( boost::bind(&Server::run, this));
    }

    const NodeInfo& Node::getNodeInfo() const {
      return fNodeInfo;
    }

    // shame about this definition of a handler....
    // would be great if we could do R onMessage( {mod, meth, T})
    boost::shared_ptr<AL::Messaging::ResultDefinition> Node::onMessage(const AL::Messaging::CallDefinition &def) {
      // handle message
      std::cout << fNodeInfo.name << " received message: " << def.moduleName() << "." << def.methodName() << std::endl;

      std::string key = def.moduleName() + std::string(".") + def.methodName();
      const ServiceInfo& si = getService(key);
      if (si.nodeName == fNodeInfo.name) {
        std::cout << " Method is for this node " << std::endl;
      } else {

        std::cout << " Method is for node: " << si.nodeName << std::endl;

        const NodeInfo& ni = getNode(si.nodeName);
        if (ni.address != "") {
          std::cout << " Node is at address: " << ni.address << std::endl;
        }
      }
      // TODO find the local or remote functor
     // boost::bind<NodeInfo>(&Node::getNode, _1);

      boost::shared_ptr<AL::Messaging::ResultDefinition> res =
        boost::shared_ptr<AL::Messaging::ResultDefinition>(new ResultDefinition());
      return res;
    }


    void Node::addNode(const NodeInfo& node) {
      // TODO verify validity
      fNodeList.insert(
        std::make_pair<std::string, NodeInfo>(node.name, node));
    }

    const NodeInfo& Node::getNode(const std::string& name) const {
      NameLookup<NodeInfo>::const_iterator it = fNodeList.find(name);
      if (it != fNodeList.end()) {
        return it->second;
      }
      return fInvalidNode;
    }

    void Node::addService(const ServiceInfo& service) {
      std::string key = service.moduleName +
        std::string(".") +
        service.methodName;

      fServiceList.insert(
        std::make_pair<std::string, ServiceInfo>(key, service));
    }

    const ServiceInfo& Node::getService(const std::string& name) const {
      NameLookup<ServiceInfo>::const_iterator it = fServiceList.find(name);
      if (it != fServiceList.end()) {
        return it->second;
      }
      return fInvalidService;
    }

    // ---- private

    void Node::xCreateNodeClient(const NodeInfo& node) {
      boost::shared_ptr<DefaultClient> client = boost::shared_ptr<DefaultClient>(new DefaultClient(node.address));
      fNodeClients.insert(make_pair(node.name, client));
    }
  }
}


// Cedric ... this kind of thing works quite well for binding.
// shame that boost::bind(&f, ...) does not work in msvc
// struct visitor
//{
//    template<class T> void operator()( boost::reference_wrapper<T> const & r ) const
//    {
//        std::cout << "Reference to " << typeid(T).name() << " @ " << &r.get() << " (with value " << r.get() << ")\n";
//    }
//
//    template<class T> void operator()( T const & t ) const
//    {
//        std::cout << "Value of type " << typeid(T).name() << " (with value " << t << ")\n";
//    }
//};
//visitor v;
//visit_each(v, boost::bind<NodeInfo>(&Node::getNode, _1));
