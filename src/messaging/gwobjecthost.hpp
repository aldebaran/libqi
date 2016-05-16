#pragma once
/*
** Copyright (C) 2014 Aldebaran Robotics
** See COPYING for the license
*/

#ifndef _SRC_MESSAGING_GWOBJECTHOST_HPP_
#define _SRC_MESSAGING_GWOBJECTHOST_HPP_

#include <map>
#include <utility>
#include <vector>
#include <list>

#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>

#include <qi/atomic.hpp>
#include <qi/type/metaobject.hpp>

using ServiceId = unsigned int;
using ObjectId = unsigned int;
using GwObjectId = unsigned int;
// This structure represent the address of the object within the client.
// The same object can have multiple, different addresses if the client passes it
// to multiple services.
struct ObjectAddress
{
  ServiceId service;
  ObjectId object;
  ObjectAddress()
  {
  }
  ObjectAddress(const ObjectAddress& o)
    : service(o.service)
    , object(o.object)
  {
  }
  ObjectAddress(ServiceId s, ObjectId o)
    : service(s)
    , object(o)
  {
  }
  bool operator==(const ObjectAddress& o) const
  {
    return o.service == service && o.object == object;
  }
  bool operator<(const ObjectAddress& o) const
  {
    return (service == o.service && object < o.object) || service < o.service;
  }
};

namespace qi
{
class MetaObject;
class Message;
class TransportSocket;
class GwTransaction;
using TransportSocketPtr = boost::shared_ptr<TransportSocket>;
using TransportSocketWeakPtr = boost::weak_ptr<TransportSocket>;

/// Describes the full address of an object.
struct FullObjectAddress
{
  /// The socket from which the object is originated.
  TransportSocketWeakPtr socket;

  /// The address of the object over the associated socket.
  ObjectAddress localAddress;
};
struct ObjectInfo
{
  TransportSocketPtr socket;
  ObjectAddress address;
};

class GwObjectHost
{
public:
  ~GwObjectHost();

  void treatMessage(GwTransaction& msg, TransportSocketPtr sender, qi::TransportSocketPtr destination);
  void serviceDisconnected(ServiceId);
  void clientDisconnected(TransportSocketPtr);

  void harvestClientReplyOriginatingObjects(Message& msg, TransportSocketPtr sender, GwObjectId id, TransportSocketPtr destination);
  void harvestClientCallOriginatingObjects(Message& msg, TransportSocketPtr sender, TransportSocketPtr destination);
  void harvestServiceOriginatingObjects(Message& msg, TransportSocketPtr sender, qi::TransportSocketPtr destination);
  void harvestMessageObjects(Message& msg, TransportSocketPtr sender, qi::TransportSocketPtr destination);

  ObjectAddress getOriginalObjectAddress(const ObjectAddress& gwObjectAddress);
  ObjectInfo objectSource(const ObjectAddress&  address);

  /// Use this method to remember to which socket an object was transmitted.
  /// @param destination The destination socket.
  /// @param origin The origin socket.
  /// @param objectAddress The local address of the object over the origin socket.
  void setDestinationSocketForObject(
      const TransportSocketPtr& destination,
      const TransportSocketPtr& origin,
      const ObjectAddress& objectAddress);

private:
  void assignClientMessageObjectsGwIds(const Signature& sig, Message& msg, TransportSocketPtr sender, qi::TransportSocketPtr destination);

  boost::shared_mutex _mutex;

  // Objects originating from services
  // TODO OPTI: stocker une methodmap a la place du metaobject.
  // Retirer les methodes de la map quand elles sont appelees et qu'il y a pas d'object dedans.
  // -> devrait faire gagner du temps
  std::map<ServiceId, std::map<ObjectId, MetaObject> > _servicesMetaObjects;
  // All objects that a service uses - needed so that they can be invalidated if the
  // service goes offline.
  std::map<ServiceId, std::list<GwObjectId> > _objectsUsedOnServices;

  // Objects originating from clients
  std::map<GwObjectId, MetaObject> _objectsMetaObjects;


  // Tracks the connection between a GWObjectId and a client + messageaddress
  std::map<GwObjectId, ObjectInfo> _objectsOrigin;

  /// The origin of objects that are transmitted to clients are remembered here.
  std::map<TransportSocketPtr, std::list<FullObjectAddress>> _objectOriginsPerDestination;

  std::map<TransportSocketPtr, std::map<ObjectAddress, GwObjectId> > _hostObjectBank;

  MetaObject* findMetaObject(ServiceId serviceId, ObjectId objectId);
};
}

#endif
