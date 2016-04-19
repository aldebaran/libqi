/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#include <qi/actor.hpp>
#include "objecthost.hpp"

#include "boundobject.hpp"

qiLogCategory("qimessaging.objecthost");

namespace qi
{

ObjectHost::ObjectHost(unsigned int service)
 : _service(service)
 {}

 ObjectHost::~ObjectHost()
 {
   onDestroy();
   // deleting our map will trigger calls to removeObject
   // so does clear() while iterating
   ObjectMap map;
   std::swap(map, _objectMap);
   map.clear();
 }

static void async_destroy_attempt(BoundAnyObject obj, Future<void> fut)
{
  fut.wait();
  obj.reset();
}

BoundAnyObject ObjectHost::recursiveFindObject(uint32_t objectId)
{
  boost::recursive_mutex::scoped_lock lock(_mutex);
  auto it = _objectMap.find(objectId);
  auto e = end(_objectMap);
  if (it != e)
  {
    return it->second;
  }
  // Object was not found, so search in the children.
  auto b = begin(_objectMap);
  while (b != e)
  {
    BoundObject* obj{b->second.get()};
    // Children are BoundObjects. Unfortunately, BoundObject has no common
    // ancestors with ObjectHost. Nevertheless, some children can indeed
    // be ObjectHost. There is no way to get this information but to
    // perform a dynamic cast. The overhead should be negligible, as
    // this case is rare.
    if (auto* host = dynamic_cast<ObjectHost*>(obj))
    {
      if (auto obj = host->recursiveFindObject(objectId))
      {
        return obj;
      }
    }
    ++b;
  }
  return {};
}

void ObjectHost::onMessage(const qi::Message &msg, TransportSocketPtr socket)
{
  BoundAnyObject obj{recursiveFindObject(msg.object())};
  if (!obj)
  {
    // Should we treat this as an error ? Returning without error is the
    // legacy behavior.
    return;
  }
  obj->onMessage(msg, socket);

  qi::Promise<void> destructPromise;
  qi::async(boost::bind(&async_destroy_attempt, obj, destructPromise.future()));
  obj.reset();
  destructPromise.setValue(0);
}

unsigned int ObjectHost::addObject(BoundAnyObject obj, StreamContext* remoteRef, unsigned int id)
{
  boost::recursive_mutex::scoped_lock lock(_mutex);
  if (!id)
    id = nextId();
  QI_ASSERT(_objectMap.find(id) == _objectMap.end());
  _objectMap[id] = obj;
  _remoteReferences[remoteRef].push_back(id);
  return id;
}

void ObjectHost::removeRemoteReferences(TransportSocketPtr socket)
{
  boost::recursive_mutex::scoped_lock lock(_mutex);

  RemoteReferencesMap::iterator it = _remoteReferences.find(socket.get());
  if (it == _remoteReferences.end())
    return;
  for (std::vector<unsigned int>::iterator vit = it->second.begin(), end = it->second.end();
       vit != end;
       ++vit)
    removeObject(*vit);
  _remoteReferences.erase(it);
}

void ObjectHost::removeObject(unsigned int id)
{
  /* Ensure we are not in the middle of iteration when
  *  removing our ref on BoundAnyObject.
  */
  BoundAnyObject obj;
  {
    boost::recursive_mutex::scoped_lock lock(_mutex);
    ObjectMap::iterator it = _objectMap.find(id);
    if (it == _objectMap.end())
    {
      qiLogDebug() << this << " No match in host for " << id;
      return;
    }
    obj = it->second;
    _objectMap.erase(it);
    qiLogDebug() << this << " count " << obj.use_count();
    qi::async(boost::bind(&qi::detail::hold<BoundAnyObject>, obj));
  }
  qiLogDebug() << this << " Object " << id << " removed.";
}

void ObjectHost::clear()
{
  boost::recursive_mutex::scoped_lock lock(_mutex);
  for (ObjectMap::iterator it = _objectMap.begin(); it != _objectMap.end(); ++it)
  {
    ServiceBoundObject* sbo = dynamic_cast<ServiceBoundObject*>(it->second.get());
    if (sbo)
      sbo->_owner = 0;
  }
  _objectMap.clear();
}

}
