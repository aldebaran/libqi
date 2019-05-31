/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#include <boost/make_shared.hpp>

#include <qi/anyobject.hpp>
#include <qi/type/objecttypebuilder.hpp>
#include "boundobject.hpp"

qiLogCategory("qimessaging.boundobject");

const auto invalidValueError = "The value is invalid.";

namespace qi {

  static AnyReference forwardEvent(const GenericFunctionParameters& params,
                                   unsigned int service, unsigned int object,
                                   unsigned int event, Signature sig,
                                   MessageSocketPtr client,
                                   boost::weak_ptr<ObjectHost> context,
                                   const std::string& signature)
  {
    qiLogDebug() << "forwardEvent";
    qi::Message msg;
    // FIXME: would like to factor with serveresult.hpp convertAndSetValue()
    // but we have a setValue/setValues issue
    bool processed = false;
    if (!signature.empty() && client->remoteCapability("MessageFlags", false))
    {
      qiLogDebug() << "forwardEvent attempting conversion to " << signature;
      try
      {
        GenericFunctionParameters res = params.convert(signature);
        // invalid conversion does not throw it seems
        bool valid = true;
        for (unsigned i=0; i<res.size(); ++i)
        {
          if (!res[i].type())
          {
            valid = false;
            break;
          }
        }
        if (valid)
        {
          qiLogDebug() << "forwardEvent success " << res[0].type()->infoString();
          msg.setValues(res, "m", context, client.get());
          msg.addFlags(Message::TypeFlag_DynamicPayload);
          res.destroy();
          processed = true;
        }
      }
      catch(const std::exception& /* e */)
      {
        qiLogDebug() << "forwardEvent failed to convert to forced type";
      }
    }
    if (!processed)
    {
      try {
        msg.setValues(params, sig, context, client.get());
      }
      catch (const std::exception& e)
      {
        qiLogVerbose() << "forwardEvent::setValues exception: " << e.what();
        if (!client->remoteCapability("MessageFlags", false))
          throw e;
        // Delegate conversion to the remote end.
        msg.addFlags(Message::TypeFlag_DynamicPayload);
        msg.setValues(params, "m", context, client.get());
      }
    }
    msg.setService(service);
    msg.setFunction(event);
    msg.setType(Message::Type_Event);
    msg.setObject(object);
    client->send(std::move(msg));
    return AnyReference();
  }

  struct ServiceBoundObject::CancelableKit
  {
    ServiceBoundObject::CancelableMap map;
    boost::mutex                      guard;
  };

  ServiceBoundObject::ServiceBoundObject(unsigned int serviceId, unsigned int objectId,
                                         qi::AnyObject object,
                                         qi::MetaCallType mct,
                                         bool bindTerminate,
                                         boost::optional<boost::weak_ptr<ObjectHost>> owner)
    : ObjectHost(serviceId)
    , _cancelables(boost::make_shared<CancelableKit>())
    , _links()
    , _serviceId(serviceId)
    , _objectId(objectId)
    , _object(object)
    , _callType(mct)
    , _owner(owner)
  {
    _self = createServiceBoundObjectType(this, bindTerminate);
  }

  ServiceBoundObject::~ServiceBoundObject()
  {
    qiLogDebug() << "~ServiceBoundObject()";
    destroy();
    _cancelables.reset();
    ObjectHost::clear();
    qiLogDebug() << "~ServiceBoundObject() reseting object " << _object.use_count();
    _object.reset();
    qiLogDebug() << "~ServiceBoundObject() finishing";
  }

  qi::AnyObject ServiceBoundObject::createServiceBoundObjectType(ServiceBoundObject *self, bool bindTerminate) {
    static qi::ObjectTypeBuilder<ServiceBoundObject>* ob = nullptr;

    static boost::mutex* mutex = nullptr;
    QI_THREADSAFE_NEW(mutex);
    boost::mutex::scoped_lock lock(*mutex);
    if (!ob)
    {
      ob = new qi::ObjectTypeBuilder<ServiceBoundObject>();
      // these are called synchronously by onMessage (and this is needed for
      // _currentSocket), no need for threadsafety here
      ob->setThreadingModel(ObjectThreadingModel_MultiThread);
      /* Network-related stuff.
      */
      ob->advertiseMethod("registerEvent"  , &ServiceBoundObject::registerEvent, MetaCallType_Direct, qi::Message::BoundObjectFunction_RegisterEvent);
      ob->advertiseMethod("unregisterEvent", &ServiceBoundObject::unregisterEvent, MetaCallType_Direct, qi::Message::BoundObjectFunction_UnregisterEvent);
      ob->advertiseMethod("terminate",       &ServiceBoundObject::terminate, MetaCallType_Direct, qi::Message::BoundObjectFunction_Terminate);
      /* GenericObject-related stuff.
      * Those methods could be advertised and implemented by GenericObject itself.
      * But since we already have a wrapper system in place in BoundObject, us it.
      * There is no use-case that requires the methods below without a BoundObject present.
      */
      ob->advertiseMethod("metaObject"     , &ServiceBoundObject::metaObject, MetaCallType_Direct, qi::Message::BoundObjectFunction_MetaObject);
      ob->advertiseMethod("property", &ServiceBoundObject::property, MetaCallType_Queued, qi::Message::BoundObjectFunction_GetProperty);
      ob->advertiseMethod("setProperty", &ServiceBoundObject::setProperty, MetaCallType_Queued, qi::Message::BoundObjectFunction_SetProperty);
      ob->advertiseMethod("properties",       &ServiceBoundObject::properties, MetaCallType_Direct, qi::Message::BoundObjectFunction_Properties);
      ob->advertiseMethod("registerEventWithSignature"  , &ServiceBoundObject::registerEventWithSignature, MetaCallType_Direct, qi::Message::BoundObjectFunction_RegisterEventWithSignature);
    }
    AnyObject result = ob->object(self, &AnyObject::deleteGenericObjectOnly);
    return result;
  }

  // Bound Method
  qi::Future<SignalLink> ServiceBoundObject::registerEvent(unsigned int objectId, unsigned int eventId, SignalLink remoteSignalLinkId) {
    // fetch signature
    const MetaSignal* ms = _object.metaObject().signal(eventId);
    if (!ms)
      throw std::runtime_error("No such signal");
    QI_ASSERT(_currentSocket);
    AnyFunction mc = AnyFunction::fromDynamicFunction(boost::bind(&forwardEvent, _1, _serviceId, _objectId, eventId, ms->parametersSignature(), _currentSocket, weakPtr(), ""));
    qi::Future<SignalLink> linking = _object.connect(eventId, mc);
    auto& linkEntry = _links[_currentSocket][remoteSignalLinkId];
    linkEntry = RemoteSignalLink(linking, eventId);
    return linking.andThen([=](SignalLink linkId) mutable {
      qiLogDebug() << "SBO rl " << remoteSignalLinkId << " ll " << linkId;
      return linkId;
    });
  }

  qi::Future<SignalLink> ServiceBoundObject::registerEventWithSignature(unsigned int objectId, unsigned int eventId, SignalLink remoteSignalLinkId, const std::string& signature) {
    // fetch signature
    const MetaSignal* ms = _object.metaObject().signal(eventId);
    if (!ms)
      throw std::runtime_error("No such signal");
    QI_ASSERT(_currentSocket);
    AnyFunction mc = AnyFunction::fromDynamicFunction(boost::bind(&forwardEvent, _1, _serviceId, _objectId, eventId, ms->parametersSignature(), _currentSocket, weakPtr(), signature));
    qi::Future<SignalLink> linking = _object.connect(eventId, mc);
    auto& linkEntry = _links[_currentSocket][remoteSignalLinkId];
    linkEntry = RemoteSignalLink(linking, eventId);
    return linking.andThen([=](SignalLink linkId) mutable {
      qiLogDebug() << "SBO rl " << remoteSignalLinkId << " ll " << linkId;
      return linkId;
    });
  }

  // Bound Method
  qi::Future<void> ServiceBoundObject::unregisterEvent(unsigned int objectId, unsigned int QI_UNUSED(event), SignalLink remoteSignalLinkId) {
    ServiceSignalLinks&          sl = _links[_currentSocket];
    ServiceSignalLinks::iterator it = sl.find(remoteSignalLinkId);

    if (it == sl.end())
    {
      std::stringstream ss;
      ss << "Unregister request failed for " << remoteSignalLinkId << " " << objectId;
      qiLogError() << ss.str();
      throw std::runtime_error(ss.str());
    }

    auto localSignalLinkId = it->second.localSignalLinkId;
    sl.erase(it);
    if (sl.empty())
      _links.erase(_currentSocket);
    return localSignalLinkId.andThen([=](SignalLink link) {
      return _object.disconnect(link).async();
    }).unwrap();
  }

  // Bound Method
  qi::MetaObject ServiceBoundObject::metaObject(unsigned int objectId) {
    // we inject specials methods here
    return qi::MetaObject::merge(_self.metaObject(), _object.metaObject());
  }


  void ServiceBoundObject::terminate(unsigned int)
  {
    qiLogDebug() << "terminate() received";
    if (_owner)
    {
      if (boost::shared_ptr<ObjectHost> owner = _owner->lock())
        owner->removeObject(_objectId);
      else
        qiLogDebug() << "terminate() received an object with an expired owner";
    }
    else
      qiLogWarning() << "terminate() received on object without an owner";
  }

  static void destroyAbstractFuture(AnyReference value)
  {
    value.destroy();
  }

  void ServiceBoundObject::onMessage(const qi::Message &msg, MessageSocketPtr socket) {
    boost::mutex::scoped_lock lock(_callMutex);
    try {
      if (msg.version() > Message::Header::currentVersion())
      {
        std::stringstream ss;
        ss << "Cannot negotiate QiMessaging connection: "
           << "remote end doesn't support binary protocol v" << msg.version();
        serverResultAdapter(qi::makeFutureError<AnyReference>(ss.str()), Signature(),
                            _gethost(), socket, msg.address(), Signature(), CancelableKitWeak());
        return;
      }

      qiLogDebug() << this << "(" << service() << '/' << _objectId << ") msg " << msg.address() << " " << msg.buffer().size();

      if (msg.object() > _objectId)
      {
        qiLogDebug() << "Passing message to children";
        ObjectHost::onMessage(msg, socket);
        return;
      }

      qi::AnyObject    obj;
      unsigned int     funcId;
      //choose between special function (on BoundObject) or normal calls
      // Manageable functions are at the end of reserver range but dispatch to _object
      const bool isSpecialFunction = msg.function() < Manageable::startId;
      if (isSpecialFunction) {
        obj = _self;
      } else {
        obj = _object;
      }
      funcId = msg.function();

      qi::Signature sigparam;
      GenericFunctionParameters mfp;

      // Validate call target
      if (msg.type() == qi::Message::Type_Call) {
        const qi::MetaMethod *mm = obj.metaObject().method(funcId);
        if (!mm) {
          std::stringstream ss;
          ss << "No such method " << msg.address();
          qiLogError() << ss.str();
          throw std::runtime_error(ss.str());
        }
        sigparam = mm->parametersSignature();
      }

      else if (msg.type() == qi::Message::Type_Post) {
        const qi::MetaSignal *ms = obj.metaObject().signal(funcId);
        if (ms)
          sigparam = ms->parametersSignature();
        else {
          const qi::MetaMethod *mm = obj.metaObject().method(funcId);
          if (mm)
            sigparam = mm->parametersSignature();
          else {
            qiLogError() << "No such signal/method on event message " << msg.address();
            return;
          }
        }
      }
      else if (msg.type() == qi::Message::Type_Cancel)
      {
        unsigned int origMsgId = msg.value("I", socket).to<unsigned int>();
        cancelCall(socket, msg, origMsgId);
        return;
      }
      else
      {
        qiLogError() << "Unexpected message type " << msg.type() << " on " << msg.address();
        return;
      }

      AnyReference ref;
      if (msg.flags() & Message::TypeFlag_DynamicPayload)
        sigparam = "m";
      // ReturnType flag appends a signature to the payload
      Signature originalSignature;
      bool hasReturnType = (msg.flags() & Message::TypeFlag_ReturnType) ? true : false;
      if (hasReturnType)
      {
        originalSignature = sigparam;
        sigparam = "(" + sigparam.toString() + "s)";
      }
      // Since the following code does some AnyReference juggling (assignment
      // with its own sub-parts), it is simpler here to directly manipulate an
      // AnyReference and achieve exception-safety through a scoped, than using
      // an AnyValue.
      bool mustDestroyRef = true;
      ref = msg.value(sigparam, socket).release();
      auto guard = ka::scoped([&]() {
        if (mustDestroyRef)
        {
          ref.destroy();
        }
      });
      std::string returnSignature;
      if (hasReturnType)
      {
        returnSignature = ref[1].to<std::string>();
        ref[1].destroy();
        ref = ref[0];
        sigparam = originalSignature;
      }
      if (sigparam == "m")
      {
        // received dynamically typed argument pack, unwrap
        AnyValue* content = ref.ptr<AnyValue>();
        // steal it (`release` doesn't throw).
        AnyReference pContent = content->release();

        // free the object content
        mustDestroyRef = false; // If next line throws, don't redestroy.
        ref.destroy();
        ref = pContent;
        mustDestroyRef = true; // Reactivate destroy on scope exit.
      }
      mfp = ref.asTupleValuePtr();
      /* Because of 'global' _currentSocket, we cannot support parallel
      * executions at this point.
      * Both on self, and on obj which can use currentSocket() too.
      *
      * So put a lock, and rely on metaCall we invoke being asynchronous for// execution
      * This is decided by _callType, set from BoundObject ctor argument, passed by Server, which
      * uses its internal _defaultCallType, passed to its constructor, default
      * to queued. When Server is instanciated by ObjectHost, it uses the default
      * value.
      *
      * As a consequence, users of currentSocket() must set _callType to Direct.
      * Calling currentSocket multiple times in a row should be avoided.
      */
      switch (msg.type())
      {
      case Message::Type_Call: {
        boost::recursive_mutex::scoped_lock lock(_mutex);
        _currentSocket = socket;

        // Property accessors are insecure to call synchronously
        // because users can customize them.
        const bool isUserDefinedFunction =
            !isSpecialFunction || funcId == 5 /* property */ || funcId == 6 /* setProperty */;
        qi::MetaCallType callType = isUserDefinedFunction ? _callType : MetaCallType_Direct;

        qi::Signature sig = returnSignature.empty() ? Signature() : Signature(returnSignature);
        qi::Future<AnyReference> fut = obj.metaCall(funcId, mfp, callType, sig);
        AtomicIntPtr cancelRequested = boost::make_shared<Atomic<int> >(0);
        {
          qiLogDebug() << this << " Registering future for " << socket.get() << ", message:" << msg.id();
          boost::mutex::scoped_lock futlock(_cancelables->guard);
          _cancelables->map[socket][msg.id()] = std::make_pair(fut, cancelRequested);
        }
        Signature retSig;
        const MetaMethod* mm = obj.metaObject().method(funcId);
        if (mm)
          retSig = mm->returnSignature();
        _currentSocket.reset();

        fut.connect(boost::bind<void>
                    (&ServiceBoundObject::serverResultAdapter, _1, retSig, _gethost(), socket, msg.address(), sig,
                     CancelableKitWeak(_cancelables), cancelRequested));
      }
        break;
      case Message::Type_Post: {
        if (obj == _self) // we need a sync call (see comment above), post does not provide it
          obj.metaCall(funcId, mfp, MetaCallType_Direct);
        else
          obj.metaPost(funcId, mfp);
      }
        break;
      default:
        qiLogError() << "unknown request of type " << (int)msg.type() << " on service: " << msg.address();
      }
      //########################
    } catch (const std::runtime_error &e) {
      if (msg.type() == Message::Type_Call) {
        qi::Promise<AnyReference> prom;
        prom.setError(e.what());
        serverResultAdapter(prom.future(), Signature(), _gethost(), socket, msg.address(), Signature(),
                            CancelableKitWeak(_cancelables));
      }
    } catch (...) {
      if (msg.type() == Message::Type_Call) {
        qi::Promise<AnyReference> prom;
        prom.setError("Unknown error catch");
        serverResultAdapter(prom.future(), Signature(), _gethost(), socket, msg.address(), Signature(),
                            CancelableKitWeak(_cancelables));
      }
    }
  }

  void ServiceBoundObject::cancelCall(MessageSocketPtr socket, const Message& cancelMessage, MessageId origMsgId)
  {
    qiLogDebug() << "Canceling call: " << origMsgId << " on client " << socket.get();
    std::pair<Future<AnyReference>, AtomicIntPtr > fut;
    {
      boost::mutex::scoped_lock lock(_cancelables->guard);
      CancelableMap& cancelableCalls = _cancelables->map;
      CancelableMap::iterator it = cancelableCalls.find(socket);
      if (it == cancelableCalls.end())
      {
        qiLogDebug() << "Socket " << socket.get() << " not recorded";
        return;
      }
      FutureMap::iterator futIt = it->second.find(origMsgId);

      if (futIt == it->second.end())
      {
        qiLogDebug() << "No recorded future for message " << origMsgId;
        return;
      }
      fut = futIt->second;
    }

    // We count the number or requested cancels.
    // ServerResultAdapter can also process some cancels.
    // We want the total amount of effective cancels be equal to
    // how many times cancel has been requested.
    int cancelCount = ++(*fut.second);
    Future<AnyReference>& future = fut.first;
    // this future is from metaCall, canceling invokes only our code, no user code, so it won't block
    future.cancel();

    FutureState state = future.wait(0);
    if (state == FutureState_FinishedWithValue)
    {
      _removeCachedFuture(CancelableKitWeak(_cancelables), socket, origMsgId);
      // Check if we have an underlying future: in that case it needs
      // to be cancelled as well.
      AnyReference val = future.value();
      boost::shared_ptr<GenericObject> ao = qi::detail::getGenericFuture(val);
      if (!ao)
      {
        qiLogDebug() << "Message " << origMsgId << ": return value is not a future.";
        return;
      }

      // Check if serverResultAdapter hasn't run before us and is taking care of
      // cancelling the inner future.
      bool doCancel = false;
      while (cancelCount)
      {
        if (fut.second->setIfEquals(cancelCount, cancelCount - 1))
        {
          doCancel = true;
          break;
        }
        cancelCount = fut.second->load();
      }
      if (!doCancel)
      {
        return;
      }
      // This outer future is 'done', so its completion callback has already been
      // called or is in the process of being called (that would be serverResultAdapter).
      // It will register a completion callback on its inner future (if applicable),
      // so we just need to call cancel.
      // We do the call in async because this may invoke user code, we must not block this thread
      ao->async<void>("cancel");
    }
  }

  void ServiceBoundObject::onSocketDisconnected(MessageSocketPtr client, std::string error)
  {
    // Disconnect event links set for this client.
    if (_onSocketDisconnectedCallback)
      _onSocketDisconnectedCallback(client, error);
    {
      boost::mutex::scoped_lock lock(_cancelables->guard);
      _cancelables->map.erase(client);
    }
    BySocketServiceSignalLinks::iterator it = _links.find(client);
    if (it != _links.end())
    {
      for (ServiceSignalLinks::iterator jt = it->second.begin(); jt != it->second.end(); ++jt)
      {
        _object.disconnect(jt->second.localSignalLinkId.value()).async()
            .then([](Future<void> f) { if (f.hasError()) qiLogError() << f.error(); });
      }
      _links.erase(it);
    }
    removeRemoteReferences(client);
  }

  qi::BoundAnyObject makeServiceBoundAnyObject(unsigned int serviceId, qi::AnyObject object, qi::MetaCallType mct) {
    boost::shared_ptr<ServiceBoundObject> ret = boost::make_shared<ServiceBoundObject>(serviceId, Message::GenericObject_Main, object, mct); // TODO ju
    return ret;
  }

  qi::Future<AnyValue> ServiceBoundObject::property(const AnyValue& prop)
  {
    if (prop.kind() == TypeKind_String)
      return _object.property<AnyValue>(prop.toString());
    else if (prop.kind() == TypeKind_Int)
    { // missing accessor, go to backend
      GenericObject* go = _object.asGenericObject();
      return go->type->property(go->value, _object, static_cast<unsigned int>(prop.toUInt()));
    }
    else
      throw std::runtime_error("Expected int or string for property index");
  }

  Future<void> ServiceBoundObject::setProperty(const AnyValue& prop, AnyValue val)
  {
    qi::Future<void> result;
    if (prop.kind() == TypeKind_String)
      result = _object.setProperty(prop.toString(), val);
    else if (prop.kind() == TypeKind_Int)
    {
      GenericObject* go = _object.asGenericObject();
      result = go->type->setProperty(go->value, _object, static_cast<unsigned int>(prop.toUInt()), val);
    }
    else
      throw std::runtime_error("Expected int or string for property index");

    return result;
  }

  std::vector<std::string> ServiceBoundObject::properties()
  {
    // FIXME implement
    std::vector<std::string> res;
    const MetaObject& mo = _object.metaObject();
    MetaObject::PropertyMap map = mo.propertyMap();
    for (MetaObject::PropertyMap::iterator it = map.begin(); it != map.end(); ++it)
      res.push_back(it->second.name());
    return res;
  }

  void ServiceBoundObject::_removeCachedFuture(CancelableKitWeak kit, MessageSocketPtr sock, MessageId id)
  {
    CancelableKitPtr kitPtr = kit.lock();
    if (!kitPtr)
      return;

    boost::mutex::scoped_lock lock(kitPtr->guard);
    CancelableMap& cancelableCalls = kitPtr->map;
    CancelableMap::iterator it = cancelableCalls.find(sock);

    if (it != cancelableCalls.end())
    {
      FutureMap::iterator futIt = it->second.find(id);
      if (futIt != it->second.end())
      {
        it->second.erase(futIt);
        if (it->second.size() == 0)
          cancelableCalls.erase(it);
      }
    }
  }

  // if 'val' is a qi::Object<> its ownership will be given to the 'host' object
  static inline void convertAndSetValue(Message& ret, AnyReference val,
    const Signature& targetSignature, boost::weak_ptr<ObjectHost> host,
    MessageSocket* socket, const Signature& forcedSignature)
  {
    if (!val.isValid())
      throw std::runtime_error(invalidValueError);

    // We allow forced signature conversion to fail, in which case we
    // go on with original expected signature.
    if (forcedSignature.isValid() && socket->remoteCapability("MessageFlags", false))
    {
      auto conv = val.convert(TypeInterface::fromSignature(forcedSignature));
      qiLogDebug("qimessaging.serverresult")
          << "Converting to forced signature " << forcedSignature.toString()
          << ", data=" << val.type()->infoString() << ", advertised=" << targetSignature.toString()
          << ", success=" << conv->isValid();
      if (conv->type())
      {
        ret.setValue(*conv, "m", host, socket);
        ret.addFlags(Message::TypeFlag_DynamicPayload);
        return;
      }
    }
    ret.setValue(val, targetSignature, host, socket);
  }

  // second bounce when returned type is a future
  void ServiceBoundObject::serverResultAdapterNext(AnyReference val, // the future
                                                   Signature targetSignature,
                                                   boost::weak_ptr<ObjectHost> host,
                                                   MessageSocketPtr socket,
                                                   const qi::MessageAddress& replyaddr,
                                                   const Signature& forcedReturnSignature,
                                                   CancelableKitWeak kit)
  {
    QI_ASSERT_TRUE(val.isValid());
    _removeCachedFuture(kit, socket, replyaddr.messageId);
    if (!socket->isConnected())
    {
      val.destroy();
      qiLogDebug() << "Can't send call result: the socket has been disconnected";
      return;
    }
    qi::Message ret(Message::Type_Reply, replyaddr);
    try {
      TypeKind kind = TypeKind_Unknown;
      boost::shared_ptr<GenericObject> ao = qi::detail::getGenericFuture(val, &kind);
      if (ao->call<bool>("hasError", 0))
      {
        ret.setType(qi::Message::Type_Error);
        ret.setError(ao->call<std::string>("error", 0));
      }
      else if (ao->call<bool>("isCanceled"))
      {
        qiLogDebug() << "Call " << replyaddr.messageId << " has been canceled.";
        if (!socket->sharedCapability(capabilityname::remoteCancelableCalls, false))
        {
          ret.setType(Message::Type_Error);
          ret.setError("Call has been canceled.");
        }
        else
          ret.setType(Message::Type_Canceled);
      }
      else
      {
        // Future<void>::value() give a void* so we need a special handling to
        // produce a real void
        AnyValue value;
        if (kind == TypeKind_Void)
          value = AnyValue(qi::typeOf<void>());
        else
          value = ao->call<AnyValue>("value", 0);
        convertAndSetValue(ret, value.asReference(), targetSignature, host, socket.get(), forcedReturnSignature);
      }
    } catch (const std::exception &e) {
      //be more than safe. we always want to nack the client in case of error
      ret.setType(qi::Message::Type_Error);
      ret.setError(std::string("Uncaught error: ") + e.what());
    } catch (...) {
      //be more than safe. we always want to nack the client in case of error
      ret.setType(qi::Message::Type_Error);
      ret.setError("Unknown error caught while forwarding the answer");
    }
    if (!socket->send(std::move(ret)))
    {
      // TODO: if `convertAndSetValue` transfers ownership of `val` in the object host,
      // `val.destroy()` below won't be enough. Check if it's necessary to destroy
      // `val` inside the host.
      qiLogWarning("qimessaging.serverresult") << "Can't generate an answer for address:" << replyaddr;
    }
    val.destroy();
  }

  void ServiceBoundObject::serverResultAdapter(Future<AnyReference> future,
                                               const qi::Signature& targetSignature,
                                               boost::weak_ptr<ObjectHost> host,
                                               MessageSocketPtr socket,
                                               const qi::MessageAddress& replyaddr,
                                               const Signature& forcedReturnSignature,
                                               CancelableKitWeak kit,
                                               AtomicIntPtr cancelRequested)
  {
    if(!socket->isConnected())
    {
      _removeCachedFuture(kit, socket, replyaddr.messageId);
      future.setOnDestroyed(&destroyAbstractFuture);
      qiLogDebug() << "Can't send call result: the socket has been disconnected";
      return;
    }
    qi::Message ret(Message::Type_Reply, replyaddr);
    if (future.hasError()) {
      ret.setType(qi::Message::Type_Error);
      ret.setError(future.error());
    } else if (future.isCanceled()) {
      ret.setType(Message::Type_Canceled);
      qiLogDebug() << "Call " << replyaddr.messageId << " was cancelled.";
    } else {
      try {
        qi::AnyReference val = future.value();
        boost::shared_ptr<GenericObject> ao = qi::detail::getGenericFuture(val);
        if (ao)
        {
          boost::function<void()> cb = boost::bind(&ServiceBoundObject::serverResultAdapterNext, val, targetSignature,
                                                   host, socket, replyaddr, forcedReturnSignature, kit);
          if (ao->call<bool>("isValid"))
          {
            ao->call<void>("_connect", cb);
            // Check if the atomic is set to true.
            // If it is and we manage to set it to false, we're taking care of cancelling the future.
            if (cancelRequested)
            {
              int cancelCount = cancelRequested->load();
              bool doCancel = false;
              while (cancelCount)
              {
                if (cancelRequested->setIfEquals(cancelCount, cancelCount - 1))
                {
                  doCancel = true;
                  break;
                }
                cancelCount = cancelRequested->load();
              }
              if (doCancel)
              {
                qiLogDebug() << "Cancel requested for call " << replyaddr.messageId;
                ao->call<void>("cancel");
              }
            }
            return;
          }
          else
          {
            ret.setType(Message::Type_Error);
            ret.setError(qi::detail::InvalidFutureError);
          }
        }
        else
        {
          convertAndSetValue(ret, val, targetSignature, host, socket.get(), forcedReturnSignature);
          future.setOnDestroyed(&destroyAbstractFuture);
        }
      } catch (const std::exception &e) {
        //be more than safe. we always want to nack the client in case of error
        ret.setType(qi::Message::Type_Error);
        ret.setError(std::string("Uncaught error: ") + e.what());
      } catch (...) {
        //be more than safe. we always want to nack the client in case of error
        ret.setType(qi::Message::Type_Error);
        ret.setError("Unknown error caught while sending the answer");
      }
    }
    _removeCachedFuture(kit, socket, replyaddr.messageId);
    if (!socket->send(std::move(ret)))
    {
      // TODO: Check if `val` must be destroyed here. Take into account the potential
      // transfer ownership to the object host.
      qiLogWarning("qimessaging.serverresult") << "Can't generate an answer for address:" << replyaddr;
    }
  }

// id 1 is for the service itself, we must not use it for sub-objects
qi::Atomic<unsigned int> ServiceBoundObject::_nextId(2);

}
