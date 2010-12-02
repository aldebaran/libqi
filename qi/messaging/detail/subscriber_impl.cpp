/*
** Author(s):
**  - Chris Kilner <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#include <qi/messaging/detail/subscriber_impl.hpp>
#include <qi/transport/detail/zmq/zmq_subscriber.hpp>
#include <qi/transport/detail/network/master_endpoint.hpp>
#include <qi/serialization/serializeddata.hpp>
#include <qi/log.hpp>

namespace qi {
  namespace detail {
    SubscriberImpl::SubscriberImpl(
      const std::string& name,
      const std::string& masterAddress) :
      MasterClient(name, masterAddress),
      _transportSubscriber(new qi::transport::ZMQSubscriber())
    {
      _endpointContext.type = SUBSCRIBER_ENDPOINT;
      init();
      _transportSubscriber->setSubscribeHandler(this);
    }

    SubscriberImpl::~SubscriberImpl() {
      unregisterEndpoint(_endpointContext);
    }

    void SubscriberImpl::subscribe(const std::string& topicName, qi::Functor* f) {
      qisDebug << "SubscriberImpl::subscribe: inserting: " << topicName << std::endl;
      ServiceInfo si(topicName, f);

      // endpoint = locateTopic(topicName, _endpointContext.endpointID);
      // if (endpoint.empty()) {
      //    not found
      //    return;
      // }
      // if (!_knownPublisherEndpoints.exist(endpoint) {
      //   _transportPublisher.connect(endpoint);
      // }

      _subscriberCallBacks.insert(topicName, si);

      // need subscriber for each publisher, or need to connect to each
      //SubscriberImpl subImpl(_masterAddress);
      xConnect("tcp://127.0.0.1:6000");
      //_transportSubscriber->setSubscribeHandler(this);
      // Create Subscriber to publisher ...
    }

    void SubscriberImpl::subscribeHandler(qi::transport::Buffer &requestMessage) {
      qisDebug << "SubscriberImpl::subscribeHandler called " << std::endl;
      qi::serialization::BinarySerializer ser(requestMessage);
      std::string targetTopic;
      ser.readString(targetTopic);
      const ServiceInfo& si = _subscriberCallBacks.get(targetTopic);
      if (si.methodName.empty() || si.functor == NULL) {
        qisDebug << "SubscriberImpl::subscribeHandler: topic not found \"" << targetTopic << "\"" << std::endl;
      } else {
        qisDebug << "SubscriberImpl::subscribeHandler: found topic \"" << si.methodName << "\"" << std::endl;
        qi::serialization::SerializedData sd;
        si.functor->call(ser, sd);
      }
    }

    bool SubscriberImpl::xConnect(const std::string& address) {
      try {
        _transportSubscriber->connect(address);
        registerEndpoint(_endpointContext);
        _transportSubscriber->subscribe();
        _isInitialized = true;
      } catch(const std::exception& e) {
        qisDebug << "Subscriber failed to create subscriber for address \"" << address << "\" Reason: " << e.what() << std::endl;
      }
      return _isInitialized;
    }

  }
}
