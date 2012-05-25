/*
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#ifndef _QIMESSAGING_METAMETHOD_HPP_
#define _QIMESSAGING_METAMETHOD_HPP_

namespace qi {

  class MetaMethodPrivate;
  class QIMESSAGING_API MetaMethod {
  public:
    MetaMethod(const std::string &sig, const qi::Functor *functor);
    MetaMethod();
    ~MetaMethod();

    const std::string &signature() const;
    const qi::Functor *functor() const;
    unsigned int       index() const;

  protected:
  public:
    MetaMethodPrivate   *_p;
  };

  QIMESSAGING_API qi::DataStream &operator<<(qi::DataStream &stream, const MetaMethod &meta);
  QIMESSAGING_API qi::DataStream &operator>>(qi::DataStream &stream, MetaMethod &meta);

}; // namespace qi

#endif // __METAMETHOD_HPP__