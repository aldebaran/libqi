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
#include <qimessaging/metaevent.hpp>

namespace qi {

  class MetaObjectPrivate;
  class QIMESSAGING_API MetaObject {
  public:
    MetaObject();
    MetaObject(const MetaObject &other);
    MetaObject& operator=(const MetaObject &other);
    ~MetaObject();

    int methodId(const std::string &name);
    int eventId(const std::string &name);

    typedef std::map<unsigned int, MetaMethod> MethodMap;
    MethodMap methods() const;

    typedef std::map<unsigned int, MetaEvent> EventMap;
    EventMap events() const;

    MetaMethod *method(unsigned int id);
    const MetaMethod *method(unsigned int id) const;

    MetaEvent *event(unsigned int id);
    const MetaEvent *event(unsigned int id) const;

    std::vector<MetaMethod> findMethod(const std::string &name);
    std::vector<MetaEvent> findEvent(const std::string &name);

    MetaObjectPrivate   *_p;
  };

  QIMESSAGING_API qi::ODataStream &operator<<(qi::ODataStream &stream, const MetaObject &meta);
  QIMESSAGING_API qi::IDataStream &operator>>(qi::IDataStream &stream, MetaObject &meta);
  QIMESSAGING_API qi::SignatureStream &operator&(qi::SignatureStream &stream, const MetaObject &meta);

};

#endif /* !METAOBJECT_PP_ */
