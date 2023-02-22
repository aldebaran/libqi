/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#include <qi/future.hpp>
#include <qi/type/typeinterface.hpp>
#include <qi/type/dynamicobjectbuilder.hpp>
#include <qi/type/dynamicobject.hpp>
#include "metaobject_p.hpp"
#include <boost/optional.hpp>

qiLogCategory("qitype.objectbuilder");

namespace qi
{
  class DynamicObjectBuilderPrivate
  {
  public:
    DynamicObjectBuilderPrivate()
      : _object(new DynamicObject())
      , _isObjectOwner(true)
      , _objptr()
    {}

    DynamicObjectBuilderPrivate(DynamicObject *dynobject, bool isObjectOwner)
      : _object(dynobject)
      , _isObjectOwner(isObjectOwner)
      , _objptr()
    {}

    ~DynamicObjectBuilderPrivate()
    {
      if (_isObjectOwner)
        delete _object;
    }

    DynamicObject* _object;
    bool           _isObjectOwner;
    AnyObject      _objptr;
  };

  DynamicObjectBuilder::DynamicObjectBuilder()
    : _p(new DynamicObjectBuilderPrivate)
  {
  }

  DynamicObjectBuilder::DynamicObjectBuilder(DynamicObject *dynobject, bool isObjectOwner)
    : _p(new DynamicObjectBuilderPrivate(dynobject, isObjectOwner))
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

    const unsigned int nextId = _p->_object->metaObject()._p->addMethod(builder).id;

    _p->_object->setMethod(nextId, func, threadingModel);
    return nextId;
  }

  unsigned int DynamicObjectBuilder::xAdvertiseSignal(const std::string &name, const qi::Signature& signature, bool isSignalProperty)
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
    const auto signalAddResult = _p->_object->metaObject()._p->addSignal(name, signature, -1, isSignalProperty);
    if (isSignalProperty && !signalAddResult.isNewMember)
    {
      throw std::runtime_error("Registering property failed: name already used by a member Signal: " + name);
    }
    const unsigned int nextId = signalAddResult.id;
    return nextId;
  }

  unsigned int DynamicObjectBuilder::advertiseSignal(const std::string &name, qi::SignalBase *sig)
  {
    return advertiseSignal(name, boost::shared_ptr<SignalBase>(sig, ka::constant_function()));
  }

  unsigned int DynamicObjectBuilder::advertiseSignal(const std::string& name,
                                                     boost::shared_ptr<qi::SignalBase> sig)
  {
    QI_ASSERT_NOT_NULL(sig);
    const unsigned int nextId = xAdvertiseSignal(name, sig->signature());
    _p->_object->setSignal(nextId, sig);
    return nextId;
  }

  unsigned int DynamicObjectBuilder::advertiseProperty(const std::string &name, qi::PropertyBase *prop)
  {
    return advertiseProperty(name, boost::shared_ptr<PropertyBase>(prop, ka::constant_function()));
  }

  unsigned int DynamicObjectBuilder::advertiseProperty(const std::string& name,
                                                       boost::shared_ptr<qi::PropertyBase> prop)
  {
    QI_ASSERT_NOT_NULL(prop);
    const auto sigsignature = prop->signal()->signature();
    if (!sigsignature.hasChildren() || sigsignature.children().size() != 1)
      throw std::runtime_error("Registering property with invalid signal signature");
    const auto propsignature = sigsignature.children()[0];

    const unsigned int nextId = xAdvertiseSignal(name, sigsignature, true);
    xAdvertiseProperty(name, propsignature, nextId);
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
    unsigned int res = _p->_object->metaObject()._p->addProperty(name, sig, id).id;
    return res;
  }


  void DynamicObjectBuilder::setDescription(const std::string &desc) {
    _p->_object->metaObject()._p->setDescription(desc);
  }

  AnyObject DynamicObjectBuilder::object(boost::function<void (GenericObject*)> onDelete)
  {
    if (!_p->_objptr)
    {
      _p->_objptr = makeDynamicAnyObject(_p->_object,
                                         _p->_isObjectOwner,
                                         _p->_object->uid(), onDelete);
      // The ownership of the object is moved from the builder to the constructed `AnyObject`.
      _p->_isObjectOwner = false;
      _p->_object->setManageable(_p->_objptr.asGenericObject());
    }
    return _p->_objptr;
  }

  DynamicObject* DynamicObjectBuilder::bareObject()
  {
    return _p->_object;
  }

  void DynamicObjectBuilder::setManageable(DynamicObject* obj, Manageable* m)
  {
    obj->setManageable(m);
  }

  void DynamicObjectBuilder::setThreadingModel(ObjectThreadingModel model)
  {
    _p->_object->setThreadingModel(model);
  }

  void DynamicObjectBuilder::setOptionalUid(boost::optional<ObjectUid> maybeUid)
  {
    return _p->_object->setUid(maybeUid);
  }
}
