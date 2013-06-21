/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#include <qitype/anyfunction.hpp>
#include <qitype/anyobject.hpp>

#include "metamethod_p.hpp"

qiLogCategory("qitype.metamethod");

namespace qi {
  // ***** MetaMethod Implementation *****
  MetaMethod::MetaMethod()
    : _p (new MetaMethodPrivate())
  {}

  MetaMethod::MetaMethod(unsigned int newUid, const MetaMethod& other)
    : _p (new MetaMethodPrivate())
  {
    *(this->_p) = *(other._p);
    this->_p->uid = newUid;
  }

  MetaMethod::MetaMethod(const MetaMethod& other)
    : _p (new MetaMethodPrivate())
  {
    *(this->_p) = *(other._p);
  }

  MetaMethod& MetaMethod::operator =(const MetaMethod& other) {
    *(this->_p) = *(other._p);
    return (*this);
  }

  MetaMethod::~MetaMethod() {
    delete this->_p;
  }

  unsigned int MetaMethod::uid() const {
    return this->_p->uid;
  }

  std::string MetaMethod::name() const {
    return this->_p->name;
  }

  const qi::Signature& MetaMethod::parametersSignature() const {
    return this->_p->parametersSignature;
  }

  std::string MetaMethod::toString() const {
    return this->_p->name + "::" + this->_p->parametersSignature.toString();
  }

  const qi::Signature& MetaMethod::returnSignature() const {
    return this->_p->sigreturn;
  }

  std::string MetaMethod::description() const {
    return this->_p->description;
  }

  MetaMethodParameterVector MetaMethod::parameters() const {
    return this->_p->parameters;
  }

  std::string MetaMethod::returnDescription() const {
    return this->_p->returnDescription;
  }

  bool MetaMethod::isPrivate() const {
    return MetaObject::isPrivateMember(name(), uid());
  }

  // ***** MetaMethodPrivate Implementation *****
  MetaMethodPrivate::MetaMethodPrivate()
    : uid(0), parameters (0)
  {}

  void MetaMethodPrivate::appendParameter(const MetaMethodParameter& mm) {
    this->parameters.push_back(mm);
  }

  void MetaMethodPrivate::setDescription(const std::string &desc) {
    this->description = desc;
  }


  // ***** MetaMethodParameter Implementation *****
  MetaMethodParameter::MetaMethodParameter()
    : _p (new MetaMethodParameterPrivate())
  {}

  MetaMethodParameter::MetaMethodParameter(const std::string &name,
                                           const std::string &doc)
    : _p (new MetaMethodParameterPrivate(name, doc))
  {}

  MetaMethodParameter::MetaMethodParameter(const MetaMethodParameter &other)
    : _p (new MetaMethodParameterPrivate())
  {
    *(this->_p) = *(other._p);
  }

  MetaMethodParameter& MetaMethodParameter::operator =(const MetaMethodParameter& other) {
    *(this->_p) = *(other._p);
    return (*this);
  }

  MetaMethodParameter::~MetaMethodParameter() {
    delete this->_p;
  }

  std::string MetaMethodParameter::name() const {
    return this->_p->name;
  }

  std::string MetaMethodParameter::description() const {
    return this->_p->description;
  }


  // ***** MetaMethodParameterPrivate Implementation *****
  MetaMethodParameterPrivate::MetaMethodParameterPrivate()
    : name (), description ()
  {}

  MetaMethodParameterPrivate::MetaMethodParameterPrivate(const std::string &name,
                                                         const std::string &doc)
    : name (name), description (doc)
  {}


  // ***** MetaMethodBuilder Implementation *****
  MetaMethodBuilder::MetaMethodBuilder()
    : _p (new MetaMethodBuilderPrivate())
  {}

  MetaMethodBuilder::MetaMethodBuilder(const qi::Signature& sigreturn,
                                       const std::string& name,
                                       const qi::Signature& signature,
                                       const std::string& desc)
    : _p (new MetaMethodBuilderPrivate())
  {
    setReturnSignature(sigreturn);
    setName(name);
    setParametersSignature(signature);
    setDescription(desc);
  }

  MetaMethodBuilder::MetaMethodBuilder(const MetaMethodBuilder &other)
    : _p (new MetaMethodBuilderPrivate())
  {
    *(this->_p) = *(other._p);
  }

  MetaMethodBuilder& MetaMethodBuilder::operator =(const MetaMethodBuilder& other) {
    *(this->_p) = *(other._p);
    return (*this);
  }

  MetaMethodBuilder::~MetaMethodBuilder() {
    delete this->_p;
  }

  MetaMethod MetaMethodBuilder::metaMethod() {
    return this->_p->metaMethod;
  }

  std::string MetaMethodBuilder::name() const {
    return this->_p->metaMethod._p->name;
  }

  void MetaMethodBuilder::setUid(unsigned int uid) {
    this->_p->metaMethod._p->uid = uid;
  }

  void MetaMethodBuilder::setName(const std::string &name) {
    this->_p->metaMethod._p->name = name;
  }

  void MetaMethodBuilder::setParametersSignature(const qi::Signature &sig) {
    this->_p->metaMethod._p->parametersSignature = sig;
  }

  void MetaMethodBuilder::setReturnSignature(const qi::Signature& sig) {
    this->_p->metaMethod._p->sigreturn = sig;
  }

  void MetaMethodBuilder::setSignature(const AnyFunction& f)
  {
    //this is always a method: drop the first argument
    setParametersSignature(f.parametersSignature(true));
    setReturnSignature(f.returnSignature());
  }

  void MetaMethodBuilder::setReturnDescription(const std::string& desc) {
    this->_p->metaMethod._p->returnDescription = desc;
  }

  void MetaMethodBuilder::appendParameter(const std::string& name,
                                       const std::string& description)
  {
    MetaMethodParameter mmp(name, description);
    this->_p->metaMethod._p->appendParameter(mmp);
  }

  void MetaMethodBuilder::setDescription(const std::string &description) {
    this->_p->metaMethod._p->setDescription(description);
  }

  MetaMethod::MetaMethod(unsigned int uid, const Signature &returnSignature,
      const std::string& name, const Signature &parametersSignature,
      const std::string& description, const MetaMethodParameterVector& parameters,
      const std::string& returnDescription)
  : _p (new MetaMethodPrivate())
  {
    _p->uid = uid;
    _p->sigreturn = returnSignature;
    _p->name = name;
    _p->parametersSignature = parametersSignature;
    _p->description = description;
    _p->parameters = parameters;
    _p->returnDescription = returnDescription;
  }
}

#define PBOUNCE(cls, field, type) \
  static const type& QI_CAT(QI_CAT(cls, _), field)(::qi::cls* ptr) { \
    return ptr->_p->field; \
  }

namespace {
PBOUNCE(MetaMethod, uid,         unsigned int)
PBOUNCE(MetaMethod, name,        std::string)
PBOUNCE(MetaMethod, description, std::string)
PBOUNCE(MetaMethod, parameters,  ::qi::MetaMethodParameterVector)
PBOUNCE(MetaMethod, returnDescription, std::string)

PBOUNCE(MetaMethodParameter, name,        std::string)
PBOUNCE(MetaMethodParameter, description, std::string)
}

QI_TYPE_STRUCT_AGREGATE_CONSTRUCTOR_REGISTER(::qi::MetaMethod,
  QI_STRUCT_HELPER("uid", MetaMethod_uid),
  ("returnSignature", returnSignature),
  QI_STRUCT_HELPER("name", MetaMethod_name),
  ("parametersSignature", parametersSignature),
  QI_STRUCT_HELPER("description", MetaMethod_description),
  QI_STRUCT_HELPER("parameters", MetaMethod_parameters),
  QI_STRUCT_HELPER("returnDescription", MetaMethod_returnDescription));

QI_TYPE_STRUCT_AGREGATE_CONSTRUCTOR_REGISTER(::qi::MetaMethodParameter,
  QI_STRUCT_HELPER("name", MetaMethodParameter_name),
  QI_STRUCT_HELPER("description", MetaMethodParameter_description));

