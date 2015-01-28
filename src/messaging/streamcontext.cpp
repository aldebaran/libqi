/*
**  Copyright (C) 2014 Aldebaran Robotics
**  See COPYING for the license
*/

#include <boost/algorithm/string.hpp>

#include "streamcontext.hpp"

namespace qi
{

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

boost::optional<AnyValue> StreamContext::remoteCapability(const std::string& key)
{
  boost::mutex::scoped_lock loc(_contextMutex);
  CapabilityMap::iterator it = _remoteCapabilityMap.find(key);
  if (it != _remoteCapabilityMap.end())
    return it->second;
  else
    return boost::optional<AnyValue>();
}

bool StreamContext::hasReceivedRemoteCapabilities() const
{
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

boost::optional<AnyValue> StreamContext::localCapability(const std::string& key)
{
  boost::mutex::scoped_lock loc(_contextMutex);
  CapabilityMap::iterator it = _localCapabilityMap.find(key);
  if (it != _localCapabilityMap.end())
    return it->second;
  else
    return boost::optional<AnyValue>();
}

MetaObject StreamContext::receiveCacheGet(unsigned int uid)
{
  // Return by value, as map is by value.
  boost::mutex::scoped_lock lock(_contextMutex);
  ReceiveMetaObjectCache::iterator it = _receiveMetaObjectCache.find(uid);
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

static CapabilityMap* _defaultCapabilities = 0;
static void initCapabilities()
{
  _defaultCapabilities  = new CapabilityMap();
  /* ClientServerSocket : A client socket has the capability to accept and
   * dispatch Type_Call messages (& friends).
   * If set, stream used to register a service to the SD can be reused
   * to communicate with said service, for instance.
   */
  (*_defaultCapabilities)["ClientServerSocket"] = AnyValue::from(true);
  /* MetaObjectCache: Object serialization protocol supports the
  * caching of MetaObjects (binary protocol change).
  */
  (*_defaultCapabilities)["MetaObjectCache"] = AnyValue::from(true);
  /* MessageFlags: remote ends support Message flags (flags in 'type' header field)
  */
  (*_defaultCapabilities)["MessageFlags"] = AnyValue::from(true);
  /* RemoteCancelableCalls: remote end supports call cancelations.
   */
  (*_defaultCapabilities)["RemoteCancelableCalls"] = AnyValue::from(true);
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
