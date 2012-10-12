/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/
#include <iostream>
#include <cassert>
#include <cstring>

#include <qitype/genericvalue.hpp>
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

  Message::Message(Type type, const MessageAddress &address)
    : _p(new qi::MessagePrivate())
  {
    setType(type);
    setAddress(address);
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

  void Message::setType(Message::Type type)
  {
    _p->header.type = type;
  }

  Message::Type Message::type() const
  {
    return static_cast<Message::Type>(_p->header.type);
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

  void Message::setParameters(const GenericFunctionParameters &parameters)
  {
    ODataStream out(_p->buffer);
    for (unsigned int i = 0; i < parameters.size(); ++i)
      qi::details::serialize(parameters[i], out);
  }

  GenericFunctionParameters Message::parameters(const qi::Signature &sig) const {
    GenericFunctionParameters result;
    IDataStream in(_p->buffer);
    Signature::iterator it = sig.begin();
    while (it != sig.end())
    {
      qi::Type* compatible = qi::Type::fromSignature(*it);
      if (!compatible)
      {
        qiLogError("qi.GenericFunctionParameters") <<"fromBuffer: unknown type " << *it;
        throw std::runtime_error("Could not construct type for " + *it);
      }
      result.push_back(qi::details::deserialize(compatible, in));
      ++it;
    }
    return result;
  }

  const qi::Buffer &Message::buffer() const
  {
    return _p->buffer;
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

    if (object() == qi::Message::GenericObject_None)
    {
      qiLogError("qimessaging.TransportSocket")  << "Message dropped (object is 0)" << std::endl;
      assert(object() != qi::Message::GenericObject_None);
      return false;
    }

    return true;
  }

  void Message::setAddress(const MessageAddress &address) {
    _p->header.id = address.messageId;
    _p->header.service = address.serviceId;
    _p->header.object = address.objectId;
    _p->header.action = address.functionId;
  }

  MessageAddress Message::address() const {
    return MessageAddress(_p->header.id, _p->header.service, _p->header.object, _p->header.action);
  }

  std::ostream &operator<<(std::ostream &os, const qi::MessageAddress &address) {
    os << "{" << address.serviceId << "." << address.objectId << "." << address.functionId << ", id:" << address.messageId << "}";
    return os;
  }

}

