/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <qimessaging/message.hpp>
#include <qimessaging/datastream.hpp>
#include <iostream>
#include <boost/atomic.hpp>
#include <qi/log.hpp>
#include <cassert>

#include "src/buffer_p.hpp"


namespace qi {
#include <event2/buffer.h>

  const unsigned int messageMagic = 0x42adde42;

  unsigned int newMessageId()
  {
    static boost::atomic_uint32_t id(0);
    id++;
    return id;
  }

  Message::Message()
  {
    _header = new qi::Message::MessageHeader();
    memset(_header, 0, sizeof(MessageHeader));
    _header->id = newMessageId();
    _header->magic = messageMagic;
    _buffer = new qi::Buffer();
    _withBuffer = false;
  }

  Message::Message(Buffer *buf)
  {
    _header = new qi::Message::MessageHeader();
    memset(_header, 0, sizeof(MessageHeader));
    _header->id = newMessageId();
    _header->magic = messageMagic;
    _buffer = buf;
    _withBuffer = true;
  }

  Message::Message(const Message &msg) {
    memcpy(_header, msg._header, sizeof(MessageHeader));
    _buffer = msg._buffer;
    _withBuffer = true;
  }

  Message &Message::operator=(const Message &msg) {
    memcpy(_header, msg._header, sizeof(MessageHeader));
    _buffer = msg._buffer;
    _withBuffer = true;
    return *this;
  }

  Message::~Message()
  {
    if (!_withBuffer)
      delete _buffer;
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
    os << msg._header->magic
       << msg.id()
       << msg.size()
       << msg.type()
       << msg.service()
       << msg.path()
       << msg.function()
       << msg._header->reserved;

    qi::Buffer *buf = reinterpret_cast<qi::Buffer *>(os.ioDevice());
    evbuffer_add_buffer(buf->_p->data(), msg.buffer()->_p->data());
    return os;
  }

  unsigned int Message::size() const
  {
    return _buffer->size();
  }

  void Message::setId(uint32_t id)
  {
    _header->id = id;
  }

  unsigned int Message::id() const
  {
    return _header->id;
  }

  void Message::setType(uint32_t type)
  {
    _header->type = type;
  }

  unsigned int Message::type() const
  {
    return _header->type;
  }

  void Message::setService(uint32_t service)
  {
    _header->service = service;
  }

  unsigned int Message::service() const
  {
    return _header->service;
  }

  void Message::setPath(uint32_t path)
  {
    _header->path = path;
  }

  unsigned int Message::path() const
  {
    return _header->path;
  }

  void Message::setFunction(uint32_t function)
  {
    _header->function = function;
  }

  unsigned int Message::function() const
  {
    return _header->function;
  }

  qi::Buffer *Message::buffer() const
  {
    return _buffer;
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

  bool Message::checkMagic()
  {
    assert(messageMagic == _header->magic);
    return (messageMagic == _header->magic);
  }
}
