#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _SRC_OBJECTHOST_HPP_
#define _SRC_OBJECTHOST_HPP_

#include <map>

#include <boost/thread/mutex.hpp>

#include <qi/atomic.hpp>

#include <qi/type/fwd.hpp>

#include "transportsocket.hpp"


namespace qi
{
  class Message;
  class BoundObject;
  class StreamContext;
  typedef boost::shared_ptr<BoundObject> BoundAnyObject;

  class ObjectHost
  {
  public:
    ObjectHost(unsigned int service);
    ~ObjectHost();
    void onMessage(const qi::Message &msg, TransportSocketPtr socket);
    unsigned int addObject(BoundAnyObject obj, StreamContext* remoteReferencer, unsigned int objId = 0);
    void removeObject(unsigned int);
    void removeRemoteReferences(TransportSocketPtr socket);
    unsigned int service() { return _service;}
    unsigned int nextId() { return ++_nextId;}
    qi::Signal<> onDestroy;
  protected:
    void clear();
  private:
    typedef std::map<unsigned int, BoundAnyObject > ObjectMap;
    typedef std::map<StreamContext*, std::vector<unsigned int> > RemoteReferencesMap;
    boost::recursive_mutex    _mutex;
    unsigned int    _service;
    ObjectMap       _objectMap;
    RemoteReferencesMap _remoteReferences;
    /* Be static for better readability
    */
    static qi::Atomic<int> _nextId;
  };
}

#endif  // _SRC_OBJECTHOST_HPP_
