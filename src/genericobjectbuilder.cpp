/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#include <qi/future.hpp>
#include <qitype/type.hpp>
#include <qitype/genericobjectbuilder.hpp>
#include <qitype/dynamicobject.hpp>
#include "metaobject_p.hpp"

namespace qi
{
  class GenericObjectBuilderPrivate
  {
  public:
    GenericObjectBuilderPrivate()
      : _object(new DynamicObject())
      , _deleteOnDestroy(true)
      , _objptr()
      , _threadingModel(ObjectThreadingModel_SingleThread)
    {}

    GenericObjectBuilderPrivate(DynamicObject *dynobject, bool deleteOnDestroy)
      : _object(dynobject)
      , _deleteOnDestroy(deleteOnDestroy)
      , _objptr()
      , _threadingModel(ObjectThreadingModel_SingleThread)
    {}

    ~GenericObjectBuilderPrivate()
    {}

    DynamicObject* _object;
    bool           _deleteOnDestroy;
    qi::ObjectPtr  _objptr;
    ObjectThreadingModel _threadingModel;
  };

  GenericObjectBuilder::GenericObjectBuilder()
    : _p(new GenericObjectBuilderPrivate)
  {
  }

  GenericObjectBuilder::GenericObjectBuilder(DynamicObject *dynobject, bool deleteOnDestroy)
    : _p(new GenericObjectBuilderPrivate(dynobject, deleteOnDestroy))
  {}

  GenericObjectBuilder::~GenericObjectBuilder()
  {
    delete _p;
  }

  int GenericObjectBuilder::xAdvertiseMethod(const std::string& sigret,
                                             const std::string& signature,
                                             GenericFunction func,
                                             const std::string& desc,
                                             MetaCallType threadingModel)
  {
    MetaMethodBuilder mmb;
    mmb.setSigreturn(sigret);
    mmb.setSignature(signature);
    mmb.setDescription(desc);
    return xAdvertiseMethod(mmb, func, threadingModel);
  }

  int GenericObjectBuilder::xAdvertiseMethod(MetaMethodBuilder& builder,
                                             GenericFunction func,
                                             MetaCallType threadingModel)
  {
    if (_p->_objptr) {
      qiLogWarning("GenericObjectBuilder")
          << "GenericObjectBuilder: Called xAdvertiseMethod with method '"
          << builder.metaMethod().signature()
          << "' but object is already created.";
    }

    unsigned int nextId = _p->_object->metaObject()._p->addMethod(builder);
    _p->_object->setMethod(nextId, func, threadingModel);
    return nextId;
  }

  int GenericObjectBuilder::xAdvertiseEvent(const std::string& signature)
  {
    if (_p->_objptr) {
      qiLogWarning("GenericObjectBuilder") << "GenericObjectBuilder: Called xAdvertiseEvent on event '" << signature << "' but object is already created.";
    }
    unsigned int nextId = _p->_object->metaObject()._p->addSignal(signature);
    return nextId;
  }

  void GenericObjectBuilder::setDescription(const std::string &desc) {
    _p->_object->metaObject()._p->setDescription(desc);
  }

  ObjectPtr GenericObjectBuilder::object()
  {
    if (!_p->_objptr)
      _p->_objptr = makeDynamicObjectPtr(_p->_object, _p->_deleteOnDestroy);
    return _p->_objptr;
  }

  void GenericObjectBuilder::setThreadingModel(ObjectThreadingModel model)
  {
    _p->_threadingModel = model;
  }
}
