/*
*  Author(s):
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/

#include <qi/messaging/src/subscriber_impl.hpp>
#include <qi/transport/transport_subscriber.hpp>
#include <qi/messaging/src/network/master_endpoint.hpp>
#include <qi/serialization/message.hpp>
#include <qi/log.hpp>

namespace qi {
  namespace detail {
    SubscriberImpl::SubscriberImpl(
      const std::string& name,
      const std::string& masterAddress) :
      MasterClient(name, masterAddress),
        _transportSubscriber(new qi::transport::TransportSubscriber())
    {
      _endpointContext.type = SUBSCRIBER_ENDPOINT;
      init();
      registerMachine(_machineContext);
      registerEndpoint(_endpointContext);
      _transportSubscriber->setSubscribeHandler(this);
    }

    SubscriberImpl::~SubscriberImpl() {
      unregisterEndpoint(_endpointContext);
    }

    void SubscriberImpl::subscribe(const std::string& topicName, qi::Functor* f) {
      std::string endpoint = locateTopic(topicName, _endpointContext);
      if (endpoint.empty()) {
        qisWarning << "Subscriber \"" << _endpointContext.name << "\": Topic not found \"" << topicName << "\"" << std::endl;
         return;
      }

      qisDebug << "SubscriberImpl::subscribe: storing callback for : " << topicName << std::endl;
      ServiceInfo si(topicName, f);
      _subscriberCallBacks.insert(topicName, si);

      if (! _subscribedEndpoints.exists(endpoint)) {
        xConnect(endpoint);
        if (_subscribedEndpoints.empty()) {
          _transportSubscriber->subscribe();
        }
        _subscribedEndpoints.insert(endpoint, endpoint);
      }
    }

    void SubscriberImpl::subscribeHandler(qi::transport::Buffer &requestMessage) {
      qisDebug << "SubscriberImpl::subscribeHandler called " << std::endl;
      qi::serialization::Message ser(requestMessage);
      std::string targetTopic;
      ser.readString(targetTopic);
      const ServiceInfo& si = _subscriberCallBacks.get(targetTopic);
      if (si.methodName.empty() || si.functor == NULL) {
        qisDebug << "SubscriberImpl::subscribeHandler: topic not found \"" << targetTopic << "\"" << std::endl;
      } else {
        qisDebug << "SubscriberImpl::subscribeHandler: found topic \"" << si.methodName << "\"" << std::endl;
        qi::serialization::Message sd;
        si.functor->call(ser, sd);
      }
    }

    bool SubscriberImpl::xConnect(const std::string& address) {
      try {
        qisDebug << "SubscriberImpl::xConnect: connecting to: " << address << std::endl;
        _transportSubscriber->connect(address);
        _isInitialized = true;
      } catch(const std::exception& e) {
        _isInitialized = false;
        qisDebug << "Subscriber failed to create subscriber for address \"" << address << "\" Reason: " << e.what() << std::endl;
      }
      return _isInitialized;
    }

  }
}
