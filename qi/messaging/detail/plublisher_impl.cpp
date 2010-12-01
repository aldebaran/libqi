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

    PublisherImpl::PublisherImpl(const std::string& masterAddress) :
      _masterAddress(masterAddress),
      _publisher(new qi::transport::ZMQPublisher())
    {
      _endpointContext.type = PUBLISHER_ENDPOINT;
      _endpointContext.contextID = _qiContext.getID();
      xInit();
    }

    void PublisherImpl::xInit() {
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

    bool PublisherImpl::bind(const std::vector<std::string>& publishAddresses) {
      try {
        _publisher->bind(publishAddresses);
        xRegisterSelfWithMaster();
        _isInitialized = true;
      } catch(const std::exception& e) {
        qisDebug << "Publisher failed to create publisher for addresses" << std::endl;
        std::vector<std::string>::const_iterator it = publishAddresses.begin();
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
      xUnregisterSelfWithMaster();
    }

  }
}
