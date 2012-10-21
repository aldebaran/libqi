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

  MetaSignal::MetaSignal(unsigned int uid, const std::string &sig)
  : _uid(uid)
  , _signature(sig)
  {
  }


  MetaSignal::~MetaSignal()
  {
  }

  const std::string &MetaSignal::signature() const
  {
    return _signature;
  }

  unsigned int       MetaSignal::uid() const
  {
    return _uid;
  }

};
