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

#include "src/buffer_p.hpp"
#include "src/message_p.hpp"


namespace qi {

#include <event2/buffer.h>

  const unsigned int messageMagic = 0x42adde42;

  unsigned int newMessageId()
  {
    static qi::atomic<long> id(0);
    ++id;
    return *id;
  }

  Message::Message()
    : _p(new qi::MessagePrivate())
  {
    memset(_p, 0, sizeof(MessagePrivate));
    _p->id = newMessageId();
    _p->magic = messageMagic;
    _p->buffer = new qi::Buffer();
    _p->deleteBuffer = true;
  }

  Message::Message(Buffer *buf)
    : _p(new qi::MessagePrivate())
  {
    memset(_p, 0, sizeof(MessagePrivate));
    _p->id = newMessageId();
    _p->magic = messageMagic;
    _p->buffer = buf;
    _p->deleteBuffer = false;
  }

  Message::Message(const Message &msg)
    : _p(new qi::MessagePrivate())
  {
    memcpy(_p, msg._p, sizeof(MessagePrivate));
    _p->deleteBuffer = false;
  }

  Message &Message::operator=(const Message &msg) {
    memcpy(_p, msg._p, sizeof(MessagePrivate));
    _p->buffer = msg._p->buffer;
    _p->deleteBuffer = false;
    return *this;
  }

  Message::~Message()
  {
    if (_p->deleteBuffer)
      delete _p->buffer;
    delete _p;
  }

  void Message::setBuffer(qi::Buffer *buffer) {
    if (_p->deleteBuffer)
      delete _p->buffer;
    _p->buffer = buffer;
    _p->deleteBuffer = false;
  }

  std::ostream& operator<<(std::ostream& os, const qi::Message& msg) {
    os << "message {"
       << "size=" << msg.size()
       << ", id=" << msg.id()
       << ", type=" << msg.type()
       << ", serv=" << msg.service()
       << ", path=" << msg.path()
       << ", func=" << msg.function()
       << "}";
    return os;
  }


  qi::DataStream& operator<<(qi::DataStream& os, const qi::Message& msg)
  {
    os << msg._p->magic
       << msg.id()
       << msg.size()
       << msg.type()
       << msg.service()
       << msg.path()
       << msg.function()
       << msg._p->reserved;

    qi::Buffer *buf = reinterpret_cast<qi::Buffer *>(os.ioDevice());
    evbuffer_add_buffer(buf->_p->data(), msg.buffer()->_p->data());
    return os;
  }

  unsigned int Message::size() const
  {
    return _p->buffer->size();
  }

  void Message::setId(uint32_t id)
  {
    _p->id = id;
  }

  unsigned int Message::id() const
  {
    return _p->id;
  }

  void Message::setType(uint32_t type)
  {
    _p->type = type;
  }

  unsigned int Message::type() const
  {
    return _p->type;
  }

  void Message::setService(uint32_t service)
  {
    _p->service = service;
  }

  unsigned int Message::service() const
  {
    return _p->service;
  }

  void Message::setPath(uint32_t path)
  {
    _p->path = path;
  }

  unsigned int Message::path() const
  {
    return _p->path;
  }

  void Message::setFunction(uint32_t function)
  {
    _p->function = function;
  }

  unsigned int Message::function() const
  {
    return _p->function;
  }

  qi::Buffer *Message::buffer() const
  {
    return _p->buffer;
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

  void Message::swap(Message &msg)
  {
    MessagePrivate *p = _p;
    _p = msg._p;
    msg._p = p;
  }

  bool Message::isValid() const
  {
    return (messageMagic == _p->magic);
  }
}
