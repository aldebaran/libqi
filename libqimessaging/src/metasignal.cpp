/*
**
** Copyright (C) 2012 Aldebaran Robotics
*/


#include "src/metasignal_p.hpp"
#include <qimessaging/metasignal.hpp>
#include <qimessaging/object.hpp>
namespace qi {

  MetaSignal::MetaSignal()
    : _p(new MetaSignalPrivate())
  {
  }

  MetaSignal::MetaSignal(const std::string &sig)
    : _p(new MetaSignalPrivate(sig))
  {
  }

  MetaSignal::MetaSignal(const MetaSignal &other)
    : _p(new MetaSignalPrivate())
  {
    *_p = *(other._p);
  }

  MetaSignal& MetaSignal::operator=(const MetaSignal &other)
  {
    *_p = *(other._p);
    return (*this);
  }

  MetaSignal::~MetaSignal()
  {
    delete _p;
  }

  const std::string &MetaSignal::signature() const
  {
    return _p->signature();
  }

  unsigned int       MetaSignal::uid() const
  {
    return _p->uid();
  }


  qi::ODataStream &operator<<(qi::ODataStream &stream, const MetaSignal &meta) {
    stream << meta._p->_signature;
    stream << meta._p->_uid;
    return stream;
  }

  qi::IDataStream &operator>>(qi::IDataStream &stream, MetaSignal &meta) {
    stream >> meta._p->_signature;
    stream >> meta._p->_uid;
    return stream;
  }

  qi::SignatureStream &operator&(qi::SignatureStream &stream, const MetaSignal &meta) {
    stream & meta._p->_signature;
    stream & meta._p->_uid;
    return stream;
  }

};
