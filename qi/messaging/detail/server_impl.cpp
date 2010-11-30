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

    ServerImpl::ServerImpl() {}

    ServerImpl::~ServerImpl() {
      if (!_isMasterServer) {
        xUnregisterSelfWithMaster();
      }
    }

    ServerImpl::ServerImpl(
      const std::string serverName,
      const std::string masterAddress) :
        _isMasterServer(false),
        _name(serverName)
    {
      _endpointContext.type = SERVER_ENDPOINT;
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


    bool ServerImpl::xTopicExists(const std::string& topicName) {
      static const std::string method("master.topicExists::b:s");
      qi::transport::Buffer               ret;
      qi::serialization::BinarySerializer msg;
      msg.writeString(method);
      msg.writeString(topicName);
      _transportClient.send(msg.str(), ret);
      qi::serialization::BinarySerializer retSer(ret);
      bool exists;
      retSer.readBool(exists);
      return exists;
    }

    //void ServerImpl::xRegisterPublisherWithMaster(const EndpointContext& publisherContext) {
    //  //
    //}

    //void xRegisterTopicWithMaster() {

    //}

    boost::shared_ptr<qi::detail::PublisherImpl> ServerImpl::advertiseTopic(
      const std::string& topicName,
      const std::string& typeSignature)
    {
      boost::shared_ptr<qi::detail::PublisherImpl> pubImpl(new qi::detail::PublisherImpl());
      bool exists = xTopicExists(topicName);
      if (exists) {
        qisError << "Attempt to publish on existing topic " << topicName << std::endl;
        return pubImpl;
      }
      //std::string subscribeAddress = xGetTopicSubscribeAddress(_endpointContext.contextID);
      int publisherPort = xGetNewPortFromMaster(_machineContext.machineID);
      //xRegisterPublisherWithMaster();
      //xRegisterTopicWithMaster();
      // pubImpl->bind(qi::detail::getEndpoints(_endpointContext, _machineContext));
      // contact master to get topic info
      //bool ok xAdvertiseTopicWithMaster("name");
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
