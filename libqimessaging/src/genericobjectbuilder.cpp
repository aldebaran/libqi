/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/
#include <qimessaging/genericobjectbuilder.hpp>
#include <qimessaging/dynamicobject.hpp>
#include "metaobject_p.hpp"

namespace qi
{

  class GenericObjectBuilderPrivate
  {
  public:
    GenericObjectBuilderPrivate() : object(new DynamicObject())  {}
    ~GenericObjectBuilderPrivate() {}
    DynamicObject* object;
    MetaObject     metaObject;
  };

  GenericObjectBuilder::GenericObjectBuilder()
  {
    _p = new GenericObjectBuilderPrivate;
  }

  GenericObjectBuilder::~GenericObjectBuilder()
  {
    delete _p;
  }

  int GenericObjectBuilder::xAdvertiseMethod(const std::string &retsig, const std::string& signature, MetaCallable func)
  {
    unsigned int nextId = _p->metaObject._p->_methods.size()
      +  _p->metaObject._p->_events.size();
    MetaMethod mm(nextId, retsig, signature);
    _p->metaObject._p->_methods[mm.uid()] = mm;
    _p->object->setMethod(nextId, func);
    return nextId;
  }

  int GenericObjectBuilder::xAdvertiseEvent(const std::string& signature)
  {
    unsigned int nextId = _p->metaObject._p->_methods.size()
      +  _p->metaObject._p->_events.size();
    MetaSignal ms(nextId, signature);
    _p->metaObject._p->_events[nextId] = ms;
    return nextId;
  }

  GenericObject GenericObjectBuilder::object()
  {
    _p->metaObject._p->refreshCache();
    _p->object->setMetaObject(_p->metaObject);
    return makeDynamicObject(_p->object);
  }
}
