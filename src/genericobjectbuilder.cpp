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
    {}

    GenericObjectBuilderPrivate(DynamicObject *dynobject, bool deleteOnDestroy)
      : _object(dynobject)
      , _deleteOnDestroy(deleteOnDestroy)
      , _objptr()
    {}

    ~GenericObjectBuilderPrivate()
    {}

    DynamicObject* _object;
    bool           _deleteOnDestroy;
    qi::ObjectPtr  _objptr;
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

  static int isSignatureValid(const std::string &sigret, const std::string& name, const std::string& signature)
  {
    Signature sparams(signature);
    if (!sparams.isValid())
      return -1;

    if (name.empty())
      return -1;

    if (sigret != "")
    {
      Signature sret(sigret);
      if (sret.isValid() == false)
        return -1;
    }
    return 0;
  }

  int GenericObjectBuilder::xAdvertiseMethod(const std::string& sigret,
                                             const std::string& name,
                                             const std::string& signature,
                                             GenericFunction func,
                                             const std::string& desc,
                                             MetaCallType threadingModel)
  {
    if (isSignatureValid(sigret, name, signature) < 0) {
      qiLogWarning() << "GenericObjectBuilder: Called xAdvertiseMethod with an invalid signature.";
      return -1;
    }
    MetaMethodBuilder mmb;
    mmb.setReturnSignature(sigret);
    mmb.setName(name);
    mmb.setParametersSignature(signature);
    mmb.setDescription(desc);
    return xAdvertiseMethod(mmb, func, threadingModel);
  }

  int GenericObjectBuilder::xAdvertiseMethod(MetaMethodBuilder& builder,
                                             GenericFunction func,
                                             MetaCallType threadingModel)
  {
    MetaMethod mm = builder.metaMethod();
    if (isSignatureValid(mm.returnSignature(), mm.name(), mm.parametersSignature()) < 0) {
      qiLogWarning() << "GenericObjectBuilder: Called xAdvertiseMethod("<< mm.returnSignature() << "," << mm.name() << "," << mm.parametersSignature() << ") with an invalid signature.";
      return -1;
    }
    if (_p->_objptr) {
      qiLogWarning()
          << "GenericObjectBuilder: Called xAdvertiseMethod with method '"
          << mm.toString()
          << "' but object is already created.";
    }

    int nextId = _p->_object->metaObject()._p->addMethod(builder);
    if (nextId < 0)
      return -1;
    _p->_object->setMethod(nextId, func, threadingModel);
    return nextId;
  }

  int GenericObjectBuilder::xAdvertiseSignal(const std::string &name, const std::string& signature)
  {
    if (!Signature(signature).isValid()) {
      qiLogWarning() << "GenericObjectBuilder: Called xAdvertiseSignal("<< name << "," << signature << ") with an invalid signature.";
      return -1;
    }
    if (_p->_objptr) {
      qiLogWarning() << "GenericObjectBuilder: Called xAdvertiseSignal on event '" << signature << "' but object is already created.";
    }
    int nextId = _p->_object->metaObject()._p->addSignal(name, signature);
    if (nextId < 0)
      return -1;
    return nextId;
  }

  int GenericObjectBuilder::advertiseSignal(const std::string &name, qi::SignalBase *sig)
  {
    int nextId = xAdvertiseSignal(name, sig->signature());
    _p->_object->setSignal(nextId, sig);
    return nextId;
  }

  int GenericObjectBuilder::advertiseProperty(const std::string &name, qi::PropertyBase *prop)
  {
    //todo: prop.signature()
    int nextId = xAdvertiseProperty(name, prop->signal()->signature());
    _p->_object->setProperty(nextId, prop);
    return nextId;
  }

  int GenericObjectBuilder::xAdvertiseProperty(const std::string& name, const std::string& sig, int id)
  {
    if (!Signature(sig).isValid()) {
      qiLogWarning() << "GenericObjectBuilder: Called xAdvertiseProperty("<< name << "," << sig << ") with an invalid signature.";
      return -1;
    }
    return _p->_object->metaObject()._p->addProperty(name, sig, id);
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
    _p->_object->setThreadingModel(model);
  }
}
