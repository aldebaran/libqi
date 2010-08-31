/*
** Author(s):
**  - Chris Kilner <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#include <alcommon-ng/common/client_node.hpp>
#include <alcommon-ng/messaging/client.hpp>

// would not be needed if we had a specific client
#include <alcommon-ng/messaging/call_definition.hpp>
#include <alcommon-ng/messaging/result_definition.hpp>

namespace AL {
  using namespace Messaging;
  namespace Common {

    ClientNode::ClientNode() {}

    ClientNode::ClientNode(
      const std::string& clientName,
      const std::string& masterAddress) : 
        fClientName(clientName),
        fMasterAddress(masterAddress) {
      xInit();
    }

    ClientNode::~ClientNode() {}

    void ClientNode::xInit() {
      xCreateServerClient(fMasterAddress);
      xUpdateServicesFromMaster();
    }

    ResultDefinition ClientNode::call(const CallDefinition& callDef) {        
        // todo make a hash from the calldef
        std::string hash = callDef.moduleName() + "." + callDef.methodName();
        std::string nodeName = xLocateService(hash);
        
        // get the relevant messaging client for the node that host the service
        NameLookup<boost::shared_ptr<DefaultClient> >::iterator it;
        it = fServerClients.find(hash);
        if (it == fServerClients.end()) {
           // create messaging client if needed ???
          std::cout << "Client: " << fClientName << ", could not find Server for message " << hash << std::endl;
          // throw?
          ResultDefinition r;
          return r;
        }

        // call
        std::cout << "Client: " << fClientName << ", found server for message " << hash << "." << callDef.methodName() <<  std::endl;
        ResultDefinition result = (it->second)->send(callDef);
        std::cout << "  Client: " << fClientName << ", received result" << std::endl;
        return result;  
    }

    void ClientNode::xUpdateServicesFromMaster() {
      // if master is alive
      // get list of services
    }

    void ClientNode::xCreateServerClient(const std::string& serverAddress) {
      // TODO error handling
      boost::shared_ptr<DefaultClient> client = 
        boost::shared_ptr<DefaultClient>(new DefaultClient(serverAddress));

      // TODO find existing server and update if it exists

      // add server client
      fServerClients.insert(make_pair(serverAddress, client));
    }


    const std::string ClientNode::xLocateService(const std::string& methodHash) {
      std::string nodeAddress = fServiceCache.get(methodHash);

      // empty means not found
      if (!nodeAddress.empty()) {
        return nodeAddress;
      }

      // TODO ... force master name to "master" ?
      if (fClientName != "master") {
        // cache lookup failed ... time to ask master

        // add service to cache

        // return it
      }

      return nodeAddress;
    }

  }
}
