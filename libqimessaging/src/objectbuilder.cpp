/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <qimessaging/objectbuilder.hpp>
#include <boost/thread.hpp>
#include <qimessaging/object.hpp>
#include <qimessaging/dynamicobject.hpp>

#include "metaobject_p.hpp"

namespace qi {

  class StaticObjectBuilderPrivate
  {
  public:
    StaticObjectBuilderPrivate() : type(0), classType(0)  {}
    ObjectTypeData data;
    ObjectType*    type;
    Type*          classType;
    MetaObject     metaObject;
  };

  StaticObjectBuilder::StaticObjectBuilder()
  : _p(new StaticObjectBuilderPrivate())
  {
  }

  StaticObjectBuilder::~StaticObjectBuilder()
  {
    delete _p;
  }

  int StaticObjectBuilder::xAdvertiseMethod(const std::string &retsig, const std::string& signature, GenericMethod func)
  {
    unsigned int nextId = _p->metaObject._p->_methods.size()
      +  _p->metaObject._p->_events.size();
    MetaMethod mm(nextId, retsig, signature);
    _p->metaObject._p->_methods[mm.uid()] = mm;
    _p->data.methodMap[mm.uid()] = func;
    return nextId;
  }

  int StaticObjectBuilder::xAdvertiseEvent(const std::string& signature, SignalMemberGetter getter)
  {
    unsigned int nextId = _p->metaObject._p->_methods.size()
      +  _p->metaObject._p->_events.size();
    MetaSignal ms(nextId, signature);
    _p->metaObject._p->_events[nextId] = ms;
    _p->data.signalGetterMap[nextId] = getter;
    return nextId;
  }

  void StaticObjectBuilder::xBuildFor(Type* type)
  {
    _p->classType = type;
  }

  ObjectType* & StaticObjectBuilder::_type()
  {
    return _p->type;
  }
  void StaticObjectBuilder::_init()
  {
    _p->metaObject._p->refreshCache();
    dynamic_cast<StaticObjectTypeBase*>(_p->type)->initialize(_p->metaObject, _p->data);
  }

  const MetaObject& StaticObjectBuilder::metaObject()
  {
    _p->metaObject._p->refreshCache();
    return _p->metaObject;
  }

  class DynamicObjectBuilderPrivate
  {
  public:
    DynamicObjectBuilderPrivate() : object(new DynamicObject())  {}
    ~DynamicObjectBuilderPrivate() {}
    DynamicObject* object;
    MetaObject     metaObject;
  };

  DynamicObjectBuilder::DynamicObjectBuilder()
  {
    _p = new DynamicObjectBuilderPrivate;
  }

  DynamicObjectBuilder::~DynamicObjectBuilder()
  {
    delete _p;
  }

  int DynamicObjectBuilder::xAdvertiseMethod(const std::string &retsig, const std::string& signature, MetaCallable func)
  {
    unsigned int nextId = _p->metaObject._p->_methods.size()
      +  _p->metaObject._p->_events.size();
    MetaMethod mm(nextId, retsig, signature);
    _p->metaObject._p->_methods[mm.uid()] = mm;
    _p->object->setMethod(nextId, func);
    return nextId;
  }

  int DynamicObjectBuilder::xAdvertiseEvent(const std::string& signature)
  {
    unsigned int nextId = _p->metaObject._p->_methods.size()
      +  _p->metaObject._p->_events.size();
    MetaSignal ms(nextId, signature);
    _p->metaObject._p->_events[nextId] = ms;
    return nextId;
  }

  Object DynamicObjectBuilder::object()
  {
    _p->metaObject._p->refreshCache();
    _p->object->setMetaObject(_p->metaObject);
    return makeDynamicObject(_p->object);
  }

}
