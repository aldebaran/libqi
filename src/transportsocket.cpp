/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

// Disable "'this': used in base member initializer list"
#ifdef _MSC_VER
# pragma warning( push )
# pragma warning(disable: 4355)
#endif

#include <iostream>

#include <qi/log.hpp>

#include "transportsocket.hpp"
#include "tcptransportsocket.hpp"

qiLogCategory("qimessaging.transportsocket");

namespace qi
{

  TransportSocket::~TransportSocket()
  {
  }

  TransportSocketPtr makeTransportSocket(const std::string &protocol, qi::EventLoop *eventLoop) {
    TransportSocketPtr ret;

    if (protocol == "tcp")
    {
      return TcpTransportSocketPtr(new TcpTransportSocket(eventLoop, false));
    }
#ifdef WITH_SSL
    else if (protocol == "tcps")
    {
      return TcpTransportSocketPtr(new TcpTransportSocket(eventLoop, true));
    }
#endif
    else
    {
      qiLogError() << "Unrecognized protocol to create the TransportSocket: " << protocol;
      return ret;
    }
  }

  void TransportSocket::setCapability(const std::string& key, const AnyValue& value)
  {
    CapabilityMap cm;
    cm[key] = value;
    setCapabilities(cm);
  }

  static CapabilityMap* _defaultCapabilities;
  static void initCapabilities()
  {
    _defaultCapabilities  = new CapabilityMap();
    (*_defaultCapabilities)["ClientServerSocket"] = AnyValue::from(true);
  }

  const CapabilityMap& TransportSocket::defaultCapabilities()
  {
    QI_ONCE(initCapabilities());
    return *_defaultCapabilities;
  }
}

#ifdef _MSC_VER
# pragma warning( pop )
#endif
