#pragma once
/*
** Copyright (C) 2014 Aldebaran Robotics
** See COPYING for the license
*/

#ifndef _QI_MESSAGING_STREAMCONTEXT_HPP_
#define _QI_MESSAGING_STREAMCONTEXT_HPP_

#include <qi/api.hpp>
#include <qi/anyvalue.hpp>
#include <qi/type/metaobject.hpp>
#include <map>

namespace qi
{

using CapabilityMap = std::map<std::string, AnyValue>;

  namespace capabilityname
  {
    // Capability: A client socket has the capability to accept and
    // dispatch Type_Call messages (& friends).
    // If set, stream used to register a service to the SD can be reused
    // to communicate with said service, for instance.
    QI_API extern char const * const clientServerSocket;

    // Capability: Object serialization protocol supports the
    // caching of MetaObjects (binary protocol change).
    QI_API extern char const * const metaObjectCache;

    // Capability: remote ends support Message flags (flags in 'type' header field)
    QI_API extern char const * const messageFlags;

    // Capability: remote end supports call cancelations.
    QI_API extern char const * const remoteCancelableCalls;

    // Capability: Objects allow unique identification using Ptruid/ObjectUid.
    QI_API extern char const * const objectPtrUid;

    // Capability: ServiceDirectory may add relative endpoints to services to the list of endpoints
    // in service information.
    QI_API extern char const * const relativeEndpointUri;
  }

  /// State of the `RelativeEndpointsUri` capability.
  enum class RelativeEndpointsUriEnabled : bool
  {
    No = false,
    Yes = true,
  };

/** Store contextual data associated to one point-to-point point transport.
 *
 * Currently handles:
 * - A map of local and remote capabilities. Overload advertiseCapabilities() to
 *   perform the actual sending of local capabilities to the remote endpoint.
 * - A MetaObject cache so that any given MetaObject is sent in full only once
 *   for each transport stream.
 */
class QI_API StreamContext
{
public:
  StreamContext();
  virtual ~StreamContext();

  /// Set or update a local capability, and immediately advertise to the other end
  virtual void advertiseCapability(const std::string& key, const AnyValue& value);

  /** Set or update and advertise a set of local capabilities.
   * Implementer must either update _localCapabilityMap or overload localCapability().
   *
   */
  virtual void advertiseCapabilities(const CapabilityMap& map);

  /// Fetch remote capability (default: from local cache).
  virtual boost::optional<AnyValue> remoteCapability(const std::string& key) const;

  bool hasReceivedRemoteCapabilities() const;

  template<typename T>
  T remoteCapability(const std::string& key, const T& defaultValue) const;

  const CapabilityMap& remoteCapabilities() const;
  const CapabilityMap& localCapabilities() const;

  /// Fetch back what we advertised to the other end (default: local cache)
  virtual boost::optional<AnyValue> localCapability(const std::string& key) const;

  template<typename T>
  T localCapability(const std::string& key, const T& defaultValue) const;

  /** Return a value based on shared capability.
  * If not present on one side, returns void.
  * If present on both sides with same type, return the lesser of both values.
  * Otherwise, throws.
  */
  boost::optional<AnyValue> sharedCapability(const std::string& key) const;

  /// Similar to above, but replace error conditions by default value.
  template<typename T>
  T sharedCapability(const std::string& key, const T& defaultValue) const;

  /// Return (cacheUid, wasInserted)
  std::pair<unsigned int, bool> sendCacheSet(const MetaObject& mo);

  void receiveCacheSet(unsigned int uid, const MetaObject& mo);

  MetaObject receiveCacheGet(unsigned int uid) const;

  /// Default capabilities injected on all transports upon connection
  static const CapabilityMap& defaultCapabilities();


protected:
  qi::Atomic<int> _cacheNextId;
  // Protects all storage
  mutable boost::mutex  _contextMutex;

  CapabilityMap _remoteCapabilityMap; // remote capabilities we received
  CapabilityMap _localCapabilityMap; // memory of what we advertisedk

  using SendMetaObjectCache = std::map<MetaObject, unsigned int>;
  using ReceiveMetaObjectCache = std::map<unsigned int, MetaObject>;
  SendMetaObjectCache _sendMetaObjectCache;
  ReceiveMetaObjectCache _receiveMetaObjectCache;
};

template<typename T>
T StreamContext::remoteCapability(const std::string& key, const T& defaultValue) const
{
  boost::optional<AnyValue> v = remoteCapability(key);
  if (v)
    return v->to<T>();
  else
    return defaultValue;
}

template<typename T>
T StreamContext::localCapability(const std::string& key, const T& defaultValue) const
{
  boost::optional<AnyValue> v = localCapability(key);
  if (v)
    return v->to<T>();
  else
    return defaultValue;
}

template<typename T>
T StreamContext::sharedCapability(const std::string& key, const T& defaultValue) const
{
  try
  {
    T v1 = localCapability(key, defaultValue);
    T v2 = remoteCapability(key, defaultValue);
    qiLogDebug("qitype.capability") << "Share check compare: " << v1 <<' ' << v2;
    return std::min (v1, v2);
  }
  catch(const std::exception& e)
  {
    qiLogDebug("qitype.capability") << "Share check exception: " << e.what();
    return defaultValue;
  }
}

}

#endif
