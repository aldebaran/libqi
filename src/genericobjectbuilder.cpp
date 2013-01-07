/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#include <qi/future.hpp>
#include <qitype/type.hpp>
namespace qi
{
  template class QITYPE_API Future<GenericValuePtr>;
  template class QITYPE_API Future<void>;
}

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

  int GenericObjectBuilder::xAdvertiseMethod(const std::string &retsig,
                                             const std::string& signature,
                                             GenericFunction func,
                                             MetaCallType threadingModel)
  {
    if (_p->_objptr) {
      qiLogWarning("GenericObjectBuilder") << "GenericObjectBuilder: Called xAdvertiseMethod with method '" << signature << "' but object is already created.";
    }

    unsigned int nextId = _p->_object->metaObject()._p->addMethod(retsig, signature);
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
