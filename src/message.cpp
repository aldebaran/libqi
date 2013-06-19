/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/
#include <iostream>
#include <cassert>
#include <cstring>

#include <boost/make_shared.hpp>
#include <boost/dynamic_bitset.hpp>

#include <qitype/genericvalue.hpp>
#include "message.hpp"

#include <qi/atomic.hpp>
#include <qi/log.hpp>
#include <boost/cstdint.hpp>
#include <qi/types.hpp>
#include <qi/buffer.hpp>

#include <qitype/binarycodec.hpp>

#include "boundobject.hpp"
#include "remoteobject_p.hpp"

qiLogCategory("qimessaging.message");

namespace qi {

  unsigned int newMessageId()
  {
    static qi::Atomic<int> id(0);
    return ++id;
  }

  const char* Message::typeToString(Type t)
  {
    switch (t)
    {
    case Type_None:
      return "None";
    case Type_Call:
      return "Call";
    case Type_Reply:
      return "Reply";
    case Type_Error:
      return "Error";
    case Type_Post:
      return "Post";
    case Type_Event:
      return "Event";
    default:
      return "Unknown";
    }
  }

  const char* Message::actionToString(unsigned int action, unsigned int service)
  {
    switch (action)
    {
    case BoundObjectFunction_RegisterEvent:
      return "RegisterEvent";
    case BoundObjectFunction_UnregisterEvent:
      return "UnregisterEvent";
    case BoundObjectFunction_MetaObject:
      return "MetaObject";
    case BoundObjectFunction_Terminate:
      return "Terminate";
    case BoundObjectFunction_GetProperty:
      return "GetProperty";
    case BoundObjectFunction_SetProperty:
      return "SetProperty";
    case BoundObjectFunction_Properties:
      return "Properties";
    }

    if (service != qi::Message::Service_ServiceDirectory)
    {
      return 0;
    }

    switch (action)
    {
    case ServiceDirectoryAction_Service:
      return "Service";
    case ServiceDirectoryAction_Services:
      return "Services";
    case ServiceDirectoryAction_RegisterService:
      return "RegisterService";
    case ServiceDirectoryAction_UnregisterService:
      return "UnregisterService";
    case ServiceDirectoryAction_ServiceReady:
      return "ServiceReady";
    case ServiceDirectoryAction_UpdateServiceInfo:
      return "UpdateServiceInfo";
    case ServiceDirectoryAction_ServiceAdded:
      return "ServiceAdded";
    case ServiceDirectoryAction_ServiceRemoved:
      return "ServiceRemoved";
    default:
      return 0;
    }
  }

  MessagePrivate::MessagePrivate()
  {
    memset(&header, 0, sizeof(MessagePrivate::MessageHeader));
    header.version = qi::Message::currentVersion();
    header.id = newMessageId();
    header.magic = qi::MessagePrivate::magic;
  }

  MessagePrivate::MessagePrivate(const MessagePrivate& b)
  : buffer(b.buffer)
  , signature(b.signature)
  , header(b.header)
  {
  }

  MessagePrivate::~MessagePrivate()
  {
  }

  Message::Message()
    : _p(boost::make_shared<MessagePrivate>())
  {

  }
  Message::Message(const Message &msg)
    : _p(msg._p)
  {
  }

  void Message::cow()
  {
    if (_p.use_count() > 1)
      _p = boost::make_shared<MessagePrivate>(*_p.get());
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
  }


  std::ostream& operator<<(std::ostream& os, const qi::Message& msg) {
    os << "message {" << std::endl
       << "  size=" << msg._p->header.size << "," << std::endl
       << "  id  =" << msg.id() << "," << std::endl
       << "  vers=" << msg.version() << "," << std::endl
       << "  type=" << qi::Message::typeToString(msg.type()) << "," << std::endl
       << "  serv=";

    if (msg.service() == qi::Message::Service_ServiceDirectory)
    {
      os << "ServiceDirectory";
    }
    else
    {
      os << msg.service();
    }

    os << "," << std::endl
       << "  obje=";

    if (msg.object() == qi::Message::GenericObject_Main)
    {
      os << "main";
    }
    else
    {
      os << msg.object();
    }

    os << std::endl << "  acti=";

    const char* s = qi::Message::actionToString(msg.action(), msg.service());
    if (s != 0)
    {
      os << s;
    }
    else
    {
      os << msg.action();
    }

    os << "," << std::endl
       << "  data=" << std::endl;
    qi::details::printBuffer(os, msg._p->buffer);
    os << std::endl << "}";
    return os;
  }

  void Message::setId(qi::uint32_t id)
  {
    cow();
    _p->header.id = id;
  }

  unsigned int Message::id() const
  {
    return _p->header.id;
  }

  void Message::setVersion(qi::uint16_t version)
  {
    cow();
    _p->header.version = version;
  }

  unsigned int Message::version() const
  {
    return _p->header.version;
  }

  void Message::setType(Message::Type type)
  {
    cow();
    _p->header.type = type;
  }

  Message::Type Message::type() const
  {
    return static_cast<Message::Type>(_p->header.type);
  }

  void Message::setService(qi::uint32_t service)
  {
    cow();
    _p->header.service = service;
  }

  unsigned int Message::service() const
  {
    return _p->header.service;
  }

  void Message::setObject(qi::uint32_t object)
  {
    cow();
    _p->header.object = object;
  }

  unsigned int Message::object() const
  {
    return _p->header.object;
  }

  void Message::setFunction(qi::uint32_t function)
  {
    cow();
    if (type() == Type_Event)
    {
      qiLogDebug() << "called setFunction() on Type_Event message";
    }
    _p->header.action = function;
  }

  unsigned int Message::function() const
  {
    if (type() == Type_Event)
    {
      qiLogDebug() << "called function() on Type_Event message";
    }
    return _p->header.action;
  }

  void Message::setEvent(qi::uint32_t event)
  {
    cow();
    if (type() != Type_Event)
    {
      qiLogDebug() << "called setEvent() on non Type_Event message";
    }
    _p->header.action = event;
  }

  unsigned int Message::event() const
  {
    if (type() != Type_Event)
    {
      qiLogDebug() << "called event() on non Type_Event message";
    }
    return _p->header.action;
  }

  unsigned int Message::action() const
  {
    return _p->header.action;
  }

  void Message::setBuffer(const Buffer &buffer)
  {
    cow();
    _p->buffer = buffer;
  }

  void Message::setError(const std::string &error) {
    if (type() != Type_Error) {
      qiLogWarning() << "called setError on a non Type_Error message";
      return;
    }
    // Error message is of type m (dynamic)
    GenericValue v(AnyReference(error), false, false);
    AnyReference vr(v);
    setValue(vr);
  }

  namespace {
    ObjectSerializationInfo serializeObject(
      AnyObject object,
      ObjectHost* context)
    {
      if (!context)
        throw std::runtime_error("Unable to serialize object without a valid ObajectHost");
      unsigned int sid = context->service();
      unsigned int oid = context->nextId();
      ServiceBoundObject* sbo = new ServiceBoundObject(sid, oid, object, MetaCallType_Queued, true, context);
      boost::shared_ptr<BoundObject> bo(sbo);
      context->addObject(bo, oid);
      qiLogDebug() << "Hooking " << oid <<" on " << context;
      qiLogDebug() << "sbo " << sbo << "obj " << object.get();
      // Transmit the metaObject augmented by ServiceBoundObject.
      ObjectSerializationInfo res;
      res.metaObject = sbo->metaObject(oid);
      res.serviceId = sid;
      res.objectId = oid;
      return res;
    }

    void onProxyLost(GenericObject* ptr)
    {
      qiLogDebug() << "Proxy on argument object lost, invoking terminate...";
      DynamicObject* dobj = reinterpret_cast<DynamicObject*>(ptr->value);
      // dobj is a RemoteObject
      //FIXME: use post()
      ptr->call<void>("terminate", static_cast<RemoteObject*>(dobj)->service()).async();
    }

    AnyObject deserializeObject(const ObjectSerializationInfo& osi,
      TransportSocketPtr context)
    {
      if (!context)
        throw std::runtime_error("Unable to deserialize object without a valid TransportSocket");
      qiLogDebug() << "Creating unregistered object " << osi.serviceId << '/' << osi.objectId << " on " << context.get();
      RemoteObject* ro = new RemoteObject(osi.serviceId, osi.objectId, osi.metaObject, context);
      AnyObject o = makeDynamicAnyObject(ro, true, &onProxyLost);
      qiLogDebug() << "New object is " << o.get() << "on ro " << ro;
      assert(o);
      return o;
    }
  }

  AnyReference Message::value(const qi::Signature &signature, const qi::TransportSocketPtr &socket) const {
    qi::TypeInterface* type = qi::TypeInterface::fromSignature(signature);
    if (!type) {
      qiLogError() <<"fromBuffer: unknown type " << signature.toString();
      throw std::runtime_error("Could not construct type for " + signature.toString());
    qiLogDebug() << "Serialized message body: " << _p->buffer.size();
    }
    qi::BufferReader br(_p->buffer);
    AnyReference res(type);
    decodeBinary(&br, res, boost::bind(deserializeObject, _1, socket));
    return res;
  }

  void Message::setValue(const AnyReference &value, ObjectHost* context) {
    cow();
    if (value.type->kind() != qi::TypeInterface::Void)
    {
      encodeBinary(&_p->buffer, value, boost::bind(serializeObject, _1, context));
    }
  }

  void Message::setValues(const std::vector<qi::AnyReference>& values, ObjectHost* context)
  {
    cow();
    SerializeObjectCallback scb = boost::bind(serializeObject, _1, context);
    for (unsigned i = 0; i < values.size(); ++i)
      encodeBinary(&_p->buffer, values[i], scb);
  }

  //convert args then call setValues
  void Message::setValues(const std::vector<qi::AnyReference>& in, const qi::Signature& expectedSignature, ObjectHost* context) {
    qi::Signature argsSig = qi::makeTupleSignature(in, false);
    if (expectedSignature == argsSig) {
      setValues(in, context);
      return;
    }
    if (expectedSignature == "m")
    {
      /* We need to send a dynamic containing the value tuple to push the
      * signature. This wraps correctly without copying the data.
      */
      std::vector<qi::TypeInterface*> types;
      std::vector<void*> values;
      types.resize(in.size());
      values.resize(in.size());
      for (unsigned i=0; i<in.size(); ++i)
      {
        types[i] = in[i].type;
        values[i] = in[i].value;
      }
      AnyReference tuple = makeGenericTuplePtr(types, values);
      GenericValue val(tuple, false, false);
      setValue(AnyReference(val), context);
      return;
    }
    std::vector<AnyReference> nargs(in);
    Signature src = argsSig.begin().children();
    Signature dst = expectedSignature.begin().children();
    Signature::iterator its = src.begin(), itd = dst.begin();
    boost::dynamic_bitset<> allocated(nargs.size());
    for (unsigned i = 0; i< nargs.size(); ++i, ++its, ++itd)
    {
      if (*its != *itd)
      {
        ::qi::TypeInterface* target = ::qi::TypeInterface::fromSignature(*itd);
        if (!target)
          throw std::runtime_error("remote call: Failed to obtain a type from signature " + (*itd).toString());
        std::pair<AnyReference, bool> c = nargs[i].convert(target);
        if (!c.first.type)
        {
          throw std::runtime_error(
                _QI_LOG_FORMAT("remote call: failed to convert argument %s from %s to %s", i, (*its).toString(), (*itd).toString()));
        }
        nargs[i] = c.first;
        allocated[i] = c.second;
      }
    }
    setValues(nargs, context);
    for (unsigned i = 0; i< nargs.size(); ++i)
      if (allocated[i])
        nargs[i].destroy();
  }

  const qi::Buffer &Message::buffer() const
  {
    return _p->buffer;
  }

  bool Message::isValid()
  {
    if (_p->header.magic != qi::MessagePrivate::magic)
    {
      qiLogError()  << "Message dropped (magic is incorrect)" << std::endl;
      return false;
    }

    if (type() == qi::Message::Type_None)
    {
      qiLogError()  << "Message dropped (type is None)" << std::endl;
      return false;
    }

    if (object() == qi::Message::GenericObject_None)
    {
      qiLogError()  << "Message dropped (object is 0)" << std::endl;
      assert(object() != qi::Message::GenericObject_None);
      return false;
    }

    return true;
  }

  void Message::setAddress(const MessageAddress &address) {
    cow();
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

