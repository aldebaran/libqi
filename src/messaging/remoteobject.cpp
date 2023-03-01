/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

// Disable "'this': used in base member initializer list"
#include <ka/macro.hpp>
KA_WARNING_PUSH()
KA_WARNING_DISABLE(4355, )

#include "remoteobject_p.hpp"
#include "message.hpp"
#include "messagesocket.hpp"
#include <src/type/signal_p.hpp>
#include <qi/log.hpp>
#include <boost/thread/mutex.hpp>
#include <qi/eventloop.hpp>

qiLogCategory("qimessaging.remoteobject");

// Helper for debug logs of the class. Can only be called from non static member functions of
// `qi::RemoteObject`.
#define QI_LOG_DEBUG_REMOTEOBJECT() \
  qiLogDebug() << this << " (service=" << _service << ", object=" << _object << ") - "

namespace qi {


  static qi::MetaObject createRemoteObjectSpecialMetaObject() {
    qi::MetaObjectBuilder mob;
    mob.addMethod("L", "registerEvent", "(IIL)", qi::Message::BoundObjectFunction_RegisterEvent);
    mob.addMethod("v", "unregisterEvent", "(IIL)", qi::Message::BoundObjectFunction_UnregisterEvent);
    mob.addMethod(typeOf<MetaObject>()->signature(), "metaObject", "(I)", qi::Message::BoundObjectFunction_MetaObject);
    mob.addMethod("L", "registerEventWithSignature", "(IILs)", qi::Message::BoundObjectFunction_RegisterEventWithSignature);
    const auto mo = mob.metaObject();
    QI_ASSERT(mo.methodId("registerEvent::(IIL)") == qi::Message::BoundObjectFunction_RegisterEvent);
    QI_ASSERT(mo.methodId("unregisterEvent::(IIL)") == qi::Message::BoundObjectFunction_UnregisterEvent);
    QI_ASSERT(mo.methodId("metaObject::(I)") == qi::Message::BoundObjectFunction_MetaObject);
    QI_ASSERT(mo.methodId("registerEventWithSignature::(IILs)") == qi::Message::BoundObjectFunction_RegisterEventWithSignature);
    return mo;
  }

  namespace
  {
    [[noreturn]]
    void throwRemoteObjectDestroyedException()
    {
      throw std::runtime_error("the remote object instance has been destroyed.");
    }
  }

  RemoteObject::RemoteObject(unsigned int service, unsigned int object, boost::optional<ObjectUid> uid)
    : ObjectHost(service)
    , _socket()
    , _service(service)
    , _object(object)
    , _self(makeDynamicAnyObject(this, false, uid))
  {
    setUid(_self.uid()); // Make sure this object's uid and _self's uid are the same.
    // Simple metaObject with only special methods. (<100)
    // Will be *replaced* by metaObject received from remote end, when fetchMetaObject is invoked
    // and returns.
    static const qi::MetaObject mo = createRemoteObjectSpecialMetaObject();
    setMetaObject(mo);
  }

  RemoteObject::~RemoteObject()
  {
    QI_LOG_DEBUG_REMOTEOBJECT() << "~RemoteObject " << this;
    //close may already have been called. (by Session_Service.close)
    close("RemoteObject destroyed");
  }

  //### RemoteObject

  void RemoteObject::setTransportSocket(qi::MessageSocketPtr socket) {
    MessageSocketPtr sock = *_socket;
    if (socket == sock)
      return;
    if (sock) {
      close("Socket invalidated");
    }
    auto syncSocket = _socket.synchronize();
    *syncSocket = socket;
    //do not set the socket on the remote object
    if (socket) {
      QI_LOG_DEBUG_REMOTEOBJECT() << "Adding connection to socket " << socket;
      const auto weakPtr = weak_from_this();
      _linkMessageDispatcher = socket->messagePendingConnect(_service,
        _object,
        [=](const Message& msg) {
          if (auto self = weakPtr.lock())
            return self->onMessagePending(msg);
          qiLogVerbose() << "Could not handle message " << msg.address()
                         << " because the remote object has already been destroyed.";
          return DispatchStatus::MessageHandlingFailure;
        });
      _linkDisconnected = socket->disconnected.connect(
        track([=](const std::string& reason) { onSocketDisconnected(reason); },
              weakPtr));
    }
  }

  //should be done in the object thread
  void RemoteObject::onSocketDisconnected(std::string /*error*/)
  {
    close("Socket Disconnected", true);
    throw PointerLockException();
  }

  void RemoteObject::onMetaObject(qi::Future<qi::MetaObject> fut, qi::Promise<void> prom) {
    if (fut.hasError()) {
      qiLogVerbose() << "MetaObject error: " << fut.error();
      prom.setError(fut.error());
      return;
    }
    qiLogVerbose() << "Fetched metaobject";
    setMetaObject(fut.value());
    prom.setValue(0);
  }



  //retrieve the metaObject from the network
  qi::Future<void> RemoteObject::fetchMetaObject() {
    qiLogVerbose() << "Requesting metaobject";
    qi::Promise<void> prom(qi::FutureCallbackType_Sync);
    qi::Future<qi::MetaObject> fut =
      _self.async<qi::MetaObject>("metaObject", 0U);
    fut.connect(trackWithFallback(&throwRemoteObjectDestroyedException,
                                  boost::bind<void>(&RemoteObject::onMetaObject, this, _1, prom),
                                  weak_from_this()));
    return prom.future();
  }

  //should be done in the object thread
  DispatchStatus RemoteObject::onMessagePending(const qi::Message &msg)
  {
    MessageSocketPtr sock = *_socket;
    QI_LOG_DEBUG_REMOTEOBJECT() << "msg " << msg.address()
                                << " type=" << msg.type()
                                << " size=" << msg.buffer().size();

    QI_ASSERT_TRUE(msg.object() == _object);

    if (msg.type() == qi::Message::Type_Event) {
      SignalBase* sb = signal(msg.event());
      if (sb)
      {
        try {
          // Signal associated with properties have incorrect signature,
          // Trust MetaObject.
          //std::string sig = sb->signature();
          const MetaSignal* ms  = _self.metaObject().signal(msg.event());
          qi::Signature sig = ms->parametersSignature();

          // Remove top-level tuple
          //sig = sig.substr(1, sig.length()-2);
          //TODO: Optimise
          AnyValue value(msg.value((msg.flags() & Message::TypeFlag_DynamicPayload) ? "m" : sig,
                                   sock));

          {
            GenericFunctionParameters args;
            if (sig == "m")
              args = value.content().asTupleValuePtr();
            else
              args = value.asTupleValuePtr();
            QI_LOG_DEBUG_REMOTEOBJECT()
              << "Triggering local event listeners with args : " << args.size();
            sb->trigger(args);
          }
        }
        catch (const std::exception& e)
        {
          qiLogWarning() << "Deserialize error on event: " << e.what();
        }
      }
      else
      {
        qiLogWarning() << "Event message on unknown signal " << msg.event();
      }
      return DispatchStatus::MessageHandled_WithError;
    }


    if (msg.type() != qi::Message::Type_Reply
      && msg.type() != qi::Message::Type_Error
      && msg.type() != qi::Message::Type_Canceled) {
      qiLogError() << "Message " << msg.address()
                   << " type not handled: " << msg.type() << " ("
                   << msg.type() << ")";

      return DispatchStatus::MessageNotHandled;
    }

    qi::Promise<AnyReference> promise;
    {
      auto syncPromises = _promises.synchronize();
      auto it = syncPromises->find(msg.id());
      if (it != syncPromises->end()) {
        promise = (*syncPromises)[msg.id()];
        syncPromises->erase(it);
        QI_LOG_DEBUG_REMOTEOBJECT() << "Handling promise id:" << msg.id();
      }
      else
      {
        qiLogError() << "no promise found for req id:" << msg.id() << "  obj: " << msg.service()
                     << "  func: " << msg.function()
                     << "  type: " << msg.type();
        return DispatchStatus::MessageHandled_WithError;
      }
    }

    switch (msg.type()) {
      case qi::Message::Type_Canceled: {
        QI_LOG_DEBUG_REMOTEOBJECT() << "Message " << msg.address() << " has been cancelled.";
        promise.setCanceled();
        return DispatchStatus::MessageHandled;
      }
      case qi::Message::Type_Reply: {
        // Get call signature
        MetaMethod* mm =  metaObject().method(msg.function());
        if (!mm)
        {
          qiLogError() << "Result for unknown function "
           << msg.function();
           promise.setError("Result for unknown function");
           return DispatchStatus::MessageHandled_WithError;
        }
        try {
          AnyValue value(msg.value((msg.flags() & Message::TypeFlag_DynamicPayload) ?
                                     "m" :
                                     mm->returnSignature(),
                                   sock));
          promise.setValue(value.release());
        } catch (std::runtime_error &err) {
          promise.setError(err.what());
          return DispatchStatus::MessageHandled_WithError;
        }

        QI_LOG_DEBUG_REMOTEOBJECT() << "Message " << msg.address() << " passed to promise";
        return DispatchStatus::MessageHandled;
      }

      case qi::Message::Type_Error: {
        try {
          static std::string sigerr("m");
          const std::string err = msg.value(sigerr, sock).content().asString();
          qiLogVerbose() << "Received error message"  << msg.address() << ":" << err;
          promise.setError(err);
        } catch (std::runtime_error &e) {
          //houston we have an error about the error..
          promise.setError(e.what());
          return DispatchStatus::MessageHandled_WithError;
        }
        return DispatchStatus::MessageHandled;
      }
      default:
        //not possible
        return DispatchStatus::MessageNotHandled;
    }
  }


  qi::Future<AnyReference> RemoteObject::metaCall(AnyObject, unsigned int method, const qi::GenericFunctionParameters &in, MetaCallType /*callType*/, Signature returnSignature)
  {
    MetaMethod *mm = metaObject().method(method);
    if (!mm) {
      std::stringstream ss;
      ss << "Method " << method << " not found on service " << _service;
      return makeFutureError<AnyReference>(ss.str());
    }
    QI_LOG_DEBUG_REMOTEOBJECT() << "Metacall method id=" << method << " (" << mm->name() << ")";

    float canConvert = 1;
    if (returnSignature.isValid())
    {
      canConvert = mm->returnSignature().isConvertibleTo(returnSignature);
      QI_LOG_DEBUG_REMOTEOBJECT() << "Return type conversion score: " << canConvert;
      if (canConvert == 0)
      {
        // last chance for dynamics and adventurous users
        canConvert = returnSignature.isConvertibleTo(mm->returnSignature());
        if (canConvert == 0)
          return makeFutureError<AnyReference>(
            "Call error: will not be able to convert return type from "
              + mm->returnSignature().toString()
              + " to " + returnSignature.toString());
        else
          qiLogVerbose() << "Return signature might be incorrect depending on the value, from "
            + mm->returnSignature().toString()
            + " to " + returnSignature.toString();
      }
    }

    qi::Promise<AnyReference> out;
    qi::Message msg;
    MessageSocketPtr sock;
    {
      auto syncSock = _socket.synchronize();
      sock = *syncSock;
      // Check socket while holding the lock to avoid a race with close()
      // where we would add a promise to the map after said map got cleared
      if (!sock || !sock->isConnected())
      {
        return makeFutureError<AnyReference>("Socket is not connected");
      }
      auto syncPromises = _promises.synchronize();
      if (syncPromises->find(msg.id()) != syncPromises->end())
      {
        qiLogError() << "There is already a pending promise with id "
                                   << msg.id();
      }
      QI_LOG_DEBUG_REMOTEOBJECT() << "Adding promise of message response id:" << msg.id();
      (*syncPromises)[msg.id()] = out;
    }
    qi::Signature funcSig = mm->parametersSignature();
    try {
      msg.setValues(in, funcSig, weak_from_this(), sock);
    }
    catch(const std::exception& e)
    {
      qiLogVerbose() << "setValues exception: " << e.what();
      if (!sock->remoteCapability("MessageFlags", false))
        throw e;
      // Delegate conversion to the remote end.
      msg.addFlags(Message::TypeFlag_DynamicPayload);
      msg.setValues(in, "m", weak_from_this(), sock);
    }
    if (canConvert < 0.2)
    {
      msg.addFlags(Message::TypeFlag_ReturnType);
      msg.setValue(returnSignature.toString(), Signature("s"));
    }
    msg.setType(qi::Message::Type_Call);
    msg.setService(_service);
    msg.setObject(_object);
    msg.setFunction(method);

    //error will come back as a error message
    const auto msgId = msg.id();
    if (!sock->isConnected() || !sock->send(std::move(msg))) {
      qi::MetaMethod*   meth = metaObject().method(method);
      std::stringstream ss;
      if (meth) {
        ss << "Network error while sending data to method: '";
        ss << meth->toString();
        ss << "'.";
      } else {
        ss << "Network error while sending data an unknown method (id=" << method << ").";
      }
      if (!sock->isConnected()) {
        ss << " Socket is not connected.";
        qiLogVerbose() << ss.str();
      } else {
        qiLogError() << ss.str();
      }
      out.setError(ss.str());
      QI_LOG_DEBUG_REMOTEOBJECT() << "Removing promise id:" << msgId;
      _promises->erase(msgId);
    }
    else
      out.setOnCancel(qi::bind(&RemoteObject::onFutureCancelled, this, msgId));
    return out.future();
  }

  void RemoteObject::onFutureCancelled(unsigned int originalMessageId)
  {
    QI_LOG_DEBUG_REMOTEOBJECT() << "Cancel request for message " << originalMessageId;
    MessageSocketPtr sock = *_socket;
    Message cancelMessage;

    if (!sock)
    {
      qiLogWarning() << "Tried to cancel a call, but the socket to service "
                   << _service << " is disconnected.";
      return;
    }
    if (!sock->sharedCapability<bool>(capabilityname::remoteCancelableCalls, false))
    {
      qiLogWarning() << "Remote end does not support cancelable calls.";
      return;
    }
    cancelMessage.setService(_service);
    cancelMessage.setType(Message::Type_Cancel);
    cancelMessage.setValue(AnyReference::from(originalMessageId), "I");
    cancelMessage.setObject(_object);
    sock->send(std::move(cancelMessage));
  }

  void RemoteObject::metaPost(AnyObject, unsigned int event, const qi::GenericFunctionParameters &in)
  {
    // Bounce the emit request to server
    // TODO: one optimisation that could be done is to trigger the local
    // subscribers immediately.
    // But it is a bit complex, because the server will bounce the
    // event back to us.
    qi::Message msg;
    // apparent signature must match for correct serialization
    qi::Signature argsSig = qi::makeTupleSignature(in, false);
    qi::Signature funcSig;
    const MetaMethod* mm = metaObject().method(event);
    if (mm)
      funcSig = mm->parametersSignature();
    else
    {
      const MetaSignal* ms = metaObject().signal(event);
      if (!ms)
        throw std::runtime_error("Post target id does not exist");
      funcSig = ms->parametersSignature();
    }
    MessageSocketPtr sock = *_socket;
    try {
      msg.setValues(in, funcSig, weak_from_this(), sock);
    }
    catch(const std::exception& e)
    {
      qiLogVerbose() << "setValues exception: " << e.what();
      if (!sock->remoteCapability("MessageFlags", false))
        throw e;
      // Delegate conversion to the remote end.
      msg.addFlags(Message::TypeFlag_DynamicPayload);
      msg.setValues(in, "m", weak_from_this(), sock);
    }
    msg.setType(Message::Type_Post);
    msg.setService(_service);
    msg.setObject(_object);
    msg.setFunction(event);
    if (!sock || !sock->send(std::move(msg))) {
      qiLogVerbose() << "error while emitting event";
      return;
    }
  }

  static void onEventConnected(RemoteObject* /*ro*/, qi::Future<SignalLink> fut, qi::Promise<SignalLink> prom, SignalLink id) {
    if (fut.hasError()) {
      prom.setError(fut.error());
      return;
    }
    prom.setValue(id);
  }

  qi::Future<SignalLink> RemoteObject::metaConnect(unsigned int event, const SignalSubscriber& sub)
  {
    qi::Promise<SignalLink> prom(qi::FutureCallbackType_Sync);

    // Bind the subscriber locally.
    SignalLink uid = DynamicObject::metaConnect(event, sub).value();

    boost::recursive_mutex::scoped_lock _lock(_localToRemoteSignalLinkMutex);
    // maintain a map of localsignal -> remotesignal
    //(we only use one remotesignal, for many locals)
    LocalToRemoteSignalLinkMap::iterator it;
    RemoteSignalLinks& rsl = _localToRemoteSignalLink[event];
    rsl.localSignalLink.push_back(uid);

    if (!isValidSignalLink(rsl.remoteSignalLink))
    {
      /* Try to handle struct versionning.
      * Hypothesis: Everything in this address space uses the same struct
      * version. It makes sense since typesystem does not handle conflicting
      * definitions for the same type name (due to global typeid->typeinterface factory).
      *
      * So we use the very first subscriber to try to detect a version mismatch
      * between what the callback expects, and what the signal advertises.
      * If so we inform the remote end to try to convert for us.
      */
      Signature subSignature = sub.signature();
      float score = 1;
      if (subSignature.isValid())
      {
        const MetaSignal* ms = metaObject().signal(event);
        if (!ms)
          return makeFutureError<SignalLink>("Signal not found");
        score = ms->parametersSignature().isConvertibleTo(subSignature);
        QI_LOG_DEBUG_REMOTEOBJECT()
          << "Conversion score " << score << " " << ms->parametersSignature().toString() << " -> "
          << subSignature.toString();
        if (!score)
        {
          std::ostringstream ss;
          ss << "Subscriber not compatible to signal signature: cannot convert " << ms->parametersSignature().toString()
             << " to " << subSignature.toString();
          return makeFutureError<SignalLink>(ss.str());
        }
      }
      rsl.remoteSignalLink = uid;
      QI_LOG_DEBUG_REMOTEOBJECT() << "connect() to " << event << " gave " << uid
                                  << " (new remote connection)";
      if (score >= 0.2)
        rsl.future = _self.async<SignalLink>("registerEvent", _service, event, uid);
      else // we might or might not be capable to convert, ask the remote end to try also
        rsl.future =
            _self.async<SignalLink>("registerEventWithSignature", _service, event, uid, subSignature.toString());
    }
    else
    {
      QI_LOG_DEBUG_REMOTEOBJECT() << "connect() to " << event << " gave " << uid << " (reusing remote connection)";
    }

    rsl.future.connect(trackWithFallback(&throwRemoteObjectDestroyedException,
                                         boost::bind<void>(&onEventConnected, this, _1, prom, uid),
                                         weak_from_this()));
    return prom.future();
  }

  qi::Future<void> RemoteObject::metaDisconnect(SignalLink linkId)
  {
    // The invalid signal link is never connected, therefore the disconnection
    // is always successful.
    if (!isValidSignalLink(linkId))
      return futurize();

    unsigned int event = linkId >> 32;
    //disconnect locally
    Future<void> fut = DynamicObject::metaDisconnect(linkId);
    return fut.then(trackWithFallback(&throwRemoteObjectDestroyedException,
      [=](Future<void> f) -> Future<void> {
        if (f.hasError())
        {
          std::stringstream ss;
          ss << "Disconnection failure for " << linkId << ", error:" << fut.error();
          qiLogWarning() << ss.str();
          throw std::runtime_error(ss.str());
        }

        boost::recursive_mutex::scoped_lock _lock(_localToRemoteSignalLinkMutex);
        LocalToRemoteSignalLinkMap::iterator it;
        it = _localToRemoteSignalLink.find(event);
        if (it == _localToRemoteSignalLink.end()) {
          qiLogWarning() << "Cannot find " << event << " in the local-to-remote signal map";
          return f;
        }

        auto toDisco = SignalBase::invalidSignalLink;
        {
          RemoteSignalLinks& rsl = it->second;
          std::vector<SignalLink>::iterator vslit;
          vslit = std::find(rsl.localSignalLink.begin(), rsl.localSignalLink.end(), linkId);

          if (vslit != rsl.localSignalLink.end()) {
            rsl.localSignalLink.erase(vslit);
          } else {
            qiLogWarning() << "Cannot find " << linkId << " in the remote signal vector (event:" << event << ")";
          }

          // Only drop the remote connection when no more local connection is registered.
          if (rsl.localSignalLink.size() == 0) {
            toDisco = exchangeInvalidSignalLink(rsl.remoteSignalLink);
            _localToRemoteSignalLink.erase(it);
          }
        }

        if (isValidSignalLink(toDisco))
        {
          MessageSocketPtr sock = *_socket;
          if (sock && sock->isConnected())
            return _self.async<void>("unregisterEvent", _service, event, toDisco);
        }
        return f;
      }, weak_from_this())).unwrap();
  }

  void RemoteObject::close(const std::string& reason, bool fromSignal)
  {
    QI_LOG_DEBUG_REMOTEOBJECT() << "Closing remote object";
    MessageSocketPtr socket;
    {
       auto syncSock = _socket.synchronize();
       socket = *syncSock;
       syncSock->reset();
    }
    if (socket)
    { // Do not hold any lock when invoking signals.
      QI_LOG_DEBUG_REMOTEOBJECT() << "Removing connection from socket " << socket;
      socket->messagePendingDisconnect(_service, _object,
        exchangeInvalidSignalLink(_linkMessageDispatcher));
      if (!fromSignal)
        socket->disconnected.disconnectAsync(
          exchangeInvalidSignalLink(_linkDisconnected));
    }
    std::map<int, qi::Promise<AnyReference> > promises;
    {
      auto syncPromises = _promises.synchronize();
      promises = *syncPromises;
      syncPromises->clear();
    }
    // Nobody should be able to add anything to promises at this point.
    for (auto& pair: promises)
    {
      qiLogVerbose() << "Reporting error for request " << pair.first << "(" << reason << ")";
      pair.second.setError(reason);
    }

    //@warning: remove connection are not removed
    //          not very important ATM, because RemoteObject
    //          cant be reconnected
  }

 qi::Future<AnyValue> RemoteObject::metaProperty(qi::AnyObject /*context*/, unsigned int id)
 {
   QI_LOG_DEBUG_REMOTEOBJECT() << "bouncing property";
   // FIXME: perform some validations on this end?
   return _self.async<AnyValue>("property", id);
 }

 qi::Future<void> RemoteObject::metaSetProperty(qi::AnyObject /*context*/, unsigned int id, AnyValue val)
 {
   QI_LOG_DEBUG_REMOTEOBJECT() << "bouncing setProperty";
   return _self.async<void>("setProperty", id, val);
 }

// we use different ranges for ids from RemoteObject and BoundObject to avoid collisions
// RemoteObject takes the upper part of the unsigned int
Atomic<unsigned int> RemoteObject::_nextId(std::numeric_limits<unsigned int>::max() / 2);

}

KA_WARNING_POP()
