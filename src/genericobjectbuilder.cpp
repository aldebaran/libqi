/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#include <qi/future.hpp>
#include <qitype/type.hpp>
#include <qitype/genericobjectbuilder.hpp>
#include <qitype/dynamicobject.hpp>
#include "metaobject_p.hpp"

qiLogCategory("qitype.objectbuilder");

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
      qiLogWarning()
          << "GenericObjectBuilder: Called xAdvertiseMethod with method '"
          << builder.metaMethod().signature()
          << "' but object is already created.";
    }

    int nextId = _p->_object->metaObject()._p->addMethod(builder);
    if (nextId < 0)
      return -1;
    _p->_object->setMethod(nextId, func, threadingModel);
    return nextId;
  }

  int GenericObjectBuilder::xAdvertiseEvent(const std::string& signature)
  {
    if (_p->_objptr) {
      qiLogWarning() << "GenericObjectBuilder: Called xAdvertiseEvent on event '" << signature << "' but object is already created.";
    }
    int nextId = _p->_object->metaObject()._p->addSignal(signature);
    if (nextId < 0)
      return -1;
    return nextId;
  }

  void GenericObjectBuilder::setDescription(const std::string &desc) {
    _p->_object->metaObject()._p->setDescription(desc);
  }

  ObjectPtr GenericObjectBuilder::object(boost::function<void (GenericObject*)> onDelete)
  {
    if (!_p->_objptr)
      _p->_objptr = makeDynamicObjectPtr(_p->_object, _p->_deleteOnDestroy, onDelete);
    return _p->_objptr;
  }

  void GenericObjectBuilder::setThreadingModel(ObjectThreadingModel model)
  {
    _p->_threadingModel = model;
  }
}
