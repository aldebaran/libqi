/*
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#ifndef _QIMESSAGING_METAEVENT_HPP_
#define _QIMESSAGING_METAEVENT_HPP_

#include <qimessaging/signature.hpp>

namespace qi {

  class MetaEventPrivate;
  class Object;
  class EventLoop;
  class QIMESSAGING_API MetaEvent {
  public:
    MetaEvent(const std::string &sig);
    MetaEvent();
    MetaEvent(const MetaEvent &other);
    MetaEvent& operator=(const MetaEvent &other);
    ~MetaEvent();

    const std::string &signature() const;
    unsigned int       uid() const;

  protected:
  public:
    MetaEventPrivate   *_p;
  };

  QIMESSAGING_API qi::ODataStream &operator<<(qi::ODataStream &stream, const MetaEvent &meta);
  QIMESSAGING_API qi::IDataStream &operator>>(qi::IDataStream &stream, MetaEvent &meta);
  QIMESSAGING_API qi::SignatureStream &operator&(qi::SignatureStream &stream, const MetaEvent &meta);

}; // namespace qi

#endif
