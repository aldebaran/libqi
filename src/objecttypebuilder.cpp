/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#include <boost/thread.hpp>
#include <qitype/genericobject.hpp>
#include <qitype/objecttypebuilder.hpp>
#include "staticobjecttype.hpp"
#include "metaobject_p.hpp"

qiLogCategory("qitype.objectbuilder");

namespace qi {

  class ObjectTypeBuilderPrivate
  {
  public:
    ObjectTypeBuilderPrivate()
    : type(0)
    , threadingModel(ObjectThreadingModel_SingleThread)
    , autoRegister(true)
    {}
    ObjectTypeData data;
    ObjectType*    type;
    MetaObject     metaObject;
    ObjectThreadingModel threadingModel;
    bool                 autoRegister;
  };

  ObjectTypeBuilderBase::ObjectTypeBuilderBase()
  : _p(new ObjectTypeBuilderPrivate())
  {
  }

  ObjectTypeBuilderBase::~ObjectTypeBuilderBase()
  {
    delete _p;
  }

  int ObjectTypeBuilderBase::xAdvertiseMethod(MetaMethodBuilder& builder,
                                              GenericFunction func,
                                              MetaCallType threadingModel,
                                              int id)
  {
    if (_p->type) {
      qiLogWarning()
          << "ObjectTypeBuilder: Called xAdvertiseMethod with method '"
          << builder.metaMethod().sigreturn() << " "
          << builder.metaMethod().signature()
          << "' but type is already created.";
    }
    int nextId = _p->metaObject._p->addMethod(builder, id);
    if (nextId < 0)
      return -1;
    _p->data.methodMap[nextId] = std::make_pair(func, threadingModel);
    return nextId;
  }

  int ObjectTypeBuilderBase::xAdvertiseEvent(const std::string& signature, SignalMemberGetter getter, int id)
  {
    if (_p->type) {
      qiLogWarning() << "ObjectTypeBuilder: Called xAdvertiseEvent with event '"
                     << signature << "' but type is already created.";
    }
    int nextId = _p->metaObject._p->addSignal(signature, id);
    if (nextId < 0)
      return -1;
    _p->data.signalGetterMap[nextId] = getter;
    return nextId;
  }

  int ObjectTypeBuilderBase::xAdvertiseProperty(const std::string& name, const std::string& sig, PropertyMemberGetter getter,int id)
  {
    int res = _p->metaObject._p->addProperty(name, sig, id);
    if (res < 0)
      return -1;
    _p->data.propertyGetterMap[res] = getter;
    return id;
  }

  void ObjectTypeBuilderBase::xBuildFor(Type* type, bool autoRegister)
  {
    _p->data.classType = type;
    _p->autoRegister = autoRegister;
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

  ObjectPtr ObjectTypeBuilderBase::object(void* ptr,
    boost::function<void (GenericObject*)> onDestroy)
  {
    ObjectPtr ret = onDestroy?
      ObjectPtr(new GenericObject(type(), ptr), onDestroy):
      ObjectPtr(new GenericObject(type(), ptr));
    return ret;
  }

  ObjectType* ObjectTypeBuilderBase::type()
  {
    if (!_p->type)
    {
      StaticObjectTypeBase* t = new StaticObjectTypeBase();
      t->initialize(metaObject(), _p->data);
      _p->type = t;
      if (_p->autoRegister)
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
      qiLogVerbose() << "Declaring inheritance "
      << _p->data.classType->infoString() << " <- " << type->infoString();
      p.push_back(std::make_pair(type, offset));
    }
  }
}


