/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#include <qi/future.hpp>
#include <qitype/typeinterface.hpp>
#include <qitype/dynamicobjectbuilder.hpp>
#include <qitype/dynamicobject.hpp>
#include "metaobject_p.hpp"

qiLogCategory("qitype.objectbuilder");

namespace qi
{
  class DynamicObjectBuilderPrivate
  {
  public:
    DynamicObjectBuilderPrivate()
      : _object(new DynamicObject())
      , _deleteOnDestroy(true)
      , _objptr()
    {}

    DynamicObjectBuilderPrivate(DynamicObject *dynobject, bool deleteOnDestroy)
      : _object(dynobject)
      , _deleteOnDestroy(deleteOnDestroy)
      , _objptr()
    {}

    ~DynamicObjectBuilderPrivate()
    {}

    DynamicObject* _object;
    bool           _deleteOnDestroy;
    qi::AnyObject  _objptr;
  };

  DynamicObjectBuilder::DynamicObjectBuilder()
    : _p(new DynamicObjectBuilderPrivate)
  {
  }

  DynamicObjectBuilder::DynamicObjectBuilder(DynamicObject *dynobject, bool deleteOnDestroy)
    : _p(new DynamicObjectBuilderPrivate(dynobject, deleteOnDestroy))
  {}

  DynamicObjectBuilder::~DynamicObjectBuilder()
  {
    delete _p;
  }

  static bool isSignatureValid(const qi::Signature &sigret, const std::string& name, const qi::Signature& signature)
  {
    if (!signature.isValid())
      return false;
    if (name.empty())
      return false;
    if (!sigret.isValid())
      return false;
    return true;
  }

  unsigned int DynamicObjectBuilder::xAdvertiseMethod(const qi::Signature& sigret,
                                                      const std::string& name,
                                                      const qi::Signature& signature,
                                                      AnyFunction func,
                                                      const std::string& desc,
                                                      MetaCallType threadingModel)
  {
    if (!isSignatureValid(sigret, name, signature)) {
      std::stringstream err;
      err << "DynamicObjectBuilder: Called xAdvertiseMethod("<< sigret.toString() << "," << name << "," << signature.toString() << ") with an invalid signature.";
      throw std::runtime_error(err.str());
    }
    MetaMethodBuilder mmb;
    mmb.setReturnSignature(sigret);
    mmb.setName(name);
    mmb.setParametersSignature(signature);
    mmb.setDescription(desc);

    // throw on error
    return xAdvertiseMethod(mmb, func, threadingModel);
  }

  unsigned int DynamicObjectBuilder::xAdvertiseMethod(MetaMethodBuilder& builder,
                                                      AnyFunction func,
                                                      MetaCallType threadingModel)
  {
    MetaMethod mm = builder.metaMethod();
    if (!isSignatureValid(mm.returnSignature(), mm.name(), mm.parametersSignature())) {
      std::stringstream err;
      err << "DynamicObjectBuilder: Called xAdvertiseMethod("<< mm.returnSignature().toString() << "," << mm.name() << "," << mm.parametersSignature().toString() << ") with an invalid signature.";
      throw std::runtime_error(err.str());
    }
    //TODO: check that func is compatible with MM
    if (_p->_objptr) {
      qiLogWarning()
          << "DynamicObjectBuilder: Called xAdvertiseMethod with method '"
          << mm.toString()
          << "' but object is already created.";
    }

    unsigned int nextId = _p->_object->metaObject()._p->addMethod(builder);

    _p->_object->setMethod(nextId, func, threadingModel);
    return nextId;
  }

  unsigned int DynamicObjectBuilder::xAdvertiseSignal(const std::string &name, const qi::Signature& signature)
  {
    if (!Signature(signature).isValid() || name.empty()) {
      std::stringstream err;
      if (name.empty())
        err << "DynamicObjectBuilder: Called xAdvertiseSignal with a signal name empty and signature " << signature.toString() << ".";
      else
        err << "DynamicObjectBuilder: Called xAdvertiseSignal("<< name << "," << signature.toString() << ") with an invalid signature.";
      throw std::runtime_error(err.str());
    }
    if (_p->_objptr) {
      qiLogWarning() << "DynamicObjectBuilder: Called xAdvertiseSignal on event '" << signature.toString() << "' but object is already created.";
    }
    // throw on error
    unsigned int nextId = _p->_object->metaObject()._p->addSignal(name, signature);
    return nextId;
  }

  unsigned int DynamicObjectBuilder::advertiseSignal(const std::string &name, qi::SignalBase *sig)
  {
    unsigned int nextId = xAdvertiseSignal(name, sig->signature());
    _p->_object->setSignal(nextId, sig);
    return nextId;
  }

  unsigned int DynamicObjectBuilder::advertiseProperty(const std::string &name, qi::PropertyBase *prop)
  {
    //todo: prop.signature()
    unsigned int nextId = xAdvertiseProperty(name, prop->signal()->signature());
    _p->_object->setProperty(nextId, prop);
    return nextId;
  }

  unsigned int DynamicObjectBuilder::xAdvertiseProperty(const std::string& name, const Signature &sig, int id)
  {
    if (!Signature(sig).isValid() || name.empty()) {
      std::stringstream err;
      if (name.empty())
        err << "DynamicObjectBuilder: Called xAdvertiseProperty with a property name empty and signature " << sig.toString() << ".";
      else
        err << "DynamicObjectBuilder: Called xAdvertiseProperty("<< name << "," << sig.toString() << ") with an invalid signature.";
      throw std::runtime_error(err.str());
    }
    return _p->_object->metaObject()._p->addProperty(name, sig, id);
  }


  void DynamicObjectBuilder::setDescription(const std::string &desc) {
    _p->_object->metaObject()._p->setDescription(desc);
  }

  AnyObject DynamicObjectBuilder::object(boost::function<void (GenericObject*)> onDelete)
  {
    if (!_p->_objptr)
    {
      _p->_objptr = makeDynamicAnyObject(_p->_object, _p->_deleteOnDestroy, onDelete);
      _p->_object->setManageable(_p->_objptr.get());
    }
    return _p->_objptr;
  }

  void DynamicObjectBuilder::setThreadingModel(ObjectThreadingModel model)
  {
    _p->_object->setThreadingModel(model);
  }
}
