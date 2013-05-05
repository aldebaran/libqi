/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#include <qitype/functiontype.hpp>

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

  const std::string& MetaMethod::parametersSignature() const {
    return this->_p->parametersSignature;
  }

  std::string MetaMethod::toString() const {
    return this->_p->name + "::" + this->_p->parametersSignature;
  }

  const std::string& MetaMethod::returnSignature() const {
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


  // ***** MetaMethodPrivate Implementation *****
  MetaMethodPrivate::MetaMethodPrivate()
    : uid(0), parameters (0)
  {}

  void MetaMethodPrivate::addParameter(const MetaMethodParameter& mm) {
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

  MetaMethodBuilder::MetaMethodBuilder(const std::string& name, const std::string& desc)
    : _p (new MetaMethodBuilderPrivate())
  {
    this->_p->name = name;
    this->_p->metaMethod._p->setDescription(desc);
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
    return this->_p->name;
  }

  void MetaMethodBuilder::setUid(unsigned int uid) {
    this->_p->metaMethod._p->uid = uid;
  }

  void MetaMethodBuilder::setSignature(const std::string& sig) {
    std::vector<std::string> split = signatureSplit(sig);
    this->_p->metaMethod._p->name = split[1];
    this->_p->metaMethod._p->parametersSignature = split[2];
  }

  void MetaMethodBuilder::setSigreturn(const std::string& sig) {
    this->_p->metaMethod._p->sigreturn = sig;
  }

  void MetaMethodBuilder::setSignatures(const GenericFunction& f)
  {
    setSignatures(name(), f);
  }

  void MetaMethodBuilder::setSignatures(const std::string& name, const GenericFunction& f)
  {
    this->_p->name = name;
    qiLogDebug() << "sig " << f.signature() << " -> " << ('(' + f.signature().substr(2));
    // Drop first argument which is the instance
    setSignature(name + "::(" + f.signature().substr(2));
    setSigreturn(f.sigreturn());
  }

  void MetaMethodBuilder::setReturnDescription(const std::string& desc) {
    this->_p->metaMethod._p->returnDescription = desc;
  }

  void MetaMethodBuilder::addParameter(const std::string& name,
                                       const std::string& description)
  {
    MetaMethodParameter mmp(name, description);
    this->_p->metaMethod._p->addParameter(mmp);
  }

  void MetaMethodBuilder::setDescription(const std::string &description) {
    this->_p->metaMethod._p->setDescription(description);
  }
}

static qi::MetaMethodPrivate* metaMethodPrivate(qi::MetaMethod* mm) { return mm->_p; }
static qi::MetaMethodParameterPrivate* metaMethodParameterPrivate(qi::MetaMethodParameter* mmp) { return mmp->_p; }

QI_TYPE_STRUCT(::qi::MetaMethodParameterPrivate, name, description);
QI_TYPE_STRUCT_BOUNCE_REGISTER(::qi::MetaMethodParameter,
                               ::qi::MetaMethodParameterPrivate,
                               metaMethodParameterPrivate);

QI_TYPE_STRUCT(::qi::MetaMethodPrivate, uid, sigreturn, name, parametersSignature, description, parameters, returnDescription);
QI_TYPE_STRUCT_BOUNCE_REGISTER(::qi::MetaMethod, ::qi::MetaMethodPrivate, metaMethodPrivate);
