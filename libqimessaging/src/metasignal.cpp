/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/
#include "src/metasignal_p.hpp"
#include <qimessaging/metasignal.hpp>
#include <qimessaging/genericobject.hpp>
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


  qi::ODataStream &operator<<(qi::ODataStream &stream, const MetaSignal &meta) {
    stream << meta.signature();
    stream << meta.uid();
    return stream;
  }

  qi::IDataStream &operator>>(qi::IDataStream &stream, MetaSignal &meta) {
    std::string sig; unsigned int uid;
    stream >> sig >> uid;
    meta = MetaSignal(uid, sig);
    return stream;
  }

  qi::SignatureStream &operator&(qi::SignatureStream &stream, const MetaSignal &meta) {
    stream & meta.signature();
    stream & meta.uid();
    return stream;
  }

};
