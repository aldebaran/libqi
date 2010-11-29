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
#include <qi/serialization/serializer.hpp>
#include <qi/transport/buffer.hpp>
#include <qi/transport/detail/network/endpoints.hpp>
#include <qi/transport/detail/network/master_endpoint.hpp>
#include <qi/log.hpp>
#include <qi/exceptions/exceptions.hpp>
#include <qi/messaging/detail/publisher_impl.hpp>


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

      std::pair<std::string, int> masterEndpointAndPort;
      if (!qi::detail::validateMasterEndpoint(masterAddress, masterEndpointAndPort)) {
        _isInitialized = false;
        qisError << "\"" << _name << "\" initialized with invalid master "
          "address: \"" << masterAddress << "\" All calls will fail."
          << std::endl;
        return;
      }

      if (_name == "master") {
        // we are the master's server, so we don't need a client to ourselves
        _isMasterServer = true;
        _isInitialized = true;
        _endpointContext.port = masterEndpointAndPort.second;
      } else {
        _isInitialized = _transportClient.connect(masterEndpointAndPort.first);
        if (! _isInitialized ) {
          qisError << "\"" << _name << "\" could not connect to master "
            "at address \"" << masterEndpointAndPort.first << "\""
            << std::endl;
          return;
        }
        _endpointContext.port = xGetNewPortFromMaster(_endpointContext.machineID);
        xRegisterMachineWithMaster();
        xRegisterSelfWithMaster();
      }

      _transportServer.serve(qi::detail::getEndpoints(_endpointContext, _machineContext));

      _transportServer.setMessageHandler(this);
      boost::thread serverThread(
        ::boost::bind(&qi::transport::Server::run, _transportServer));
    }

    bool ServerImpl::isInitialized() const {
      return _isInitialized;
    }

    const qi::detail::MachineContext& ServerImpl::getMachineContext() const {
      return _machineContext;
    }

    const qi::detail::EndpointContext& ServerImpl::getEndpointContext() const {
      return _endpointContext;
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

    void ServerImpl::addService(
      const std::string& methodSignature,
      qi::Functor* functor)
    {
      if (! _isInitialized ) {
        qisError << "Attempt to use uninitialized server: \"" << _name <<
          "\". Service \"" << methodSignature << "\" not added."
          << std::endl;
        throw qi::transport::ServerException(
          std::string("Attempt to use uninitialized server: \"") +
          _name + "\". Service not added.");
        return;
      }
      ServiceInfo service(methodSignature, functor);
      //std::cout << "Added Service" << hash << std::endl;
      _localServices.insert(methodSignature, service);
      if (!_isMasterServer) {
        xRegisterServiceWithMaster(methodSignature);
      }
    }

    const ServiceInfo& ServerImpl::xGetService(
      const std::string& methodSignature)
    {
      // functors ... should be found here
      return _localServices.get(methodSignature);
    }

    void ServerImpl::xRegisterServiceWithMaster(
      const std::string& methodSignature)
    {
      if (!_isInitialized) {
        return;
      }
      static const std::string method("master.registerService::v:ss");
      qi::transport::Buffer               ret;
      qi::serialization::BinarySerializer msg;

      msg.writeString(method);
      msg.writeString(methodSignature);
      msg.writeString(_endpointContext.endpointID);
      _transportClient.send(msg.str(), ret);
    }


    int ServerImpl::xGetNewPortFromMaster(const std::string& machineID) {
      static const std::string method("master.getNewPort::i:s");
      qi::transport::Buffer               ret;
      qi::serialization::BinarySerializer msg;

      msg.writeString(method);
      msg.writeString(machineID);
      _transportClient.send(msg.str(), ret);
      qi::serialization::BinarySerializer retSer(ret);
      int port;
      retSer.readInt(port);
      return port;
    }

    void ServerImpl::xRegisterMachineWithMaster() {
      static const std::string method = "master.registerMachine::v:sssi";
      qi::transport::Buffer               ret;
      qi::serialization::BinarySerializer msg;
      msg.writeString(method);
      msg.writeString(_machineContext.machineID);
      msg.writeString(_machineContext.hostName);
      msg.writeString(_machineContext.publicIP);
      msg.writeInt(   _machineContext.platformID);
      _transportClient.send(msg.str(), ret);
    }

    void ServerImpl::xRegisterSelfWithMaster() {
      static const std::string method("master.registerServer::v:ssssi");
      qi::transport::Buffer               ret;
      qi::serialization::BinarySerializer msg;

      msg.writeString(method);
      msg.writeString(_endpointContext.name);
      msg.writeString(_endpointContext.endpointID);
      msg.writeString(_endpointContext.contextID);
      msg.writeString(_endpointContext.machineID);
      msg.writeInt(   _endpointContext.port);
      _transportClient.send(msg.str(), ret);
    }

    void ServerImpl::xUnregisterSelfWithMaster() {
      if (!_isInitialized) {
        return;
      }
      static const std::string method("master.unregisterServer::v:s");
      qi::transport::Buffer               ret;
      qi::serialization::BinarySerializer msg;

      msg.writeString(method);
      msg.writeString(_endpointContext.endpointID);
      _transportClient.send(msg.str(), ret);
    }


    boost::shared_ptr<qi::detail::PublisherImpl> ServerImpl::advertiseTopic(
      const std::string& topicName,
      const std::string& typeSignature)
    {
      boost::shared_ptr<qi::detail::PublisherImpl> pubImpl(new qi::detail::PublisherImpl());
      // contact master to get topic info
      return pubImpl;
    //  TopicInfo topicInfo = call<TopicInfo>("getTopicInfo", "name");

    //  if (topicInfo.isPublished) {
    //    if (! topicInfo.isManyToMany) {
    //      // return dead publisher;
    //    } else (
    //      // manyToMany
    //      const std::string endpoint = getEndpoint(_endpointContext, topicInfo.publishEndpoint);
    //    PublisherImpl impl();
    //    impl.connect(endpoint);
    //    Publisher pub(impl);
    //    connect(endpoint)
    //  }
    //} else {
    //  // not Published
    //  if (isOneToMany) ( bind to getAddresses(getNewServicePort(contextID)) )
    //    registerTopic(TopicInfo);
    //  if isManyToMany
    //    connect( getEndpoint(publishPort) )
    //    registerPublisher(my)

    }
  }
}
