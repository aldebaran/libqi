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
    ObjectTypeBuilderPrivate() : type(0), threadingModel(ObjectThreadingModel_SingleThread)  {}
    ObjectTypeData data;
    ObjectType*    type;
    MetaObject     metaObject;
    ObjectThreadingModel threadingModel;
  };

  ObjectTypeBuilderBase::ObjectTypeBuilderBase()
  : _p(new ObjectTypeBuilderPrivate())
  {
  }

  ObjectTypeBuilderBase::~ObjectTypeBuilderBase()
  {
    delete _p;
  }

  int ObjectTypeBuilderBase::xAdvertiseMethod(const std::string &retsig,
                                              const std::string& signature,
                                              GenericMethod func,
                                              MetaCallType threadingModel,
                                              int id)
  {
    if (_p->type) {
      qiLogVerbose("ObjectTypeBuilder") << "ObjectTypeBuilder: Called xAdvertiseMethod with method '"
                                        << retsig << " " << signature << "' but type is already created.";
    }
    unsigned int nextId = _p->metaObject._p->addMethod(retsig, signature, id);
    _p->data.methodMap[nextId] = std::make_pair(func, threadingModel);
    return nextId;
  }

  int ObjectTypeBuilderBase::xAdvertiseEvent(const std::string& signature, SignalMemberGetter getter, int id)
  {
    if (_p->type) {
      qiLogVerbose("ObjectTypeBuilder") << "ObjectTypeBuilder: Called xAdvertiseEvent with event '"
                                        << signature << "' but type is already created.";
    }
    unsigned int nextId = _p->metaObject._p->addSignal(signature, id);
    _p->data.signalGetterMap[nextId] = getter;
    return nextId;
  }

  void ObjectTypeBuilderBase::xBuildFor(Type* type)
  {
    _p->data.classType = type;
  }

  void ObjectTypeBuilderBase::setThreadingModel(ObjectThreadingModel model)
  {
    _p->threadingModel = model;
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


