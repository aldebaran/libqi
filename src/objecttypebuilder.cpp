/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/
#include <qitype/objecttypebuilder.hpp>
#include <boost/thread.hpp>
#include <qitype/genericobject.hpp>
#include "staticobjecttype.hpp"
#include "metaobject_p.hpp"

namespace qi {

  class ObjectTypeBuilderPrivate
  {
  public:
    ObjectTypeBuilderPrivate() : type(0)  {}
    ObjectTypeData data;
    ObjectType*    type;
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

  int ObjectTypeBuilderBase::xAdvertiseMethod(const std::string &retsig, const std::string& signature, GenericMethod func, int id)
  {
    unsigned int nextId = _p->metaObject._p->addMethod(retsig, signature, id);
    _p->data.methodMap[nextId] = func;
    return nextId;
  }

  int ObjectTypeBuilderBase::xAdvertiseEvent(const std::string& signature, SignalMemberGetter getter, int id)
  {
    unsigned int nextId = _p->metaObject._p->addSignal(signature, id);
    _p->data.signalGetterMap[nextId] = getter;
    return nextId;
  }

  void ObjectTypeBuilderBase::xBuildFor(Type* type, boost::function<Manageable* (void*)> asManageable)
  {
    _p->data.asManageable = asManageable;
    _p->data.classType = type;
  }


  const MetaObject& ObjectTypeBuilderBase::metaObject()
  {
    _p->metaObject._p->refreshCache();
    return _p->metaObject;
  }

  ObjectPtr ObjectTypeBuilderBase::object(void* ptr)
  {
    ObjectPtr ret = ObjectPtr(new GenericObject(type(), ptr));
    return ret;
  }

  ObjectType* ObjectTypeBuilderBase::type()
  {
    if (!_p->type)
    {
      StaticObjectTypeBase* t = new StaticObjectTypeBase();
      t->initialize(metaObject(), _p->data);
      _p->type = t;
      registerType();
    }
    return _p->type;
  }

  void ObjectTypeBuilderBase::inherits(Type* type, int offset)
  {
    std::vector<std::pair<Type*, int> >& p = _p->data.parentTypes;
    if (type->info() != _p->data.classType->info() && std::find(p.begin(), p.end(),
      std::make_pair(type, offset)) == p.end())
    {
      qiLogVerbose("qi.meta") << "Declaring inheritance "
      << _p->data.classType->infoString() << " <- " << type->infoString();
      p.push_back(std::make_pair(type, offset));
    }
  }
}


