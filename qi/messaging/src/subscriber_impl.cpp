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

    SubscriberImpl::SubscriberImpl(const std::string& name, qi::Context *ctx)
      : ImplBase(ctx),
        _transportSubscriber(new qi::transport::TransportSubscriber(_masterClient.getQiContextPtr()->getTransportContext()))
    {
      _endpointContext.type = SUBSCRIBER_ENDPOINT;
      _endpointContext.name = name;
      _endpointContext.contextID = _masterClient.getQiContextPtr()->getID();
    }

    void SubscriberImpl::connect(const std::string& masterAddress) {
      _masterClient.connect(masterAddress);
      _masterClient.registerMachine(_machineContext);
      _masterClient.registerEndpoint(_endpointContext);
      _transportSubscriber->setSubscribeHandler(this);
      _isInitialized = _masterClient.isInitialized();
    }

    SubscriberImpl::~SubscriberImpl() {
      _masterClient.unregisterEndpoint(_endpointContext);
    }

    void SubscriberImpl::reset(const std::string &name, Context *ctx)
    {
      //TODO
    }

    void SubscriberImpl::subscribe(const std::string& signature, qi::Functor* f) {
      if (!_isInitialized) {
        qisError << "Subscriber:subscribe \"" << signature  << "\": Attempt to use uninitialized subscriber \"" << _endpointContext.name << "\"" << std::endl;
        return;
      }
      std::string endpoint = _masterClient.locateTopic(signature, _endpointContext);
      if (endpoint.empty()) {
        qisWarning << "Subscriber \"" << _endpointContext.name << "\": Topic not found \"" << signature << "\"" << std::endl;
         return;
      }

      qisDebug << "SubscriberImpl::subscribe: storing callback for : " << signature << std::endl;
      ServiceInfo si(signature, f);
      _subscriberCallBacks.insert(signature, si);

      if (! _subscribedEndpoints.exists(endpoint)) {
        xConnect(endpoint);
        if (_subscribedEndpoints.empty()) {
          // hmm creates the thread
          // TODO move the thread out of the transport subscriber
          _transportSubscriber->subscribe();
        }
        _subscribedEndpoints.insert(endpoint, endpoint);
      }
    }

    void SubscriberImpl::unsubscribe(const std::string& signature) {
      _subscriberCallBacks.remove(signature);
      // TODO cleanup the transport subscriber if no longer needed
      // TODO tell the master that we have unsubscribed
    }

    void SubscriberImpl::subscribeHandler(qi::transport::Buffer &requestMessage) {
      qi::serialization::Message ser(requestMessage);
      std::string targetTopic;
      ser.readString(targetTopic);
      const ServiceInfo& si = _subscriberCallBacks.get(targetTopic);
      if (si.methodName.empty() || si.functor == NULL) {
        qisDebug << "SubscriberImpl::subscribeHandler: handler not found for \"" << targetTopic << "\"" << std::endl;
      } else {
        //qisDebug << "SubscriberImpl::subscribeHandler: found handler for \"" << targetTopic << "\"" << std::endl;
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
