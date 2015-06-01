/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

// Disable "'this': used in base member initializer list"
#ifdef _MSC_VER
# pragma warning( push )
# pragma warning(disable: 4355)
#endif

#include "remoteobject_p.hpp"
#include "message.hpp"
#include "transportsocket.hpp"
#include <qi/log.hpp>
#include <boost/thread/mutex.hpp>
#include <qi/eventloop.hpp>

qiLogCategory("qimessaging.remoteobject");

namespace qi {


  static qi::MetaObject* createRemoteObjectSpecialMetaObject() {
    qi::MetaObject *mo = new qi::MetaObject;
    qi::MetaObjectBuilder mob;
    mob.addMethod("L", "registerEvent", "(IIL)", qi::Message::BoundObjectFunction_RegisterEvent);
    mob.addMethod("v", "unregisterEvent", "(IIL)", qi::Message::BoundObjectFunction_UnregisterEvent);
    mob.addMethod(typeOf<MetaObject>()->signature(), "metaObject", "(I)", qi::Message::BoundObjectFunction_MetaObject);
    mob.addMethod("L", "registerEventWithSignature", "(IILs)", qi::Message::BoundObjectFunction_RegisterEventWithSignature);
    *mo = mob.metaObject();

    assert(mo->methodId("registerEvent::(IIL)") == qi::Message::BoundObjectFunction_RegisterEvent);
    assert(mo->methodId("unregisterEvent::(IIL)") == qi::Message::BoundObjectFunction_UnregisterEvent);
    assert(mo->methodId("metaObject::(I)") == qi::Message::BoundObjectFunction_MetaObject);
    assert(mo->methodId("registerEventWithSignature::(IILs)") == qi::Message::BoundObjectFunction_RegisterEventWithSignature);

    return mo;
  }

  RemoteObject::RemoteObject(unsigned int service, qi::TransportSocketPtr socket)
    : ObjectHost(service)
    , Trackable<RemoteObject>(this)
    , _socket()
    , _service(service)
    , _object(1)
    , _linkMessageDispatcher(0)
    , _self(makeDynamicAnyObject(this, false))
  {
    /* simple metaObject with only special methods. (<100)
     * Will be *replaced* by metaObject received from remote end, when
     * fetchMetaObject is invoked and retuns.
    */
    static qi::MetaObject* mo = 0;
    QI_ONCE(mo = createRemoteObjectSpecialMetaObject());
    setMetaObject(*mo);
    setTransportSocket(socket);
    //fetchMetaObject should be called to make sure the metaObject is valid.
  }

  RemoteObject::RemoteObject(unsigned int service, unsigned int object, qi::MetaObject metaObject, TransportSocketPtr socket)
    : ObjectHost(service)
    , Trackable<RemoteObject>(this)
    , _socket()
    , _service(service)
    , _object(object)
    , _linkMessageDispatcher(0)
    , _self(makeDynamicAnyObject(this, false))
  {
    setMetaObject(metaObject);
    setTransportSocket(socket);
  }

  RemoteObject::~RemoteObject()
  {
    qiLogDebug() << "~RemoteObject " << this;
    //close may already have been called. (by Session_Service.close)
    close("RemoteObject destroyed");
    destroy();
  }

  //### RemoteObject

  void RemoteObject::setTransportSocket(qi::TransportSocketPtr socket) {
    if (socket == _socket)
      return;
    if (_socket) {
      close("Socket invalidated");
    }

    boost::mutex::scoped_lock lock(_socketMutex);
    _socket = socket;
    //do not set the socket on the remote object
    if (socket) {
      qiLogDebug() << "Adding connection to socket" << (void*)_socket.get();
      // We must hook on ALL_OBJECTS in case our objectHost gets filled, even
      // if we are a sub-object.
      // We have no mechanism to bounce objectHost registration
      // to a 'parent' object.
      _linkMessageDispatcher = _socket->messagePendingConnect(_service,
        TransportSocket::ALL_OBJECTS,
        boost::bind<void>(&RemoteObject::onMessagePending, this, _1));
      _linkDisconnected      = _socket->disconnected.connect (
         &RemoteObject::onSocketDisconnected, weakPtr(), _1);
    }
  }

  //should be done in the object thread
  void RemoteObject::onSocketDisconnected(std::string error)
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
    fut.connect(boost::bind<void>(&RemoteObject::onMetaObject, this, _1, prom));
    return prom.future();
  }

  //should be done in the object thread
  void RemoteObject::onMessagePending(const qi::Message &msg)
  {
    qiLogDebug() << this << "(" << _service << '/' << _object << ") msg " << msg.address() << " " << msg.buffer().size();
    if (msg.object() != _object)
    {
      qiLogDebug() << "Passing message " << msg.address() << " to host " ;
      ObjectHost::onMessage(msg, _socket);
      return;
    }


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
          AnyReference value = msg.value((msg.flags()&Message::TypeFlag_DynamicPayload)? "m":sig, _socket);

          {
            GenericFunctionParameters args;
            if (sig == "m")
              args = value.content().asTupleValuePtr();
            else
              args = value.asTupleValuePtr();
            qiLogDebug() << "Triggering local event listeners with args : " << args.size();
            sb->trigger(args);
          }
          value.destroy();
        }
        catch (const std::exception& e)
        {
          qiLogWarning() << "Deserialize error on event: " << e.what();
        }
      }
      else
      {
        qiLogWarning() << "Event message on unknown signal " << msg.event();
        qiLogDebug() << metaObject().signalMap().size();
      }
      return;
    }


    if (msg.type() != qi::Message::Type_Reply
      && msg.type() != qi::Message::Type_Error
      && msg.type() != qi::Message::Type_Canceled) {
      qiLogError() << "Message " << msg.address() << " type not handled: " << msg.type();
      return;
    }

    qi::Promise<AnyReference> promise;
    {
      boost::mutex::scoped_lock lock(_promisesMutex);
      std::map<int, qi::Promise<AnyReference> >::iterator it;
      it = _promises.find(msg.id());
      if (it != _promises.end()) {
        promise = _promises[msg.id()];
        _promises.erase(it);
        qiLogDebug() << "Handling promise id:" << msg.id();
      } else  {
        qiLogError() << "no promise found for req id:" << msg.id()
                     << "  obj: " << msg.service() << "  func: " << msg.function() << " type: " << msg.type();
        return;
      }
    }

    switch (msg.type()) {
      case qi::Message::Type_Canceled: {
        qiLogDebug() << "Message " << msg.address() << " has been cancelled.";
        promise.setCanceled();
        return;
      }
      case qi::Message::Type_Reply: {
        // Get call signature
        MetaMethod* mm =  metaObject().method(msg.function());
        if (!mm)
        {
          qiLogError() << "Result for unknown function "
           << msg.function();
           promise.setError("Result for unknown function");
           return;
        }
        try {
          qi::AnyReference val = msg.value(
            (msg.flags() & Message::TypeFlag_DynamicPayload) ?
              "m" : mm->returnSignature(),
            _socket);
          promise.setValue(val);
        } catch (std::runtime_error &err) {
          promise.setError(err.what());
        }

        qiLogDebug() << "Message " << msg.address() << " passed to promise";
        return;
      }

      case qi::Message::Type_Error: {
        try {
          static std::string sigerr("m");
          qi::AnyReference gvp = msg.value(sigerr, _socket).content();
          std::string err = gvp.asString();
          qiLogVerbose() << "Received error message"  << msg.address() << ":" << err;
          promise.setError(err);
        } catch (std::runtime_error &e) {
          //houston we have an error about the error..
          promise.setError(e.what());
        }
        return;
      }
      default:
        //not possible
        return;
    }
  }


  qi::Future<AnyReference> RemoteObject::metaCall(AnyObject, unsigned int method, const qi::GenericFunctionParameters &in, MetaCallType callType, Signature returnSignature)
  {
    MetaMethod *mm = metaObject().method(method);
    if (!mm) {
      std::stringstream ss;
      ss << "Method " << method << " not found on service " << _service;
      return makeFutureError<AnyReference>(ss.str());
    }
    float canConvert = 1;
    if (returnSignature.isValid())
    {
      canConvert = mm->returnSignature().isConvertibleTo(returnSignature);
      qiLogDebug() << "return type conversion score: " << canConvert;
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
          qiLogWarning() << "Return signature might be incorrect depending on the value, from "
            + mm->returnSignature().toString()
            + " to " + returnSignature.toString();
      }
    }
    /* The promise will be set:
     - From here in case of error
     - From a network callback, called asynchronously in thread pool
     So it is safe to use a sync promise.
     */
    qi::Promise<AnyReference> out(&PromiseNoop<AnyReference>, FutureCallbackType_Sync);
    qi::Message msg;
    TransportSocketPtr sock;
    // qiLogDebug() << this << " metacall " << msg.service() << " " << msg.function() <<" " << msg.id();
    {
      boost::mutex::scoped_lock lock(_socketMutex);
      boost::mutex::scoped_lock lock2(_promisesMutex);
      // Check socket while holding the lock to avoid a race with close()
      // where we would add a promise to the map after said map got cleared
      if (!_socket || !_socket->isConnected())
      {
        return makeFutureError<AnyReference>("Socket is not connected");
      }
      // The remote object can be concurrently closed / other operation that modifies _socket
      // (even set it to null). We store the current socket locally so that the behavior
      // of metacall stays consistent throughout the function's execution.
      sock = _socket;
      if (_promises.find(msg.id()) != _promises.end())
      {
        qiLogError() << "There is already a pending promise with id "
                                   << msg.id();
      }
      qiLogDebug() << "Adding promise id:" << msg.id();
      _promises[msg.id()] = out;
    }
    qi::Signature funcSig = mm->parametersSignature();
    try {
      msg.setValues(in, funcSig, this, sock.get());
    }
    catch(const std::exception& e)
    {
      qiLogVerbose() << "setValues exception: " << e.what();
      if (!sock->remoteCapability("MessageFlags", false))
        throw e;
      // Delegate conversion to the remote end.
      msg.addFlags(Message::TypeFlag_DynamicPayload);
      msg.setValues(in, "m", this, sock.get());
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
    if (!sock->isConnected() || !sock->send(msg)) {
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

      {
        boost::mutex::scoped_lock lock(_promisesMutex);
        qiLogDebug() << "Removing promise id:" << msg.id();
        _promises.erase(msg.id());
      }
    }
    else
      out.setOnCancel(qi::bind<void(Promise<AnyReference>)>(&RemoteObject::onFutureCancelled, this, msg.id()));
    return out.future();
  }

  void RemoteObject::onFutureCancelled(unsigned int originalMessageId)
  {
    qiLogDebug() << "Cancel request for message " << originalMessageId;
    TransportSocketPtr sock;
    {
      boost::mutex::scoped_lock lock(_socketMutex);
      sock = _socket;
    }
    Message cancelMessage;

    if (!sock)
    {
      qiLogWarning() << "Tried to cancel a call, but the socket to service "
                   << _service << " is disconnected.";
      return;
    }
    if (!sock->sharedCapability<bool>("RemoteCancelableCalls", false))
    {
      qiLogWarning() << "Remote end does not support cancelable calls.";
      return;
    }
    cancelMessage.setService(_service);
    cancelMessage.setType(Message::Type_Cancel);
    cancelMessage.setValue(AnyReference::from(originalMessageId), "I");
    cancelMessage.setObject(_object);
    sock->send(cancelMessage);
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
     try {
      msg.setValues(in, funcSig, this, _socket.get());
    }
    catch(const std::exception& e)
    {
      qiLogVerbose() << "setValues exception: " << e.what();
      if (!_socket->remoteCapability("MessageFlags", false))
        throw e;
      // Delegate conversion to the remote end.
      msg.addFlags(Message::TypeFlag_DynamicPayload);
      msg.setValues(in, "m", this, _socket.get());
    }
    msg.setType(Message::Type_Post);
    msg.setService(_service);
    msg.setObject(_object);
    msg.setFunction(event);
    TransportSocketPtr sock = _socket;
    if (!sock || !sock->send(msg)) {
      qiLogVerbose() << "error while emitting event";
      return;
    }
  }

  static void onEventConnected(RemoteObject* ro, qi::Future<SignalLink> fut, qi::Promise<SignalLink> prom, SignalLink id) {
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
    SignalLink uid = DynamicObject::metaConnect(event, sub);

    boost::recursive_mutex::scoped_lock _lock(_localToRemoteSignalLinkMutex);
    // maintain a map of localsignal -> remotesignal
    //(we only use one remotesignal, for many locals)
    LocalToRemoteSignalLinkMap::iterator it;
    RemoteSignalLinks& rsl = _localToRemoteSignalLink[event];
    rsl.localSignalLink.push_back(uid);

    if (rsl.remoteSignalLink == qi::SignalBase::invalidSignalLink)
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
        qiLogDebug() << "Conversion score " << score << " " << ms->parametersSignature().toString() << " -> "
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
      qiLogDebug() << "connect() to " << event << " gave " << uid << " (new remote connection)";
      if (score >= 0.2)
        rsl.future = _self.async<SignalLink>("registerEvent", _service, event, uid);
      else // we might or might not be capable to convert, ask the remote end to try also
        rsl.future =
            _self.async<SignalLink>("registerEventWithSignature", _service, event, uid, subSignature.toString());
    }
    else
    {
      qiLogDebug() << "connect() to " << event << " gave " << uid << " (reusing remote connection)";
    }

    rsl.future.connect(boost::bind<void>(&onEventConnected, this, _1, prom, uid));
    return prom.future();
  }

  qi::Future<void> RemoteObject::metaDisconnect(SignalLink linkId)
  {
    unsigned int event = linkId >> 32;
    //disconnect locally
    qi::Future<void> fut = DynamicObject::metaDisconnect(linkId);
    if (fut.hasError())
    {
      std::stringstream ss;
      ss << "Disconnection failure for " << linkId << ", error:" << fut.error();
      qiLogWarning() << ss.str();
      return qi::makeFutureError<void>(ss.str());
    }

    boost::recursive_mutex::scoped_lock _lock(_localToRemoteSignalLinkMutex);
    LocalToRemoteSignalLinkMap::iterator it;
    it = _localToRemoteSignalLink.find(event);
    if (it == _localToRemoteSignalLink.end()) {
      qiLogWarning() << "Cant find " << event << " in the localtoremote signal map";
      return fut;
    }
    qi::SignalLink toDisco = qi::SignalBase::invalidSignalLink;
    {
      RemoteSignalLinks &rsl = it->second;
      std::vector<SignalLink>::iterator vslit;
      vslit = std::find(rsl.localSignalLink.begin(), rsl.localSignalLink.end(), linkId);

      if (vslit != rsl.localSignalLink.end()) {
        rsl.localSignalLink.erase(vslit);
      } else {
        qiLogWarning() << "Cant find " << linkId << " in the remote signal vector (event:" << event << ")";
      }

      //only drop the remote connection when no more local connection are registered
      if (rsl.localSignalLink.size() == 0) {
        toDisco = rsl.remoteSignalLink;
        _localToRemoteSignalLink.erase(it);
      }
    }
    if (toDisco != qi::SignalBase::invalidSignalLink) {
      TransportSocketPtr sock = _socket;
      if (sock && sock->isConnected())
        return _self.async<void>("unregisterEvent", _service, event, toDisco);
    }
    return fut;
  }

  void RemoteObject::close(const std::string& reason, bool fromSignal)
  {
    qiLogDebug() << "Socket disconnection";
    TransportSocketPtr socket;
    {
       boost::mutex::scoped_lock lock(_socketMutex);
       socket = _socket;
       _socket.reset();
    }
    if (socket)
    { // Do not hold any lock when invoking signals.
        qiLogDebug() << "Removing connection from socket" << (void*)socket.get();
        socket->messagePendingDisconnect(_service, TransportSocket::ALL_OBJECTS, _linkMessageDispatcher);
        if (!fromSignal)
          socket->disconnected.disconnect(_linkDisconnected);
    }
    std::map<int, qi::Promise<AnyReference> > promises;
    {
      boost::mutex::scoped_lock lock(_promisesMutex);
      promises = _promises;
      _promises.clear();
    }
    // Nobody should be able to add anything to promises at this point.
    std::map<int, qi::Promise<AnyReference> >::iterator it;
    for (it = promises.begin(); it != promises.end(); ++it)
    {
      qiLogVerbose() << "Reporting error for request " << it->first << "(" << reason << ")";
      it->second.setError(reason);
    }

    //@warning: remove connection are not removed
    //          not very important ATM, because RemoteObject
    //          cant be reconnected
  }

 qi::Future<AnyValue> RemoteObject::metaProperty(qi::AnyObject context, unsigned int id)
 {
   qiLogDebug() << "bouncing property";
   // FIXME: perform some validations on this end?
   return _self.async<AnyValue>("property", id);
 }

 qi::Future<void> RemoteObject::metaSetProperty(qi::AnyObject context, unsigned int id, AnyValue val)
 {
   qiLogDebug() << "bouncing setProperty";
   return _self.async<void>("setProperty", id, val);
 }
}

#ifdef _MSC_VER
# pragma warning( pop )
#endif
