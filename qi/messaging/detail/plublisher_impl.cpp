/*
** Author(s):
**  - Chris Kilner <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#include <qi/messaging/detail/publisher_impl.hpp>
#include <qi/transport/detail/zmq/zmq_publisher.hpp>
#include <qi/transport/detail/network/master_endpoint.hpp>
#include <qi/log.hpp>

namespace qi {
  namespace detail {

    PublisherImpl::PublisherImpl(const std::string& name, const std::string& masterAddress) :
      MasterClient(name, masterAddress),
      _publisher(new qi::transport::ZMQPublisher())
    {
      _endpointContext.type = PUBLISHER_ENDPOINT;
      init();
    }

      //bool ServerImpl::xTopicExists(const std::string& topicName) {
      //  static const std::string method("master.topicExists::b:s");
      //  qi::transport::Buffer               ret;
      //  qi::serialization::BinarySerializer msg;
      //  msg.writeString(method);
      //  msg.writeString(topicName);
      //  _transportClient.send(msg.str(), ret);
      //  qi::serialization::BinarySerializer retSer(ret);
      //  bool exists;
      //  retSer.readBool(exists);
      //  return exists;
      //}

      //boost::shared_ptr<qi::detail::PublisherImpl> ServerImpl::advertiseTopic(
      //  const std::string& topicName,
      //  const std::string& typeSignature)
      //{
      //  //boost::shared_ptr<qi::detail::PublisherImpl> pubImpl(new qi::detail::PublisherImpl(_masterAddress));
      //  bool exists = xTopicExists(topicName);
      //  if (exists) {
      //    qisError << "Attempt to publish on existing topic " << topicName << std::endl;
      //    return pubImpl;
      //  }
      //  //std::string subscribeAddress = xGetTopicSubscribeAddress(_endpointContext.contextID);
      //  int publisherPort = getNewPort(_machineContext.machineID);
      //  std::vector<std::string> v;
      //  v.push_back("tcp://127.0.0.1:6000");
      //  pubImpl->bind(v);
      //  //xRegisterPublisherWithMaster();
      //  //xRegisterTopicWithMaster();
      //  // pubImpl->bind(qi::detail::getEndpoints(_endpointContext, _machineContext));
      //  // contact master to get topic info
      //  //bool ok xAdvertiseTopicWithMaster("name");
      //  return pubImpl;
      //  //  TopicInfo topicInfo = call<TopicInfo>("getTopicInfo", "name");

      //  //  if (topicInfo.isPublished) {
      //  //    if (! topicInfo.isManyToMany) {
      //  //      // return dead publisher;
      //  //    } else (
      //  //      // manyToMany
      //  //      const std::string endpoint = getEndpoint(_endpointContext, topicInfo.publishEndpoint);
      //  //    PublisherImpl impl();
      //  //    impl.connect(endpoint);
      //  //    Publisher pub(impl);
      //  //    connect(endpoint)
      //  //  }
      //  //} else {
      //  //  // not Published
      //  //  if (isOneToMany) ( bind to getAddresses(getNewServicePort(contextID)) )
      //  //    registerTopic(TopicInfo);
      //  //  if isManyToMany
      //  //    connect( getEndpoint(publishPort) )
      //  //    registerPublisher(my)

      //}

    void PublisherImpl::advertise(const std::string& signature) {

      std::vector<std::string> v;
      v.push_back("tcp://127.0.0.1:6000");
      bind(v);
    }

    bool PublisherImpl::bind(const std::vector<std::string>& publishAddresses) {
      try {
        _publisher->bind(publishAddresses);
        registerEndpoint(_endpointContext);
        _isInitialized = true;
      } catch(const std::exception& e) {
        qisDebug << "Publisher failed to create publisher for addresses" << std::endl;
        std::vector<std::string>::const_iterator it =  publishAddresses.begin();
        std::vector<std::string>::const_iterator end = publishAddresses.end();
        for(; it != end; ++it) {
          qisDebug << *it << std::endl;
        }
        qisDebug << "Reason: " << e.what() << std::endl;
      }
      return _isInitialized;
    }

    void PublisherImpl::publish(const std::string& data)
    {
      if (! _isInitialized) {
        qisError << "Attempt to use an uninitialized publisher." << std::endl;
        return;
      }
      _publisher->publish(data);
    }

    PublisherImpl::~PublisherImpl() {
      unregisterEndpoint(_endpointContext);
    }

  }
}
