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
 }

void ObjectHost::onMessage(const qi::Message &msg, TransportSocketPtr socket)
{
  BoundObjectPtr obj;
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

unsigned int ObjectHost::addObject(BoundObjectPtr obj, unsigned int id)
{
  boost::recursive_mutex::scoped_lock lock(_mutex);
  if (!id)
    id = ++_nextId;
  _objectMap[id] = obj;
  return id;
}

void ObjectHost::removeObject(unsigned int id)
{
  boost::recursive_mutex::scoped_lock lock(_mutex);
  _objectMap.erase(id);
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
