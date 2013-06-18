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
    ObjectTypeInterface*    type;
    MetaObject     metaObject;
    ObjectThreadingModel threadingModel;
    bool                 autoRegister;
  };

  ObjectTypeBuilderBase::ObjectTypeBuilderBase()
  : _p(new ObjectTypeBuilderPrivate())
  {
    // import manageable stuff
    // need this trick to avoid changing nextId
    _p->metaObject = MetaObject::merge(_p->metaObject, Manageable::manageableMetaObject());
    _p->data.signalGetterMap = Manageable::manageableSignalMap();
    _p->data.methodMap = Manageable::manageableMmethodMap();
  }

  ObjectTypeBuilderBase::~ObjectTypeBuilderBase()
  {
    delete _p;
  }

  int ObjectTypeBuilderBase::xAdvertiseMethod(MetaMethodBuilder& builder,
                                              AnyFunction func,
                                              MetaCallType threadingModel,
                                              int id)
  {
    if (_p->type) {
      qiLogWarning()
          << "ObjectTypeBuilder: Called xAdvertiseMethod with method '"
          << builder.metaMethod().toString()
          << "' but type is already created.";
    }
    int nextId = _p->metaObject._p->addMethod(builder, id);
    if (nextId < 0)
      return -1;
    _p->data.methodMap[nextId] = std::make_pair(func, threadingModel);
    return nextId;
  }

  int ObjectTypeBuilderBase::xAdvertiseSignal(const std::string &name, const qi::Signature& signature, SignalMemberGetter getter, int id)
  {
    if (_p->type) {
      qiLogWarning() << "ObjectTypeBuilder: Called xAdvertiseSignal with event '"
                     << signature.toString() << "' but type is already created.";
    }
    int nextId = _p->metaObject._p->addSignal(name, signature, id);
    if (nextId < 0)
      return -1;
    _p->data.signalGetterMap[nextId] = getter;
    return nextId;
  }

  int ObjectTypeBuilderBase::xAdvertiseProperty(const std::string& name, const qi::Signature& signature, PropertyMemberGetter getter, int id)
  {
    int res = _p->metaObject._p->addProperty(name, signature, id);
    if (res < 0)
      return -1;
    _p->data.propertyGetterMap[res] = getter;
    return id;
  }

  void ObjectTypeBuilderBase::xBuildFor(TypeInterface* type, bool autoRegister)
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

  const ObjectTypeData& ObjectTypeBuilderBase::typeData()
  {
    return _p->data;
  }

  ObjectTypeInterface* ObjectTypeBuilderBase::type()
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

  void ObjectTypeBuilderBase::inherits(TypeInterface* type, int offset)
  {
    std::vector<std::pair<TypeInterface*, int> >& p = _p->data.parentTypes;
    if (type->info() != _p->data.classType->info() && std::find(p.begin(), p.end(),
      std::make_pair(type, offset)) == p.end())
    {
      qiLogVerbose() << "Declaring inheritance "
      << _p->data.classType->infoString() << " <- " << type->infoString();
      p.push_back(std::make_pair(type, offset));
    }
  }
}


