/*
** Author(s):
**  - Chris Kilner <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#include <qi/messaging/detail/subscriber_impl.hpp>
#include <qi/transport/detail/zmq/zmq_subscriber.hpp>
#include <qi/transport/detail/network/master_endpoint.hpp>
#include <qi/log.hpp>

namespace qi {
  namespace detail {
    SubscriberImpl::SubscriberImpl(const std::string& masterAddress) :
      ClientImplBase(masterAddress),
      _subscriber(new qi::transport::ZMQSubscriber())
    {
      _endpointContext.type = SUBSCRIBER_ENDPOINT;
      _endpointContext.contextID = _qiContext.getID();
      xInit();
    }

    void SubscriberImpl::xInit() {
      std::pair<std::string, int> masterEndpointAndPort;
      if (!qi::detail::validateMasterEndpoint(_masterAddress, masterEndpointAndPort)) {
        _isInitialized = false;
        qisError << "Publisher initialized with invalid master address: \""
          << _masterAddress << "\" All calls will fail." << std::endl;
        return;
      }

      _isInitialized = _transportClient.connect(masterEndpointAndPort.first);
      if (! _isInitialized ) {
        qisError << "Publisher could not connect to master "
          "at address \"" << masterEndpointAndPort.first << "\""
          << std::endl;
        return;
      }
      //xRegisterMachineWithMaster();
      //xRegisterSelfWithMaster();
    }


    SubscriberImpl::~SubscriberImpl() {
      xUnregisterSelfWithMaster();
    }

    bool SubscriberImpl::connect(const std::string& address) {
      try {
        _subscriber->connect(address);
        xRegisterSelfWithMaster();
        _subscriber->subscribe();
        _isInitialized = true;
      } catch(const std::exception& e) {
        qisDebug << "Subscriber failed to create subscriber for address \"" << address << "\" Reason: " << e.what() << std::endl;
      }
      return _isInitialized;
    }

    void SubscriberImpl::setSubscribeHandler(qi::transport::SubscribeHandler* callback) {
      _subscriber->setSubscribeHandler(callback);
    }

    qi::transport::SubscribeHandler* SubscriberImpl::getSubscribeHandler() const {
      return _subscriber->getSubscribeHandler();
    }
  }
}
