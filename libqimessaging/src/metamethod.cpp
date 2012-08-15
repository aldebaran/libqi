/*
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include "src/metamethod_p.hpp"
#include <qimessaging/metamethod.hpp>

namespace qi {

  MetaMethod::MetaMethod()
    : _p(new MetaMethodPrivate())
  {
  }

  MetaMethod::MetaMethod(const std::string &sigret, const std::string &sig, const qi::Functor *functor)
    : _p(new MetaMethodPrivate(sigret, sig, functor))
  {
  }

  MetaMethod::MetaMethod(const MetaMethod &other)
    : _p(new MetaMethodPrivate())
  {
    *_p = *(other._p);
  }

  MetaMethod& MetaMethod::operator=(const MetaMethod &other)
  {
    *_p = *(other._p);
    return (*this);
  }

  MetaMethod::~MetaMethod()
  {
    delete _p;
  }

  const std::string &MetaMethod::signature() const
  {
    return _p->signature();
  }

  const std::string &MetaMethod::sigreturn() const
  {
    return _p->sigreturn();
  }

  const qi::Functor *MetaMethod::functor() const
  {
    return _p->_functor.get();
  }

  unsigned int       MetaMethod::uid() const
  {
    return _p->uid();
  }

  qi::ODataStream &operator<<(qi::ODataStream &stream, const MetaMethod &meta) {
    stream << meta._p->_signature;
    stream << meta._p->_sigret;
    stream << meta._p->_uid;
    return stream;
  }

  qi::IDataStream &operator>>(qi::IDataStream &stream, MetaMethod &meta) {
    stream >> meta._p->_signature;
    stream >> meta._p->_sigret;
    stream >> meta._p->_uid;
    return stream;
  }

  qi::SignatureStream &operator&(qi::SignatureStream &stream, const MetaMethod &meta) {
    stream & meta._p->_signature;
    stream & meta._p->_sigret;
    stream & meta._p->_uid;
    return stream;
  }

};
