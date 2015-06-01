/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

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

void ObjectHost::onMessage(const qi::Message &msg, TransportSocketPtr socket)
{
  BoundAnyObject obj;
  {
    boost::recursive_mutex::scoped_lock lock(_mutex);
    ObjectMap::iterator it = _objectMap.find(msg.object());
    if (it == _objectMap.end())
    {
      qiLogDebug() << "Object id not found " << msg.object();
      return;
    }
    qiLogDebug() << "ObjectHost forwarding " << msg.address();
    // Keep ptr alive while message is being processed, even if removeObject is called
    obj = it->second;
  }
  obj->onMessage(msg, socket);
}

unsigned int ObjectHost::addObject(BoundAnyObject obj, StreamContext* remoteRef, unsigned int id)
{
  boost::recursive_mutex::scoped_lock lock(_mutex);
  if (!id)
    id = ++_nextId;
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
    qi::async<void>(boost::bind(&qi::detail::hold<BoundAnyObject>, obj));
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

Atomic<int> ObjectHost::_nextId(2);
}
