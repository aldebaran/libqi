/*
** Author(s):
**  - Chris Kilner <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#include <alcommon-ng/common/detail/client_node_imp.hpp>
#include <string>
#include <alcommon-ng/common/detail/get_protocol.hpp>
#include <alcommon-ng/messaging/client.hpp>
#include <allog/allog.h>

namespace AL {
  using namespace Messaging;
  namespace Common {

    ClientNodeImp::ClientNodeImp() : initOK(false) {}

    ClientNodeImp::ClientNodeImp(
      const std::string& clientName,
      const std::string& masterAddress) :
      initOK(false),
      fClientName(clientName),
      fMasterAddress(masterAddress) {
        xInit();
    }

    ClientNodeImp::~ClientNodeImp() {}

    void ClientNodeImp::xInit() {
      initOK = xCreateServerClient(fMasterAddress);
      if (initOK) {
        // we assert that we think the master can locate services
        fServiceCache.insert("master.locateService", fMasterAddress);
      } else {
        alserror << "\"" << fClientName << "\" Failed to connect to master at address \"" << fMasterAddress << "\"";
      }
    }

    void ClientNodeImp::call(const std::string& methodName,
      AL::Messaging::ReturnValue& result) {
      CallDefinition def;
      def.methodName() = methodName;
      ResultDefinition res = xCall(def);
      result = res.value(); // copy crazy
    }

    void ClientNodeImp::call(const std::string& methodName,
        const AL::Messaging::ArgumentList& params,
        AL::Messaging::ReturnValue& result) {
      CallDefinition def;
      def.methodName() = methodName;
      def.args() = params;

      ResultDefinition res = xCall(def);
      result = res.value(); // copy crazy
    }

    ResultDefinition ClientNodeImp::xCall(
      const CallDefinition& callDef) {
      // todo make a hash from the calldef
      ResultDefinition result;
      if (! initOK) {
        // should we have killed the client?
        return result;
      }

      std::string hash = callDef.methodName();
      std::string nodeAddress = xLocateService(hash);

      if (nodeAddress.empty()) {
        // problem the master doesn't know about this message
        // throw?
        alserror << "Error Client: " << fClientName <<
          " could not find Server for message " << hash;
        return result;
      }

      // get the relevant messaging client for the node that host the service
      NameLookup<boost::shared_ptr<Client> >::iterator it;
      it = fServerClients.find(nodeAddress);
      if (it == fServerClients.end()) {
        // create messaging client if needed ...
        xCreateServerClient(nodeAddress);
        it = fServerClients.find(nodeAddress);

        if (it == fServerClients.end()) {
          alserror << "Client: " << fClientName <<
            ", could not find Server for message " << hash;
          // throw?
          return result;
        }
      }

      // call
      alsdebug << "Client: " << fClientName <<
        ", found server for message " << hash;
      result = (it->second)->send(callDef);
      alsdebug << "  Client: " << fClientName << ", received result";
      return result;
    }

    bool ClientNodeImp::xCreateServerClient(const std::string& serverAddress) {
      // TODO(chris) error handling
      // we don't yet know our current IP address, so we fake
      std::string serverFullAddress = getProtocol(serverAddress, serverAddress) + serverAddress;

      boost::shared_ptr<Client> client =
        boost::shared_ptr<Client>(new AL::Messaging::Client());
      bool ok = client->connect(serverFullAddress);
      if (ok) {
        fServerClients.insert(make_pair(serverAddress, client));
        alsdebug << "Client " << fClientName <<
          " creating client for server " << serverFullAddress << std::endl;
      } else {
        alsdebug << "Failed to create client for address " << serverAddress << " Reason: connect fail";
      }
      return ok;
    }

    const std::string ClientNodeImp::xLocateService(
      const std::string& methodHash) {
      std::string nodeAddress = fServiceCache.get(methodHash);

      // empty means not found
      if (!nodeAddress.empty()) {
        return nodeAddress;
      }

      alsdebug << "Client " << fClientName <<
        " asking master to locate service " << methodHash;
      ArgumentList args;
      args.push_back(methodHash);
      ReturnValue ret;
      try {
        call("master.locateService", args, ret);
      } catch(const std::exception& e) {
        alserror << "Could not connect to master Reason: " << e.what();
        return "";
      }
      nodeAddress = ret.as<std::string>();

      return nodeAddress;
    }
  }
}
