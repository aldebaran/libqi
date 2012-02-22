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

namespace qi {

  unsigned int newMessageId()
  {
    static boost::atomic_uint32_t id(0);
    id++;
    return id;
  }

  Message::Message()
  {
    _header = new qi::Message::MessageHeader();
    _header->id = newMessageId();
    _buffer = new qi::Buffer();
    _withBuffer = false;
    memset(_header, 0, sizeof(MessageHeader));
  }

  Message::Message(Buffer *buf)
  {
    _header = new qi::Message::MessageHeader();
    _header->id = newMessageId();
    _buffer = buf;
    _withBuffer = true;
    memset(_header, 0, sizeof(MessageHeader));
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

  size_t Message::size() const
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

  // write header into msg before send.
  bool Message::complete()
  {
    if (type() == qi::Message::None)
    {
      qiLogError("qimessaging.TransportSocket")  << "Message dropped (type is None)" << std::endl;
      assert(type() != qi::Message::None);
      return false;
    }

    if (service() == 0)
    {
      qiLogError("qimessaging.TransportSocket")  << "Message dropped (service is 0)" << std::endl;
      assert(service() != 0);
      return false;
    }

    _header->size = _buffer->size();
    _buffer->prepend(_header, sizeof(MessageHeader));

    return true;
  }

  void Message::buildReplyFrom(const Message &call)
  {
    setId(call.id());
    setType(qi::Message::Reply);
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
}
