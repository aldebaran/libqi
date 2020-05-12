/*
**  Copyright (C) 2014 Aldebaran Robotics
**  See COPYING for the license
*/

#include <boost/algorithm/string.hpp>

#include "streamcontext.hpp"

namespace qi
{
  namespace capabilityname
  {
    char const * const clientServerSocket    = "ClientServerSocket";
    char const * const metaObjectCache       = "MetaObjectCache";
    char const * const messageFlags          = "MessageFlags";
    char const * const remoteCancelableCalls = "RemoteCancelableCalls";
    char const * const objectPtrUid          = "ObjectPtrUID";
    char const * const relativeEndpointUri   = "RelativeEndpointURI";
  }


StreamContext::StreamContext()
{
  _localCapabilityMap = StreamContext::defaultCapabilities();
}

StreamContext::~StreamContext()
{
}

void StreamContext::advertiseCapability(const std::string& key, const AnyValue& value)
{
  _localCapabilityMap[key] = value;
}

void StreamContext::advertiseCapabilities(const CapabilityMap &map)
{
  _localCapabilityMap.insert(map.begin(), map.end());
}

boost::optional<AnyValue> StreamContext::remoteCapability(const std::string& key) const
{
  boost::mutex::scoped_lock loc(_contextMutex);
  const auto it = _remoteCapabilityMap.find(key);
  if (it != _remoteCapabilityMap.end())
    return it->second;
  else
    return {};
}

bool StreamContext::hasReceivedRemoteCapabilities() const
{
  boost::mutex::scoped_lock lock(_contextMutex);
  return _remoteCapabilityMap.size() != 0;
}


const CapabilityMap& StreamContext::remoteCapabilities() const
{
  return _remoteCapabilityMap;
}

const CapabilityMap& StreamContext::localCapabilities() const
{
  return _localCapabilityMap;
}

boost::optional<AnyValue> StreamContext::localCapability(const std::string& key) const
{
  boost::mutex::scoped_lock loc(_contextMutex);
  const auto it = _localCapabilityMap.find(key);
  if (it != _localCapabilityMap.end())
    return it->second;
  else
    return {};
}

MetaObject StreamContext::receiveCacheGet(unsigned int uid) const
{
  // Return by value, as map is by value.
  boost::mutex::scoped_lock lock(_contextMutex);
  const auto it = _receiveMetaObjectCache.find(uid);
  if (it == _receiveMetaObjectCache.end())
    throw std::runtime_error("MetaObject not found in cache");
  return it->second;
}

void StreamContext::receiveCacheSet(unsigned int uid, const MetaObject& mo)
{
  boost::mutex::scoped_lock lock(_contextMutex);
  _receiveMetaObjectCache[uid] = mo;
}

std::pair<unsigned int, bool> StreamContext::sendCacheSet(const MetaObject& mo)
{
  boost::mutex::scoped_lock lock(_contextMutex);
  SendMetaObjectCache::iterator it = _sendMetaObjectCache.find(mo);
  if (it == _sendMetaObjectCache.end())
  {
    unsigned int v = ++_cacheNextId;
    _sendMetaObjectCache[mo] = v;
    return std::make_pair(v, true);
  }
  else
    return std::make_pair(it->second, false);
}

static CapabilityMap* _defaultCapabilities = nullptr;
static void initCapabilities()
{
  static const CapabilityMap defaultCaps =
  { { capabilityname::clientServerSocket   , AnyValue::from(true)  }
  , { capabilityname::messageFlags         , AnyValue::from(true)  }
  , { capabilityname::metaObjectCache      , AnyValue::from(false) }
  , { capabilityname::remoteCancelableCalls, AnyValue::from(true)  }
  , { capabilityname::objectPtrUid         , AnyValue::from(true)  }
  , { capabilityname::relativeEndpointUri  , AnyValue::from(true)  }
  };

  _defaultCapabilities = new CapabilityMap(defaultCaps);

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

const CapabilityMap& StreamContext::defaultCapabilities()
{
  QI_ONCE(initCapabilities());
  return *_defaultCapabilities;
}

}
