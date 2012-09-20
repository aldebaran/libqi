/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/
#include <qimessaging/objecttypebuilder.hpp>
#include <boost/thread.hpp>
#include <qimessaging/genericobject.hpp>
#include "src/staticobjecttype.hpp"
#include "metaobject_p.hpp"

namespace qi {

  class ObjectTypeBuilderPrivate
  {
  public:
    ObjectTypeBuilderPrivate() : type(0), classType(0)  {}
    ObjectTypeData data;
    ObjectType*    type;
    Type*          classType;
    MetaObject     metaObject;
  };

  ObjectTypeBuilderBase::ObjectTypeBuilderBase()
  : _p(new ObjectTypeBuilderPrivate())
  {
  }

  ObjectTypeBuilderBase::~ObjectTypeBuilderBase()
  {
    delete _p;
  }

  int ObjectTypeBuilderBase::xAdvertiseMethod(const std::string &retsig, const std::string& signature, GenericMethod func)
  {
    unsigned int nextId = _p->metaObject._p->_methods.size()
      +  _p->metaObject._p->_events.size();
    MetaMethod mm(nextId, retsig, signature);
    _p->metaObject._p->_methods[mm.uid()] = mm;
    _p->data.methodMap[mm.uid()] = func;
    return nextId;
  }

  int ObjectTypeBuilderBase::xAdvertiseEvent(const std::string& signature, SignalMemberGetter getter)
  {
    unsigned int nextId = _p->metaObject._p->_methods.size()
      +  _p->metaObject._p->_events.size();
    MetaSignal ms(nextId, signature);
    _p->metaObject._p->_events[nextId] = ms;
    _p->data.signalGetterMap[nextId] = getter;
    return nextId;
  }

  void ObjectTypeBuilderBase::xBuildFor(Type* type, boost::function<Manageable* (void*)> asManageable)
  {
    _p->classType = type;
    _p->data.asManageable = asManageable;
    _p->data.typeInfo = const_cast<std::type_info*>(&type->info());
  }


  const MetaObject& ObjectTypeBuilderBase::metaObject()
  {
    _p->metaObject._p->refreshCache();
    return _p->metaObject;
  }

  GenericObject ObjectTypeBuilderBase::object(void* ptr)
  {
    GenericObject o;
    o.type = type();
    void** nptr = new void*;
    *nptr = ptr;
    o.value = nptr;
    return o;
  }

  ObjectType* ObjectTypeBuilderBase::type()
  {
    if (!_p->type)
    {
      StaticObjectTypeBase* t = new StaticObjectTypeBase();
      t->initialize(metaObject(), _p->data);
      _p->type = t;
    }
    return _p->type;
  }

  void ObjectTypeBuilderBase::inherits(Type* type, int offset)
  {
    std::vector<std::pair<Type*, int> >& p = _p->data.parentTypes;
    if (type != _p->classType && std::find(p.begin(), p.end(),
      std::make_pair(type, offset)) == p.end())
    {
      qiLogVerbose("qi.meta") << "Declaring inheritance "
      << _p->classType->infoString() << " <- " << type->infoString();
      p.push_back(std::make_pair(type, offset));
    }
  }
}


