/*
*  Author(s):
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/

#include "src/messaging/client_impl.hpp"
#include <qimessaging/exceptions.hpp>
#include <qimessaging/signature.hpp>
#include <qi/log.hpp>

namespace qi {

  namespace detail {

    ClientImpl::~ClientImpl() {
      _masterClient.unregisterEndpoint(_endpointContext);
    }

    ClientImpl::ClientImpl(const std::string& name, Context *ctx)
      : ImplBase(ctx)
    {
      _endpointContext.type = CLIENT_ENDPOINT;
      _endpointContext.name = name;
      _endpointContext.contextID = _masterClient.getQiContextPtr()->getID();
    }

    void ClientImpl::connect(const std::string& masterAddress) {
      _masterClient.connect(masterAddress);
      _masterClient.registerMachine(_machineContext);
      _masterClient.registerEndpoint(_endpointContext);
      _isInitialized = _masterClient.isInitialized();
    }

    void ClientImpl::call(const std::string &signature,
      const qi::DataStream& callDef,
            qi::DataStream& result) {
        if (!_isInitialized) {
          throw( qi::transport::ConnectionException(
            "Initialization failed. All calls will fail."));
        }

        // will throw if service not found
        const std::string& serverEndpoint = xLocateService(signature);
        // will throw if unable to find or create client
        //CTAF: todo
        //boost::shared_ptr<TransportSocket> client = xGetServerClient(serverEndpoint);
        //std::string resultData;
        //client->send(callDef.str(), resultData);
        //result.str(resultData);
    }

//    boost::shared_ptr<TransportSocket> ClientImpl::xGetServerClient(const std::string& serverEndpoint) {
//      // get the relevant messaging client for the node that hosts the service

//      boost::shared_ptr<TransportSocket> tc = _serverClients.get(serverEndpoint);
//      if (tc != NULL)
//        return tc;

//      // create messaging client if needed ...
//      bool ok = xCreateServerClient(serverEndpoint);
//      if (ok) {
//        tc = _serverClients.get(serverEndpoint);
//      }
//      if (!ok || tc == NULL) {
//        qiLogError("qimessaging") << "Could not create client for server \"" << serverEndpoint
//                 << "\" Probable connection error. " << std::endl;
//        throw( qi::transport::ConnectionException(
//          "Could not create client for server. Probable connection error."));
//      }
//      return tc;
//    }

    bool ClientImpl::xCreateServerClient(const std::string& serverEndpoint) {
      //CTAF:todo
      //      boost::shared_ptr<TransportSocket> client(new TransportSocket(_masterClient.getQiContextPtr()->getTransportContext()));
//      bool ok = client->connect(serverEndpoint);
//      if (ok) {
//        _serverClients.insert(serverEndpoint, client);
//        qiLogDebug("qimessaging") << "Client \"" << _endpointContext.name
//                 << "\" connected to " << serverEndpoint << std::endl;
//      }
      //return ok;
    }

    const std::string& ClientImpl::xLocateService(const std::string& methodSignature) {
      //CTAF:todo
      return "";
      //        const std::string& serverEndpoint = _serviceCache.get(methodSignature);
//        if (!serverEndpoint.empty()) {
//          return serverEndpoint;
//        }

//        std::string tmpEndpoint;
//        try {
//          tmpEndpoint = _masterClient.locateService(methodSignature, _endpointContext);
//        } catch(const std::exception&) {
//          qiLogWarning("qimessaging") << "ServiceNotFoundException \"" << qi::signatureToString(methodSignature)
//                     << "\" Unable to contact master." << std::endl;
//          throw( qi::transport::ServiceNotFoundException(
//            "Unable to contact master."));
//        }

//        if (!tmpEndpoint.empty()) {
//          // cache located service pair
//          _serviceCache.insert(methodSignature, tmpEndpoint);
//          // return a const string ref address
//          return _serviceCache.get(methodSignature);
//        } else {
//          qiLogWarning("qimessaging") << "ServiceNotFoundException \"" << qi::signatureToString(methodSignature)
//                     << "\" Method not known to master." << std::endl;
//          throw( qi::transport::ServiceNotFoundException(
//            "Method not known to master."));
//        }

//        // never happens: we either return early or throw
//        return serverEndpoint;
    }
  }
}
