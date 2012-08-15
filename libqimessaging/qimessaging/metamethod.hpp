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
    MetaMethod(const std::string &sigret, const std::string &sig, const qi::Functor *functor);
    MetaMethod();
    MetaMethod(const MetaMethod &other);
    MetaMethod& operator=(const MetaMethod &other);
    ~MetaMethod();

    const std::string &signature() const;
    const std::string &sigreturn() const;
    const qi::Functor *functor() const;
    unsigned int       uid() const;

  protected:
  public:
    MetaMethodPrivate   *_p;
  };

  QIMESSAGING_API qi::ODataStream &operator<<(qi::ODataStream &stream, const MetaMethod &meta);
  QIMESSAGING_API qi::IDataStream &operator>>(qi::IDataStream &stream, MetaMethod &meta);
  QIMESSAGING_API qi::SignatureStream &operator&(qi::SignatureStream &stream, const MetaMethod &meta);

}; // namespace qi


#endif // __METAMETHOD_HPP__
