/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/
#include "metasignal_p.hpp"
#include <qitype/metasignal.hpp>
#include <qitype/genericobject.hpp>
namespace qi {

  MetaSignal::MetaSignal()
  {
  }

  MetaSignal::MetaSignal(unsigned int uid, const std::string &name, const Signature &sig)
  : _uid(uid)
  , _name(name)
  , _signature(sig)
  {
  }


  MetaSignal::~MetaSignal()
  {
  }

  const std::string &MetaSignal::name() const
  {
    return _name;
  }

  std::string MetaSignal::toString() const
  {
    return _name + "::" + _signature.toString();
  }

  const qi::Signature &MetaSignal::parametersSignature() const
  {
    return _signature;
  }


  unsigned int       MetaSignal::uid() const
  {
    return _uid;
  }

  bool MetaSignal::isPrivate() const {
    return MetaObject::isPrivateMember(name(), uid());
  }

};
