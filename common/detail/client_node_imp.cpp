/*
** Author(s):
**  - Chris Kilner <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#include <alcommon-ng/common/detail/client_node_imp.hpp>
#include <alcommon-ng/messaging/client.hpp>
#include <allog/allog.h>

// would not be needed if we had a specific client
#include <alcommon-ng/messaging/call_definition.hpp>
#include <alcommon-ng/messaging/result_definition.hpp>

namespace AL {
  using namespace Messaging;
  namespace Common {

    ClientNodeImp::ClientNodeImp() {}

    ClientNodeImp::ClientNodeImp(
      const std::string& clientName,
      const std::string& masterAddress) : 
      fClientName(clientName),
      fMasterAddress(masterAddress) {
        xInit();
    }

    ClientNodeImp::~ClientNodeImp() {}

    void ClientNodeImp::xInit() {
      xCreateServerClient(fMasterAddress);
      // we assert that we think the master can locate services
      fServiceCache.insert("master.locateService", fMasterAddress);
    }

    ResultDefinition ClientNodeImp::call(const CallDefinition& callDef) {        
      // todo make a hash from the calldef
      ResultDefinition result;

      std::string hash = callDef.moduleName() + "." + callDef.methodName();
      std::string nodeAddress = xLocateService(hash);

      if (nodeAddress.empty()) {
        // problem the master doesn't know about this message
        // throw?
        alserror << "Error Client: " << fClientName << " could not find Server for message " << hash;
        return result;
      }

      // get the relevant messaging client for the node that host the service
      NameLookup<boost::shared_ptr<DefaultClient> >::iterator it;
      it = fServerClients.find(nodeAddress);
      if (it == fServerClients.end()) {
        // create messaging client if needed ...
        xCreateServerClient(nodeAddress);
        it = fServerClients.find(nodeAddress);

        if (it == fServerClients.end()) {
          alserror << "Client: " << fClientName << ", could not find Server for message " << hash;
          // throw?
          return result;
        }
      }

      // call
      alsdebug << "Client: " << fClientName << ", found server for message " << hash;
      result = (it->second)->send(callDef);
      alsdebug << "  Client: " << fClientName << ", received result";
      return result;  
    }


    void ClientNodeImp::xCreateServerClient(const std::string& serverAddress) {
      // TODO error handling
      boost::shared_ptr<DefaultClient> client = 
        boost::shared_ptr<DefaultClient>(new DefaultClient(serverAddress));

      alsdebug << "Client " << fClientName << " creating client for server " << serverAddress << std::endl;

      // add server client
      fServerClients.insert(make_pair(serverAddress, client));
    }


    const std::string ClientNodeImp::xLocateService(const std::string& methodHash) {
      std::string nodeAddress = fServiceCache.get(methodHash);

      // empty means not found
      if (!nodeAddress.empty()) {
        return nodeAddress;
      }

      alsdebug << "Client " << fClientName << " asking master to locate service " << methodHash;
      CallDefinition callDef;
      callDef.moduleName() = "master";
      callDef.methodName() = "locateService";
      callDef.args().push_back(methodHash);

      ResultDefinition res = call(callDef);
      nodeAddress = res.value();

      return nodeAddress;
    }

  }
}
