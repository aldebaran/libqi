/*
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#ifndef _QIMESSAGING_METAEVENT_HPP_
#define _QIMESSAGING_METAEVENT_HPP_

namespace qi {

  class MetaEventPrivate;
  class QIMESSAGING_API MetaEvent {
  public:
    MetaEvent(const std::string &sig);
    MetaEvent();
    MetaEvent(const MetaEvent &other);
    MetaEvent& operator=(const MetaEvent &other);
    ~MetaEvent();

    const std::string &signature() const;
    unsigned int       index() const;

  protected:
  public:
    MetaEventPrivate   *_p;
  };

  QIMESSAGING_API qi::DataStream &operator<<(qi::DataStream &stream, const MetaEvent &meta);
  QIMESSAGING_API qi::DataStream &operator>>(qi::DataStream &stream, MetaEvent &meta);

}; // namespace qi

#endif
