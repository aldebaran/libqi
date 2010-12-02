#pragma once
/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/

#include <qi/messaging/detail/publisher_impl.hpp>
#include <qi/transport/detail/zmq/zmq_publisher.hpp>
#include <qi/transport/detail/network/master_endpoint.hpp>
#include <qi/transport/detail/network/endpoints.hpp>
#include <qi/log.hpp>

namespace qi {
  namespace detail {

    PublisherImpl::PublisherImpl(const std::string& name, const std::string& masterAddress) :
      MasterClient(name, masterAddress),
      _publisher(new qi::transport::ZMQPublisher()),
      _publisherInitialized(false)
    {
      _endpointContext.type = PUBLISHER_ENDPOINT;
      init();
      registerMachine(_machineContext);
    }

      void PublisherImpl::xInitPublisher() {
        if (! _publisherInitialized) {
          // prepare this publisher
          _endpointContext.port = getNewPort(_machineContext.machineID);
          std::vector<std::string> subscribeAddresses = getEndpoints(_endpointContext, _machineContext);

          if (! xBind(subscribeAddresses)) {
            qisError << "PublisherImpl::advertise Failed to bind publisher: " << _name << std::endl;
            return;
          }
          registerEndpoint(_endpointContext);
          _publisherInitialized = true;
        }
      }

      void PublisherImpl::advertiseTopic(const std::string& topicSignature) {
        if (! _publisherInitialized) {
          xInitPublisher();
        }

        bool exists = topicExists(topicSignature);
        if (exists) {
          qisError << "Attempt to publish on an existing topic " << topicSignature << std::endl;
          return;
        }
        registerTopic(topicSignature, _endpointContext);
      }

    bool PublisherImpl::xBind(const std::vector<std::string>& publishAddresses) {
      try {
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
        unregisterEndpoint(_endpointContext);
      }
    }

  }
}
