/*
** Author(s):
**  - Chris Kilner <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#include <qi/messaging/detail/server_impl.hpp>
#include <string>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <qi/messaging/detail/get_protocol.hpp>
#include <qi/serialization/serializer.hpp>
#include <qi/transport/buffer.hpp>
#include <qi/log.hpp>
#include <cstdio>

namespace qi {
  using qi::serialization::SerializedData;

  namespace detail {

    ServerImpl::ServerImpl() : _isInitialized(false) {}

    ServerImpl::~ServerImpl() {
      if (!_isMasterServer) {
        xUnregisterSelfWithMaster();
      }
    }

    ServerImpl::ServerImpl(
      const std::string serverName,
      const std::string masterAddress) :
        _isInitialized(false),
        _isMasterServer(false),
        _name(serverName)
    {
      _endpointContext.name = serverName;
      _endpointContext.contextID = _qiContext.getID();

      if (_name == "master") {
        // we are the master's server, so we don't need a client to ourselves
        _isMasterServer = true;
        _isInitialized = true;
        _port = 5555;
      } else {
        _isInitialized = _transportClient.connect(std::string("tcp://") + masterAddress);
        if (! _isInitialized ) {
          qisError << "\"" << _name << "\" could not connect to master at address \"" << masterAddress << "\"" << std::endl;
          return;
        }
        // should really be split into two phases: getPort / register (once active)
        xRegisterSelfWithMaster();
      }

      // Ask the transport server to bind on all our addresses ----------------------------------------
      std::vector<std::string> addresses;
      char port[10];
      sprintf(port, "%d", _port);
      addresses.push_back(std::string("inproc://127.0.0.1:")                                   + port);
      addresses.push_back(std::string("ipc://127.0.0.1:")                                      + port);
      addresses.push_back(std::string("tcp://127.0.0.1:")                                      + port);
      addresses.push_back(std::string("tcp://") + _endpointContext.publicIP + std::string(":") + port);
      _transportServer.serve(addresses);
      // ----------------------------------------------------------------------------------------------

      _address = std::string("127.0.0.1:") + port; // tmp
      _transportServer.setMessageHandler(this);
      boost::thread serverThread(boost::bind(&qi::transport::Server::run, _transportServer));
    }

    bool ServerImpl::isInitialized() const {
      return _isInitialized;
    }

    void ServerImpl::messageHandler(std::string& defData, std::string& resultData) {
      // handle message
      SerializedData def(defData);
      SerializedData result(resultData);
      std::string methodSignature;
      def.readString(methodSignature);
      const ServiceInfo& si = xGetService(methodSignature);
      if (si.methodName.empty() || !si.functor) {
        qisError << "  Error: Method not found " << methodSignature << std::endl;
        return;
      }
      si.functor->call(def, result);
      resultData = result.str();
    }

    const std::string& ServerImpl::getName() const {
      return _name;
    }

    void ServerImpl::addService(const std::string& methodSignature, qi::Functor* functor) {
      ServiceInfo service(methodSignature, functor);
      //std::cout << "Added Service" << hash << std::endl;
      _localServices.insert(methodSignature, service);
      if (!_isMasterServer) {
        xRegisterServiceWithMaster(methodSignature);
      }
    }

    const ServiceInfo& ServerImpl::xGetService(
      const std::string& methodSignature) {
      // functors ... should be found here
      return _localServices.get(methodSignature);
    }

    void ServerImpl::xRegisterServiceWithMaster(const std::string& methodSignature) {
      static const std::string method("master.registerService::v:ss");
      qi::transport::Buffer               ret;
      qi::serialization::BinarySerializer msg;

      msg.writeString(method);
      msg.writeString(_address);
      msg.writeString(methodSignature);
      _transportClient.send(msg.str(), ret);
    }

    void ServerImpl::xRegisterSelfWithMaster() {
      static const std::string method("master.registerServer::i:ssssis");
      qi::transport::Buffer               ret;
      qi::serialization::BinarySerializer msg;

      msg.writeString(method);
      msg.writeString(_endpointContext.name);
      msg.writeString(_endpointContext.endpointID);
      msg.writeString(_endpointContext.contextID);
      msg.writeString(_endpointContext.machineID);
      msg.writeInt(_endpointContext.platformID);
      msg.writeString(_endpointContext.publicIP);

      _transportClient.send(msg.str(), ret);
      qi::serialization::BinarySerializer retSer(ret);
      retSer.readInt(_port);
      //ret.readInt(_endpointContext.port);
    }

    void ServerImpl::xUnregisterSelfWithMaster() {
      static const std::string method("master.unregisterServer::v:s");
      qi::transport::Buffer               ret;
      qi::serialization::BinarySerializer msg;

      msg.writeString(method);
      msg.writeString(_endpointContext.endpointID);
      _transportClient.send(msg.str(), ret);
    }
  }
}
