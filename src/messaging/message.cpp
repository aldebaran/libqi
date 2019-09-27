/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/
#include <cstring>

#include <boost/make_shared.hpp>
#include <boost/container/small_vector.hpp>

#include <qi/assert.hpp>
#include <qi/anyvalue.hpp>
#include "message.hpp"

#include <qi/atomic.hpp>
#include <qi/log.hpp>
#include <boost/cstdint.hpp>
#include <qi/types.hpp>
#include <qi/buffer.hpp>
#include <qi/binarycodec.hpp>

#include "boundobject.hpp"
#include "remoteobject_p.hpp"

qiLogCategory("qimessaging.message");

namespace qi
{
  const qi::uint32_t Message::Header::magicCookie = 0x42adde42;

  qi::uint32_t Message::Header::newMessageId()
  {
    static std::atomic<qi::uint32_t> id(0);
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
    case Type_Capability:
      return "Capability";
    case Type_Cancel:
      return "Cancel";
    case Type_Canceled:
      return "Canceled";
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
    case BoundObjectFunction_RegisterEventWithSignature:
      return "RegisterEventWithSignature";
    }

    if (service != qi::Message::Service_ServiceDirectory)
    {
      return  nullptr;
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
    case ServiceDirectoryAction_MachineId:
      return "MachineId";
    default:
      return  nullptr;
    }
  }

  std::ostream& operator<<(std::ostream& os, const qi::Message& msg)
  {
    os << "message {" << std::endl
       << "  size=" << msg.header().size << "," << std::endl
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
    qi::detail::printBuffer(os, msg.buffer());
    os << std::endl << "}";
    return os;
  }

  void Message::setFunction(qi::uint32_t function)
  {
    if (type() == Type_Event)
    {
      qiLogDebug() << "called setFunction() on Type_Event message";
    }
    _header.action = function;
  }

  unsigned int Message::function() const
  {
    if (type() == Type_Event)
    {
      qiLogDebug() << "called function() on Type_Event message";
    }
    return _header.action;
  }

  void Message::setEvent(qi::uint32_t event)
  {
    if (type() != Type_Event)
    {
      qiLogDebug() << "called setEvent() on non Type_Event message";
    }
    _header.action = event;
  }

  unsigned int Message::event() const
  {
    if (type() != Type_Event)
    {
      qiLogDebug() << "called event() on non Type_Event message";
    }
    return _header.action;
  }

  namespace
  {
    ObjectSerializationInfo serializeObject(
      AnyObject object,
      boost::weak_ptr<ObjectHost> context,
      MessageSocketPtr socket)
    {
      auto host = context.lock();
      if (!host || !socket)
        throw std::runtime_error("Unable to serialize object without a valid ObjectHost and MessageSocket");

      const unsigned int sid = host->service();
      if (!object)
      {
        ObjectSerializationInfo res;
        res.serviceId = sid;
        res.objectId = nullObjectId;
        res.objectUid = os::ptrUid(nullptr);
        return res;
      }

      const unsigned int oid = host->nextId();
      auto bo = BoundObject::makePtr(sid, oid, object, MetaCallType_Queued, true, context);
      host->addObject(bo, socket);
      qiLogDebug() << "Hooking " << oid <<" on " << host.get();
      qiLogDebug() << "BoundObject " << bo << " obj " << object.asGenericObject();
      // Transmit the metaObject augmented by BoundObject.
      ObjectSerializationInfo res;
      res.metaObject = bo->metaObject(oid);
      res.serviceId = sid;
      res.objectId = oid;
      res.objectUid = object.uid();
      return res;
    }

    void onProxyLost(GenericObject* ptr)
    {
      qiLogDebug() << "Proxy on argument object lost, invoking terminate...";
      DynamicObject* dobj = reinterpret_cast<DynamicObject*>(ptr->value);
      // dobj is a RemoteObject
      /* Warning, we are in a shared_ptr destruction callback.
      * So we cannot reacquire the shared_ptr, which call/post will try to
      * do using shared_from_this().
      * So go through backend
      */
      //FIXME: use post()
      int mid = dobj->metaObject().methodId("terminate::(I)");
      if (mid<0)
      {
        qiLogError() << "terminate() method not found, object will not be destroyed";
        return;
      }
      GenericFunctionParameters params;
      // Argument is unused by remote end, but better pass something valid just in case.
      int sid = static_cast<RemoteObject*>(dobj)->service();
      params.push_back(AnyReference::from(sid));
      dobj->metaPost(AnyObject(), mid, params);
    }

    AnyObject deserializeObject(const ObjectSerializationInfo& osi, MessageSocketPtr socket)
    {
      if (!socket)
        throw std::runtime_error("Unable to deserialize object without a valid TransportSocket");
      qiLogDebug() << "Creating unregistered object " << osi.serviceId << '/' << osi.objectId
                   << " uid = '" << (osi.objectUid ? *osi.objectUid : ObjectUid{}) << "' on "
                   << socket.get();
      RemoteObject* ro = new RemoteObject(osi.serviceId, osi.objectId, osi.metaObject, socket);
      AnyObject o = makeDynamicAnyObject(ro, true, osi.objectUid, &onProxyLost);
      qiLogDebug() << "New object is " << o.asGenericObject() << "on ro " << ro;
      QI_ASSERT(o);
      return o;
    }
  }

  AnyValue Message::value(const qi::Signature& signature,
                              const qi::MessageSocketPtr& socket) const
  {
    qi::TypeInterface* type = qi::TypeInterface::fromSignature(signature);
    if (!type) {
      qiLogError() <<"fromBuffer: unknown type " << signature.toString();
      throw std::runtime_error("Could not construct type for " + signature.toString());
    }
    qi::BufferReader br(_buffer);
    AnyReference res(type);
    return AnyValue(
      decodeBinary(&br, res, boost::bind(deserializeObject, _1, socket), socket),
      false, // i.e. don't copy
      true   // i.e. become resource owner
    );
  }

  void Message::setValue(const AutoAnyReference& value,
                         const Signature& sig,
                         boost::weak_ptr<ObjectHost> context,
                         MessageSocketPtr socket)
  {
    if (!value.isValid())
    {
      const auto msg = "Setvalue(): invalid value";
      qiLogWarning() << msg;
      setType(qi::Message::Type_Error);
      setError(msg);
      return;
    }

    Signature effective = value.type()->signature();
    if (effective != sig)
    {
      TypeInterface* ti = TypeInterface::fromSignature(sig);
      if (!ti)
        qiLogWarning() << "setValue(): cannot construct type for signature " << sig.toString();
      auto conv = value.convert(ti);
      if (!conv->type()) {
        std::stringstream ss;
        ss << "Setvalue(): failed to convert effective value "
           << value.type()->signature().toString()
           << " to expected type "
           << sig.toString() << '(' << ti->infoString() << ')';
        qiLogWarning() << ss.str();
        setType(qi::Message::Type_Error);
        setError(ss.str());
      }
      else
        encodeBinary(*conv, boost::bind(serializeObject, _1, context, socket),
                     socket);
    }
    else if (value.type()->kind() != qi::TypeKind_Void)
    {
      encodeBinary(value, boost::bind(serializeObject, _1, context, socket), socket);
    }
  }

  void Message::setValues(const std::vector<qi::AnyReference>& values,
                          boost::weak_ptr<ObjectHost> context, MessageSocketPtr socket)
  {
    SerializeObjectCallback scb = boost::bind(serializeObject, _1, context, socket);
    for (unsigned i = 0; i < values.size(); ++i)
      encodeBinary(values[i], scb, socket);
  }

  //convert args then call setValues
  void Message::setValues(const std::vector<qi::AnyReference>& in, const qi::Signature& expectedSignature,
                          boost::weak_ptr<ObjectHost> context, MessageSocketPtr socket) {
    qi::Signature argsSig = qi::makeTupleSignature(in, false);
    if (expectedSignature == argsSig) {
      setValues(in, context, socket);
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
        types[i] = in[i].type();
        values[i] = in[i].rawValue();
      }
      AnyReference tuple = makeGenericTuplePtr(types, values);
      AnyValue val(tuple, false, false);
      encodeBinary(AnyReference::from(val),
                   boost::bind(serializeObject, _1, context, socket), socket);
      return;
    }
    /* This check does not makes sense for this transport layer who does not care,
     * But it checks a general rule that is true for all the messages we use and
     * it can help catch many mistakes.
     */
    if (expectedSignature.type() != Signature::Type_Tuple)
      throw std::runtime_error("Expected a tuple, got " + expectedSignature.toString());
    AnyReferenceVector nargs(in);
    SignatureVector src = argsSig.children();
    SignatureVector dst = expectedSignature.children();
    if (src.size() != dst.size())
      throw std::runtime_error("remote call: signature size mismatch");
    SignatureVector::iterator its = src.begin(), itd = dst.begin();

    boost::container::small_vector<detail::UniqueAnyReference, detail::maxAnyFunctionArgsCountHint>
      uniqueArgs;
    uniqueArgs.reserve(nargs.size());
    for (unsigned i = 0; i< nargs.size(); ++i, ++its, ++itd)
    {
      if (*its != *itd)
      {
        ::qi::TypeInterface* target = ::qi::TypeInterface::fromSignature(*itd);
        if (!target)
          throw std::runtime_error("remote call: Failed to obtain a type from signature " +
                                   (*itd).toString());
        auto c = nargs[i].convert(target);
        if (!c->type())
        {
          throw std::runtime_error(
              _QI_LOG_FORMAT("remote call: failed to convert argument %s from %s to %s", i,
                             (*its).toString(), (*itd).toString()));
        }
        nargs[i] = *c;
        uniqueArgs.emplace_back(std::move(c));
      }
    }

    setValues(nargs, context, socket);
  }
}
