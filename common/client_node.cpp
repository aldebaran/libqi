/*
** Author(s):
**  - Chris Kilner <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#include <alcommon-ng/common/client_node.hpp>
#include <alcommon-ng/messaging/client.hpp>

namespace AL {
  using namespace Messaging;
  namespace Common {

    ClientNode::ClientNode(const std::string& clientName,
        const std::string &masterAddress) {
      fName = clientName;
      fMasterAddress = masterAddress;
      xInit();
    }

    ClientNode::~ClientNode() {}

    void ClientNode::xInit() {
      NodeInfo master("master", fMasterAddress);
      xCreateServerClient(master);
      xUpdateServicesFromMaster();
    }

    AL::Messaging::ResultDefinition call(
      const AL::Messaging::CallDefinition& callDef) {
        // make a hash from the calldef
        // lookup in the service list
        // create messaging client if needed
        // call
        ResultDefinition r;
        return r;
    }

    void ClientNode::xUpdateServicesFromMaster() {
      // if master is alive
      // get list of services
    }

    void ClientNode::xCreateServerClient(const NodeInfo& serverNodeInfo) {
      // TODO error handling
      boost::shared_ptr<DefaultClient> client = 
        boost::shared_ptr<DefaultClient>(new DefaultClient(serverNodeInfo.address));

      fServerClients.insert(make_pair(serverNodeInfo.name, client));
      fServerList.insert(make_pair(serverNodeInfo.name, serverNodeInfo)); // why not!
    }

  }
}
