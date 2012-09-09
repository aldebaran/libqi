/*
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#ifndef _QIMESSAGING_METASIGNAL_HPP_
#define _QIMESSAGING_METASIGNAL_HPP_

#include <qimessaging/signature.hpp>
#include <qimessaging/datastream.hpp>

namespace qi {

  class MetaSignalPrivate;
  class Object;
  class EventLoop;
  class QIMESSAGING_API MetaSignal {
  public:
    MetaSignal(const std::string &sig);
    MetaSignal();
    MetaSignal(const MetaSignal &other);
    MetaSignal& operator=(const MetaSignal &other);
    ~MetaSignal();

    const std::string &signature() const;
    unsigned int       uid() const;

  protected:
  public:
    MetaSignalPrivate   *_p;
  };

  QIMESSAGING_API qi::ODataStream &operator<<(qi::ODataStream &stream, const MetaSignal &meta);
  QIMESSAGING_API qi::IDataStream &operator>>(qi::IDataStream &stream, MetaSignal &meta);
  QIMESSAGING_API qi::SignatureStream &operator&(qi::SignatureStream &stream, const MetaSignal &meta);

}; // namespace qi

#endif
