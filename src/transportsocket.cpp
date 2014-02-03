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

#include <boost/algorithm/string.hpp>

#include <qi/log.hpp>

#include "transportsocket.hpp"
#include "tcptransportsocket.hpp"

qiLogCategory("qimessaging.transportsocket");

namespace qi
{

  TransportContext::TransportContext()
  {
  }

  TransportContext::~TransportContext()
  {
  }

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

  void TransportContext::setCapability(const std::string& key, const AnyValue& value)
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
    // Process override from environment
    std::string capstring = qi::os::getenv("QI_TRANSPORT_CAPABILITIES");
    std::vector<std::string> caps;
    boost::algorithm::split(caps, capstring, boost::algorithm::is_any_of(":"));
    for (unsigned i=0; i<caps.size(); ++i)
    {
      const std::string& c = caps[i];
      if (c.empty())
        continue;
      size_t p = c.find_first_of("=");
      if (p == std::string::npos)
      {
        if (c[0] == '-')
          _defaultCapabilities->erase(c.substr(1, c.npos));
        else if (c[0] == '+')
          (*_defaultCapabilities)[c.substr(1, c.npos)] = AnyValue::from(true);
        else
          (*_defaultCapabilities)[c] = AnyValue::from(true);
      }
      else
        (*_defaultCapabilities)[c.substr(0, p)] = AnyValue::from(c.substr(p+1, c.npos));
    }
  }

  const CapabilityMap& TransportContext::defaultCapabilities()
  {
    QI_ONCE(initCapabilities());
    return *_defaultCapabilities;
  }
}

#ifdef _MSC_VER
# pragma warning( pop )
#endif
