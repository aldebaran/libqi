/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#ifndef    METAOBJECT_HPP_
# define   METAOBJECT_HPP_

#include <qimessaging/datastream.hpp>
#include <qimessaging/metamethod.hpp>
#include <qimessaging/metasignal.hpp>

namespace qi {

  class MetaObjectPrivate;
  class QIMESSAGING_API MetaObject {
  public:
    MetaObject();
    MetaObject(const MetaObject &other);
    MetaObject& operator=(const MetaObject &other);
    ~MetaObject();

    int methodId(const std::string &name);
    int signalId(const std::string &name);

    typedef std::map<unsigned int, MetaMethod> MethodMap;
    MethodMap methodMap() const;

    //not called signals because it conflict with Qt keywords :S
    typedef std::map<unsigned int, MetaSignal> SignalMap;
    SignalMap signalMap() const;

    MetaMethod*       method(unsigned int id);
    const MetaMethod* method(unsigned int id) const;

    MetaSignal*       signal(unsigned int id);
    const MetaSignal* signal(unsigned int id) const;

    std::vector<MetaMethod> findMethod(const std::string &name);
    std::vector<MetaSignal> findSignal(const std::string &name);

    MetaObjectPrivate   *_p;
  };

  QIMESSAGING_API qi::ODataStream &operator<<(qi::ODataStream &stream, const MetaObject &meta);
  QIMESSAGING_API qi::IDataStream &operator>>(qi::IDataStream &stream, MetaObject &meta);
  QIMESSAGING_API qi::SignatureStream &operator&(qi::SignatureStream &stream, const MetaObject &meta);

};

#endif /* !METAOBJECT_PP_ */
