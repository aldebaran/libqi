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
    GenericObjectBuilderPrivate() : object(new DynamicObject())
    {}

    GenericObjectBuilderPrivate(DynamicObject *dynobject) : object(dynobject)
    {}

    ~GenericObjectBuilderPrivate()
    {}

    DynamicObject* object;
  };

  GenericObjectBuilder::GenericObjectBuilder()
    : _p(new GenericObjectBuilderPrivate)
  {
  }

  GenericObjectBuilder::GenericObjectBuilder(DynamicObject *dynobject)
    : _p(new GenericObjectBuilderPrivate(dynobject))
  {}

  GenericObjectBuilder::~GenericObjectBuilder()
  {
    delete _p;
  }

  int GenericObjectBuilder::xAdvertiseMethod(const std::string &retsig, const std::string& signature, MetaCallable func)
  {
    unsigned int nextId = _p->object->metaObject()._p->_methods.size() + _p->object->metaObject()._p->_events.size();
    MetaMethod mm(nextId, retsig, signature);
    _p->object->metaObject()._p->_methods[mm.uid()] = mm;
    _p->object->setMethod(nextId, func);
    _p->object->metaObject()._p->refreshCache();
    return nextId;
  }

  int GenericObjectBuilder::xAdvertiseEvent(const std::string& signature)
  {
    unsigned int nextId = _p->object->metaObject()._p->_methods.size() + _p->object->metaObject()._p->_events.size();
    MetaSignal ms(nextId, signature);
    _p->object->metaObject()._p->_events[nextId] = ms;
    _p->object->metaObject()._p->refreshCache();
    return nextId;
  }

  ObjectPtr GenericObjectBuilder::object()
  {
    return makeDynamicObjectPtr(_p->object);
  }
}
