#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _SRC_OBJECTHOST_HPP_
#define _SRC_OBJECTHOST_HPP_

#include <map>

#include <boost/thread/mutex.hpp>
#include <boost/shared_ptr.hpp>

#include <qi/atomic.hpp>

#include <qi/type/fwd.hpp>

#include "transportsocket.hpp"


namespace qi
{
  class Message;
  class BoundObject;
  class StreamContext;
  using BoundAnyObject = boost::shared_ptr<BoundObject>;

  class ObjectHost
  {
  public:
    ObjectHost(unsigned int service);
    virtual ~ObjectHost();
    void onMessage(const qi::Message &msg, TransportSocketPtr socket);
    unsigned int addObject(BoundAnyObject obj, StreamContext* remoteReferencer, unsigned int objId = 0);
    void removeObject(unsigned int);
    void removeRemoteReferences(TransportSocketPtr socket);
    unsigned int service() { return _service;}
    virtual unsigned int nextId() = 0;
    using ObjectMap = std::map<unsigned int, BoundAnyObject>;
    const ObjectMap& objects() const { return _objectMap; }
  protected:
    void clear();
  private:
    /// If an object follows a complex call path, e.g it is passed by argument to a service,
    /// then returned via a signal, and finally used to make a call, it is possible that the
    /// destination of the call (the "service") does not know directly the called object, but instead one of its
    /// (ObjectHost) children knows it.
    BoundAnyObject recursiveFindObject(uint32_t objectId);
    using RemoteReferencesMap = std::map<StreamContext*, std::vector<unsigned int>>;
    boost::recursive_mutex    _mutex;
    unsigned int    _service;
    ObjectMap       _objectMap;
    RemoteReferencesMap _remoteReferences;
  };
}

#endif  // _SRC_OBJECTHOST_HPP_
