/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QIMESSAGING_OBJECTHOST_HPP_
#define _QIMESSAGING_OBJECTHOST_HPP_

#include <map>

#include <boost/thread/mutex.hpp>

#include <qi/atomic.hpp>

#include <qitype/fwd.hpp>

#include "transportsocket.hpp"


namespace qi
{
  class Message;
  class BoundObject;
  typedef boost::shared_ptr<BoundObject> BoundObjectPtr;

  class ObjectHost
  {
  public:
    ObjectHost(unsigned int service);
    ~ObjectHost();
    void onMessage(const qi::Message &msg, TransportSocketPtr socket);
    unsigned int addObject(BoundObjectPtr obj, unsigned int objId = 0);
    void removeObject(unsigned int);
    unsigned int service() { return _service;}
    unsigned int nextId() { return ++_nextId;}
    qi::Signal<> onDestroy;
  protected:
    void clear();
  private:
    typedef std::map<unsigned int, BoundObjectPtr > ObjectMap;
    boost::recursive_mutex    _mutex;
    unsigned int    _service;
    ObjectMap       _objectMap;
    /* Be static for better readability
    */
    static qi::Atomic<int> _nextId;
  };
}

#endif
