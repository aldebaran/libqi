/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <qimessaging/objectbuilder.hpp>
#include <boost/thread.hpp>
#include <qimessaging/object.hpp>
#include "object_p.hpp"
#include "metasignal_p.hpp"
#include "objectbuilder_p.hpp"
#include "metaobject_p.hpp"

namespace qi {

  ObjectBuilder::ObjectBuilder()
    : _p(new ObjectBuilderPrivate())
  {
  }

  ObjectBuilder::~ObjectBuilder()
  {
    delete _p;
  }

  int ObjectBuilder::xForgetMethod(const std::string &meth)
  {
    boost::recursive_mutex::scoped_lock sl(_p->_metaObject._p->_mutexMethod);
    std::map<std::string, unsigned int>::iterator it;

    it = _p->_metaObject._p->_methodsNameToIdx.find(meth);
    if (it != _p->_metaObject._p->_methodsNameToIdx.end())
    {
      _p->_metaObject._p->_methodsNameToIdx.erase(it);
      return (0);
    }

    return (1);
  }

  int ObjectBuilder::xAdvertiseMethod(const std::string &sigret, const std::string& signature, MetaFunction functor) {
    boost::recursive_mutex::scoped_lock sl(_p->_metaObject._p->_mutexMethod);

    std::map<std::string, unsigned int>::iterator it;
    it = _p->_metaObject._p->_methodsNameToIdx.find(signature);
    if (it != _p->_metaObject._p->_methodsNameToIdx.end())
    {
      unsigned int uid = it->second;
      MetaMethod mm(uid, sigret, signature, functor);
      // find it
      _p->_metaObject._p->_methods[uid] = mm;
      qiLogVerbose("qi.Object") << "rebinding method:" << signature;
      return uid;
    }
    unsigned int idx = _p->_metaObject._p->_nextNumber++;
    MetaMethod mm(idx,sigret, signature, functor);
    _p->_metaObject._p->_methods[idx] = mm;
    _p->_metaObject._p->_methodsNameToIdx[signature] = idx;
    qiLogVerbose("qi.Object") << "binding method:" << signature;
    return idx;
  }

  int ObjectBuilder::xAdvertiseEvent(const std::string& signature) {
    boost::recursive_mutex::scoped_lock sl(_p->_metaObject._p->_mutexEvent);
    if (signature.empty())
    {
      qiLogError("qi.Object") << "Event has empty signature.";
      return -1;
    }
    std::map<std::string, unsigned int>::iterator it;

    it = _p->_metaObject._p->_eventsNameToIdx.find(signature);
    if (it != _p->_metaObject._p->_eventsNameToIdx.end())
    { // Event already there.
      qiLogError("qi.Object") << "event already there";
      return it->second;
    }
    unsigned int idx = _p->_metaObject._p->_nextNumber++;
    MetaSignal me(signature);
    me._p->_uid = idx;
    _p->_metaObject._p->_events[idx] = me;
    _p->_metaObject._p->_eventsNameToIdx[signature] = idx;
    qiLogVerbose("qi.Object") << "binding event:" << signature <<" with id "
    << idx;
    return idx;
  }

  qi::Object ObjectBuilder::object() {
    qi::Object obj(_p->_metaObject);
    return obj;
  }

}
