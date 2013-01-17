/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#include "objecthost.hpp"

#include "boundobject.hpp"

namespace qi
{

ObjectHost::ObjectHost(unsigned int service)
 : _service(service)
 , _nextId(2)
 {}

 ObjectHost::~ObjectHost()
 {
   onDestroy();
 }

void ObjectHost::onMessage(const qi::Message &msg, TransportSocketPtr socket)
{
  ObjectMap::iterator it = _objectMap.find(msg.object());
  if (it == _objectMap.end())
  {
    qiLogDebug("qi.ObjectHost") << "Object id not found " << msg.object();
    return;
  }
  qiLogDebug("qi.ObjectHost") << "ObjectHost forwarding " << msg.address();
  // Keep ptr alive while message is being processed, even if removeObject is called
  BoundObjectPtr obj = it->second;
  obj->onMessage(msg, socket);
}

unsigned int ObjectHost::addObject(BoundObjectPtr obj, unsigned int id)
{
  if (!id)
    id = ++_nextId;
  _objectMap[id] = obj;
  return id;
}

void ObjectHost::removeObject(unsigned int id)
{
  _objectMap.erase(id);
}

}
