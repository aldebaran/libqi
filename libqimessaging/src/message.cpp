/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <iostream>
#include <cassert>
#include <cstring>

#include <qimessaging/message.hpp>
#include <qimessaging/datastream.hpp>

#include <qi/atomic.hpp>
#include <qi/log.hpp>

#include "src/message_p.hpp"


namespace qi {

  const unsigned int messageMagic = 0x42adde42;

  unsigned int newMessageId()
  {
    static qi::atomic<long> id(0);
    ++id;
    return *id;
  }

  void MessagePrivate::complete()
  {
    header.size = buffer.size();
  }

  MessagePrivate::MessagePrivate()
  {
    memset(&header, 0, sizeof(MessagePrivate::MessageHeader));
    header.id = newMessageId();
    header.magic = messageMagic;
  }

  MessagePrivate::~MessagePrivate()
  {
  }

  Message::Message()
    : _p(new qi::MessagePrivate())
  {
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
       << "  type=" << msg.type() << "," << std::endl
       << "  serv=" << msg.service() << "," << std::endl
       << "  path=" << msg.path() << "," << std::endl
       << "  func=" << msg.function() << "," << std::endl
       << "  data=" << std::endl;
    msg._p->buffer.dump();
    os << "}";
    return os;
  }

  void Message::setId(uint32_t id)
  {
    _p->header.id = id;
  }

  unsigned int Message::id() const
  {
    return _p->header.id;
  }

  void Message::setType(uint32_t type)
  {
    _p->header.type = type;
  }

  unsigned int Message::type() const
  {
    return _p->header.type;
  }

  void Message::setService(uint32_t service)
  {
    _p->header.service = service;
  }

  unsigned int Message::service() const
  {
    return _p->header.service;
  }

  void Message::setPath(uint32_t path)
  {
    _p->header.path = path;
  }

  unsigned int Message::path() const
  {
    return _p->header.path;
  }

  void Message::setFunction(uint32_t function)
  {
    _p->header.function = function;
  }

  unsigned int Message::function() const
  {
    return _p->header.function;
  }

  void Message::setBuffer(const Buffer &buffer)
  {
    _p->buffer = buffer;
  }

  const qi::Buffer &Message::buffer() const
  {
    return _p->buffer;
  }

  void *MessagePrivate::getHeader()
  {
    return reinterpret_cast<void *>(&header);
  }

  void Message::buildReplyFrom(const Message &call)
  {
    setId(call.id());
    setType(qi::Message::Type_Reply);
    setService(call.service());
    setPath(call.path());
    setFunction(call.function());
  }

  void Message::buildForwardFrom(const Message &msg)
  {
    setType(msg.type());
    setService(msg.service());
    setPath(msg.path());
    setFunction(msg.function());
  }

  bool Message::isValid()
  {
    if (_p->header.magic != messageMagic)
    {
      qiLogError("qimessaging.TransportSocket")  << "Message dropped (magic is incorrect)" << std::endl;
      assert(_p->header.magic == messageMagic);
      return false;
    }

    if (type() == qi::Message::Type_None)
    {
      qiLogError("qimessaging.TransportSocket")  << "Message dropped (type is None)" << std::endl;
      assert(type() != qi::Message::Type_None);
      return false;
    }

    if (service() == qi::Message::Service_None)
    {
      qiLogError("qimessaging.TransportSocket")  << "Message dropped (service is 0)" << std::endl;
      assert(service() != qi::Message::Service_None);
      return false;
    }

    if (path() == qi::Message::Path_None)
    {
      qiLogError("qimessaging.TransportSocket")  << "Message dropped (path is 0)" << std::endl;
      assert(path() != qi::Message::Path_None);
      return false;
    }

    return true;
  }
}
