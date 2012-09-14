/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <iostream>
#include <cassert>
#include <cstring>

#include <qimessaging/metavalue.hpp>
#include <qimessaging/message.hpp>
#include <qimessaging/datastream.hpp>

#include <qi/atomic.hpp>
#include <qi/log.hpp>
#include <qi/types.hpp>

#include "src/message_p.hpp"


namespace qi {

  unsigned int newMessageId()
  {
    static qi::atomic<long> id(0);
    return ++id;
  }

  MessagePrivate::MessagePrivate()
  {
    memset(&header, 0, sizeof(MessagePrivate::MessageHeader));
    header.id = newMessageId();
    header.magic = qi::MessagePrivate::magic;
  }

  MessagePrivate::~MessagePrivate()
  {
  }

  Message::Message()
    : _p(new qi::MessagePrivate())
  {

  }
  Message::Message(const Message &msg)
    : _p(new qi::MessagePrivate())
  {
    this->operator =(msg);
  }

  Message& Message::operator=(const Message& msg)
  {
    _p->buffer = msg._p->buffer;
    memcpy(&(_p->header), &(msg._p->header), sizeof(MessagePrivate::MessageHeader));
    return *this;
  }

  Message::~Message()
  {
    delete _p;
  }

  std::ostream& operator<<(std::ostream& os, const qi::Message& msg) {
    os << "message {" << std::endl
       << "  size=" << msg._p->header.size << "," << std::endl
       << "  id  =" << msg.id() << "," << std::endl
       << "  vers=" << msg.version() << "," << std::endl
       << "  type=" << msg.type() << "," << std::endl
       << "  serv=" << msg.service() << "," << std::endl
       << "  path=" << msg.object() << "," << std::endl
       << "  acti=" << msg.action() << "," << std::endl
       << "  data=" << std::endl;
    msg._p->buffer.dump();
    os << "}";
    return os;
  }

  void Message::setId(qi::uint32_t id)
  {
    _p->header.id = id;
  }

  unsigned int Message::id() const
  {
    return _p->header.id;
  }

  void Message::setVersion(qi::uint16_t version)
  {
    _p->header.version = version;
  }

  unsigned int Message::version() const
  {
    return _p->header.version;
  }

  void Message::setType(qi::uint16_t type)
  {
    _p->header.type = type;
  }

  unsigned int Message::type() const
  {
    return _p->header.type;
  }

  void Message::setService(qi::uint32_t service)
  {
    _p->header.service = service;
  }

  unsigned int Message::service() const
  {
    return _p->header.service;
  }

  void Message::setObject(qi::uint32_t object)
  {
    _p->header.object = object;
  }

  unsigned int Message::object() const
  {
    return _p->header.object;
  }

  void Message::setFunction(qi::uint32_t function)
  {
    if (type() == Type_Event)
    {
      qiLogDebug("Message") << "called setFunction() on Type_Event message";
    }
    _p->header.action = function;
  }

  unsigned int Message::function() const
  {
    if (type() == Type_Event)
    {
      qiLogDebug("Message") << "called function() on Type_Event message";
    }
    return _p->header.action;
  }

  void Message::setEvent(qi::uint32_t event)
  {
    if (type() != Type_Event)
    {
      qiLogDebug("Message") << "called setEvent() on non Type_Event message";
    }
    _p->header.action = event;
  }

  unsigned int Message::event() const
  {
    if (type() != Type_Event)
    {
      qiLogDebug("Message") << "called event() on non Type_Event message";
    }
    return _p->header.action;
  }

  unsigned int Message::action() const
  {
    return _p->header.action;
  }

  void Message::setBuffer(const Buffer &buffer)
  {
    _p->buffer = buffer;
  }

  const qi::Buffer &Message::buffer() const
  {
    return _p->buffer;
  }

  void Message::buildReplyFrom(const Message &call)
  {
    setId(call.id());
    setType(qi::Message::Type_Reply);
    setService(call.service());
    setObject(call.object());
    setFunction(call.function());
  }

  void Message::buildForwardFrom(const Message &msg)
  {
    setType(msg.type());
    setService(msg.service());
    setObject(msg.object());
    setFunction(msg.function());
  }

  bool Message::isValid()
  {
    if (_p->header.magic != qi::MessagePrivate::magic)
    {
      qiLogError("qimessaging.TransportSocket")  << "Message dropped (magic is incorrect)" << std::endl;
      return false;
    }

    if (type() == qi::Message::Type_None)
    {
      qiLogError("qimessaging.TransportSocket")  << "Message dropped (type is None)" << std::endl;
      return false;
    }

    if (object() == qi::Message::Object_None)
    {
      qiLogError("qimessaging.TransportSocket")  << "Message dropped (object is 0)" << std::endl;
      assert(object() != qi::Message::Object_None);
      return false;
    }

    return true;
  }

  MessageAddress Message::address() const {
    return MessageAddress(_p->header.type, _p->header.id, _p->header.service, _p->header.object, _p->header.action);
  }

}
