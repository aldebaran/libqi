/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/

#include <qi/messaging/src/publisher_impl.hpp>
#include <qi/transport/transport_publisher.hpp>
#include <qi/messaging/src/network/master_endpoint.hpp>
#include <qi/messaging/src/network/endpoints.hpp>
#include <qi/log.hpp>

namespace qi {
  namespace detail {

    PublisherImpl::PublisherImpl(const std::string& name, Context *ctx)
      : ImplBase(ctx),
        _publisherInitialized(false),
        _publisher(new qi::transport::TransportPublisher(_masterClient.getQiContextPtr()->getTransportContext()))
    {
      _endpointContext.type = PUBLISHER_ENDPOINT;
      _endpointContext.name = name;
      _endpointContext.contextID = _masterClient.getQiContextPtr()->getID();
    }

    void PublisherImpl::connect(const std::string& masterAddress) {
      _masterClient.connect(masterAddress);
      _masterClient.registerMachine(_machineContext);
      _isInitialized = _masterClient.isInitialized();
    }

      void PublisherImpl::xInitPublisher() {
        // prepare this publisher
        _endpointContext.port = _masterClient.getNewPort(_machineContext.machineID);
        std::vector<std::string> publishAddresses = getEndpoints(_endpointContext, _machineContext);

        if (! xBind(publishAddresses)) {
          qisError << "PublisherImpl::advertise Failed to bind publisher: " << _endpointContext.name << std::endl;
          return;
        }
        _masterClient.registerEndpoint(_endpointContext);
        _publisherInitialized = true;
      }

      void PublisherImpl::advertiseTopic(const std::string& topicSignature) {
        if (!_isInitialized) {
          qisError << "PublisherImpl::advertiseTopic Attempt to use uninitializes publisher" << std::endl;
          return;
        }
        if (! _publisherInitialized) {
          xInitPublisher();
        }

        bool exists = _masterClient.topicExists(topicSignature);
        if (exists) {
          qisError << "Attempt to publish on an existing topic " << topicSignature << std::endl;
          return;
        }
        _masterClient.registerTopic(topicSignature, _endpointContext);
      }

      void PublisherImpl::unadvertiseTopic(const std::string& topicSignature) {
        _masterClient.unregisterTopic(topicSignature, _endpointContext);
      }

    bool PublisherImpl::xBind(const std::vector<std::string>& publishAddresses) {
      try {
        _publisher->init();
        _publisher->bind(publishAddresses);
        _isInitialized = true;
      } catch(const std::exception& e) {
        _isInitialized = false;
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
      if (_publisherInitialized) {
        _masterClient.unregisterEndpoint(_endpointContext);
      }
    }

  }
}
