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

  const unsigned int messageMagic = 0x42adde42;

  unsigned int newMessageId()
  {
    static qi::atomic<long> id(0);
    ++id;
    return *id;
  }

  void MessagePrivate::sentcb(const void *data, size_t datalen, void *msg)
  {
    Message *m = static_cast<Message *>(msg);
    // FIXME: should clean stuff, and release buffer
  }

  void MessagePrivate::complete()
  {
    header->size = buffer->size();
  }

  MessagePrivate::MessagePrivate()
  {
    header = new MessagePrivate::MessageHeader();
    memset(header, 0, sizeof(MessagePrivate::MessageHeader));
    header->id = newMessageId();
    header->magic = messageMagic;
    buffer = 0;
  }

  MessagePrivate::~MessagePrivate()
  {
    // FIXME: should release buffer
  }

  Message::Message(int flags)
    : _p(new qi::MessagePrivate())
  {
    if ((flags & Create_WithBuffer) == Create_WithBuffer)
    {
      _p->buffer = new qi::Buffer();
    }
  }

  Message::Message(Buffer *buf)
    : _p(new qi::MessagePrivate())
  {
    _p->buffer = buf;
  }

  Message& Message::operator=(const Message& msg)
  {
    _p->buffer = msg._p->buffer;
    memcpy(_p->header, msg._p->header, sizeof(MessagePrivate::MessageHeader));
    return *this;
  }

  Message::~Message()
  {
    delete _p;
  }

  std::ostream& operator<<(std::ostream& os, const qi::Message& msg) {
    os << "message {" << std::endl
       << "  size=" << msg.size() << "," << std::endl
       << "  id  =" << msg.id() << "," << std::endl
       << "  type=" << msg.type() << "," << std::endl
       << "  serv=" << msg.service() << "," << std::endl
       << "  path=" << msg.path() << "," << std::endl
       << "  func=" << msg.function() << "," << std::endl
       << "  data=" << std::endl;
    msg._p->buffer->dump();
    os << "}";
    return os;
  }

  unsigned int Message::size() const
  {
    return _p->buffer->size();
  }

  void Message::setId(uint32_t id)
  {
    _p->header->id = id;
  }

  unsigned int Message::id() const
  {
    return _p->header->id;
  }

  void Message::setType(uint32_t type)
  {
    _p->header->type = type;
  }

  unsigned int Message::type() const
  {
    return _p->header->type;
  }

  void Message::setService(uint32_t service)
  {
    _p->header->service = service;
  }

  unsigned int Message::service() const
  {
    return _p->header->service;
  }

  void Message::setPath(uint32_t path)
  {
    _p->header->path = path;
  }

  unsigned int Message::path() const
  {
    return _p->header->path;
  }

  void Message::setFunction(uint32_t function)
  {
    _p->header->function = function;
  }

  unsigned int Message::function() const
  {
    return _p->header->function;
  }

  void Message::setBuffer(Buffer *buffer)
  {
    _p->buffer = buffer;
  }

  qi::Buffer *Message::buffer() const
  {
    return _p->buffer;
  }

  void *Message::header() const
  {
    return _p->header;
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
    if (_p->header->magic != messageMagic)
    {
      qiLogError("qimessaging.TransportSocket")  << "Message dropped (magic is incorrect)" << std::endl;
      assert(_p->header->magic == messageMagic);
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
