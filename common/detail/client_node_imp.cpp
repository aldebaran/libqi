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
#include <alcommon-ng/exceptions/exceptions.hpp>

namespace AL {
  using namespace Messaging;
  namespace Common {

    ClientNodeImp::ClientNodeImp() : initOK(false) {}

    ClientNodeImp::~ClientNodeImp() {}

    ClientNodeImp::ClientNodeImp(
      const std::string& clientName,
      const std::string& masterAddress) :
      initOK(false),
      fClientName(clientName),
      fMasterAddress(masterAddress) {
        xInit();
    }

    void ClientNodeImp::xInit() {
      initOK = xCreateServerClient(fMasterAddress);
      if (initOK) {
        // we assert that we think the master can locate services
        fServiceCache.insert("master.locateService::s#&:s", fMasterAddress);
      } else {
        alserror << "\"" << fClientName << "\" Failed to connect to master at address \"" << fMasterAddress << "\"";
      }
    }

    void ClientNodeImp::call(
      const CallDefinition& callDef,  AL::Messaging::ResultDefinition &result) {
      if (! initOK) {
        return;
      }

      // will throw if service not found
      const std::string& nodeAddress = xLocateService(callDef.methodName());

      // get the relevant messaging client for the node that host the service
      NameLookup<boost::shared_ptr<Client> >::iterator it;
      it = fServerClients.find(nodeAddress);
      if (it == fServerClients.end()) {
        // create messaging client if needed ...
        xCreateServerClient(nodeAddress);
        it = fServerClients.find(nodeAddress);

        if (it == fServerClients.end()) {
          alserror << "Could not create client for server \"" << nodeAddress << "\" Probable connection error. " << callDef.methodName();
          throw( new AL::Transport::ConnectionException("Could not create client for server. Probable connection error."));
          return;
        }
      }
      //TODO: optimise
      result = (it->second)->send(callDef);
      return;
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
        alserror << "Failed to create client for address " << serverAddress << " Reason: connect failure.";
      }
      return ok;
    }

    const std::string& ClientNodeImp::xLocateService(
      const std::string& methodHash) {
      const std::string& nodeAddress = fServiceCache.get(methodHash);
      if (!nodeAddress.empty()) {
        return nodeAddress;
      }

      try {
        ResultDefinition r;
        call(CallDefinition("master.locateService::s#&:s", methodHash), r);
        std::string tmpAddress = r.value().as<std::string>();

        if (!tmpAddress.empty()) {
          // cache located service pair
          fServiceCache.insert(methodHash, tmpAddress);
          // return a const string ref address
          return fServiceCache.get(methodHash);
        } else {
          alserror << "Unable to find service \"" << methodHash << "\" Reason: Method not known to master.";
          throw( new AL::Transport::ServiceNotFoundException("Unable to find service. Reason: Method not known to master."));
        }
      } catch(const std::exception& e) {
        alserror << "Unable to find service \"" << methodHash << "\" Reason: master not found. Detail: " << e.what();
        throw( new AL::Transport::ServiceNotFoundException("Unable to find service. Reason: master not found."));
      }

      // never happens: we either return early or throw
      return nodeAddress;
    }
  }
}
