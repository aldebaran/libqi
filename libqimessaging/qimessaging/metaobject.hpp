/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#ifndef _QIMESSAGING_METAOBJECT_HPP_
#define _QIMESSAGING_METAOBJECT_HPP_

#include <qimessaging/datastream.hpp>
#include <qimessaging/metamethod.hpp>
#include <qimessaging/metasignal.hpp>

namespace qi {

  class MetaObjectPrivate;
  /// Description of the signals and methods accessible on an ObjectType
  class QIMESSAGING_API MetaObject  {
  public:
    MetaObject();
    MetaObject(const MetaObject &other);
    MetaObject& operator=(const MetaObject &other);
    ~MetaObject();

    int methodId(const std::string &name) const;
    int signalId(const std::string &name) const;

    typedef std::map<unsigned int, MetaMethod> MethodMap;
    MethodMap methodMap() const;

    //not called signals because it conflict with Qt keywords :S
    typedef std::map<unsigned int, MetaSignal> SignalMap;
    SignalMap signalMap() const;

    MetaMethod*       method(unsigned int id);
    const MetaMethod* method(unsigned int id) const;

    MetaSignal*       signal(unsigned int id);
    const MetaSignal* signal(unsigned int id) const;

    std::vector<MetaMethod> findMethod(const std::string &name) const;
    std::vector<MetaSignal> findSignal(const std::string &name) const;

    MetaObjectPrivate   *_p;
  };

   QIMESSAGING_API qi::ODataStream &operator<<(qi::ODataStream &stream, const MetaObject &meta);
  QIMESSAGING_API qi::IDataStream &operator>>(qi::IDataStream &stream, MetaObject &meta);
  QIMESSAGING_API qi::SignatureStream &operator&(qi::SignatureStream &stream, const MetaObject &meta);

};

#endif  // _QIMESSAGING_METAOBJECT_HPP_
