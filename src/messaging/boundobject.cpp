/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#include <boost/make_shared.hpp>

#include <qi/anyobject.hpp>
#include <qi/type/objecttypebuilder.hpp>
#include <src/type/signal_p.hpp>
#include "boundobject.hpp"
#include "qi/type/detail/typeinterface.hpp"

const auto logCategory = "qimessaging.boundobject";
qiLogCategory(logCategory);

const auto invalidValueError = "The value is invalid.";

// Helper for debug logs of the class. Can only be called from non static member functions of
// `qi::BoundObject`.
#define QI_LOG_DEBUG_BOUNDOBJECT() \
  qiLogDebug() << this << " (service=" << _serviceId << ", object=" << _objectId << ") - "

namespace qi
{

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
          msg.setValues(res, "m", context, client);
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
        msg.setValues(params, sig, context, client);
      }
      catch (const std::exception& e)
      {
        qiLogVerbose() << "forwardEvent::setValues exception: " << e.what();
        if (!client->remoteCapability("MessageFlags", false))
          throw e;
        // Delegate conversion to the remote end.
        msg.addFlags(Message::TypeFlag_DynamicPayload);
        msg.setValues(params, "m", context, client);
      }
    }
    msg.setService(service);
    msg.setFunction(event);
    msg.setType(Message::Type_Event);
    msg.setObject(object);
    client->send(std::move(msg));
    return AnyReference();
  }

  struct BoundObject::CancelableKit
  {
    BoundObject::CancelableMap map;
    boost::mutex                      guard;
  };

  BoundObject::BoundObject(unsigned int serviceId,
                           unsigned int objectId,
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
    _self = createBoundObjectType(this, bindTerminate);
    QI_LOG_DEBUG_BOUNDOBJECT() << "Constructing a BoundObject";
  }

  BoundObject::~BoundObject()
  {
    _cancelables.reset();
    ObjectHost::clear();
  }

  qi::AnyObject BoundObject::createBoundObjectType(BoundObject *self, bool /*bindTerminate*/) {
    static qi::ObjectTypeBuilder<BoundObject>* ob = nullptr;

    static boost::mutex* mutex = nullptr;
    QI_THREADSAFE_NEW(mutex);
    boost::mutex::scoped_lock lock(*mutex);
    if (!ob)
    {
      ob = new qi::ObjectTypeBuilder<BoundObject>();
      // these are called synchronously by onMessage (and this is needed for
      // _currentSocket), no need for threadsafety here
      ob->setThreadingModel(ObjectThreadingModel_MultiThread);
      /* Network-related stuff.
      */
      ob->advertiseMethod("registerEvent"  , &BoundObject::registerEvent, MetaCallType_Direct, qi::Message::BoundObjectFunction_RegisterEvent);
      ob->advertiseMethod("unregisterEvent", &BoundObject::unregisterEvent, MetaCallType_Direct, qi::Message::BoundObjectFunction_UnregisterEvent);
      ob->advertiseMethod("terminate",       &BoundObject::terminate, MetaCallType_Direct, qi::Message::BoundObjectFunction_Terminate);
      /* GenericObject-related stuff.
      * Those methods could be advertised and implemented by GenericObject itself.
      * But since we already have a wrapper system in place in BoundObject, us it.
      * There is no use-case that requires the methods below without a BoundObject present.
      */
      ob->advertiseMethod("metaObject"     , &BoundObject::metaObject, MetaCallType_Direct, qi::Message::BoundObjectFunction_MetaObject);
      ob->advertiseMethod("property", &BoundObject::property, MetaCallType_Queued, qi::Message::BoundObjectFunction_GetProperty);
      ob->advertiseMethod("setProperty", &BoundObject::setProperty, MetaCallType_Queued, qi::Message::BoundObjectFunction_SetProperty);
      ob->advertiseMethod("properties",       &BoundObject::properties, MetaCallType_Direct, qi::Message::BoundObjectFunction_Properties);
      ob->advertiseMethod("registerEventWithSignature"  , &BoundObject::registerEventWithSignature, MetaCallType_Direct, qi::Message::BoundObjectFunction_RegisterEventWithSignature);
    }
    AnyObject result = ob->object(self, &AnyObject::deleteGenericObjectOnly);
    return result;
  }

  // Bound Method
  qi::Future<SignalLink> BoundObject::registerEvent(unsigned int /*objectId*/, unsigned int eventId, SignalLink remoteSignalLinkId) {
    // fetch signature
    const MetaSignal* ms = _object.metaObject().signal(eventId);
    if (!ms)
      throw std::runtime_error("No such signal");
    QI_ASSERT(_currentSocket);
    AnyFunction mc = AnyFunction::fromDynamicFunction(boost::bind(&forwardEvent, _1, _serviceId, _objectId, eventId, ms->parametersSignature(), _currentSocket, asHostWeakPtr(), ""));
    qi::Future<SignalLink> linking = _object.connect(eventId, mc);
    auto& linkEntry = _links[_currentSocket][remoteSignalLinkId];
    linkEntry = RemoteSignalLink(linking, eventId);
    return linking.andThen([=](SignalLink linkId) mutable {
      QI_LOG_DEBUG_BOUNDOBJECT() << "Registered event remote_signal_link=" << remoteSignalLinkId
                                 << " local_link=" << linkId;
      return linkId;
    });
  }

  qi::Future<SignalLink> BoundObject::registerEventWithSignature(unsigned int /*objectId*/, unsigned int eventId, SignalLink remoteSignalLinkId, const std::string& signature) {
    // fetch signature
    const MetaSignal* ms = _object.metaObject().signal(eventId);
    if (!ms)
      throw std::runtime_error("No such signal");
    QI_ASSERT(_currentSocket);
    AnyFunction mc = AnyFunction::fromDynamicFunction(boost::bind(&forwardEvent, _1, _serviceId, _objectId, eventId, ms->parametersSignature(), _currentSocket, asHostWeakPtr(), signature));
    qi::Future<SignalLink> linking = _object.connect(eventId, mc);
    auto& linkEntry = _links[_currentSocket][remoteSignalLinkId];
    linkEntry = RemoteSignalLink(linking, eventId);
    return linking.andThen([=](SignalLink linkId) mutable {
      QI_LOG_DEBUG_BOUNDOBJECT() << "Registered event remote_signal_link=" << remoteSignalLinkId
                                 << " local_link=" << linkId;
      return linkId;
    });
  }

  // Bound Method
  qi::Future<void> BoundObject::unregisterEvent(unsigned int objectId, unsigned int QI_UNUSED(event), SignalLink remoteSignalLinkId)
  {
    // The invalid signal link is never connected, therefore the disconnection
    // is always successful.
    if (!isValidSignalLink(remoteSignalLinkId))
      return futurize();

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
  qi::MetaObject BoundObject::metaObject(unsigned int /*objectId*/) {
    // we inject specials methods here
    return qi::MetaObject::merge(_self.metaObject(), _object.metaObject());
  }


  void BoundObject::terminate(unsigned int)
  {
    QI_LOG_DEBUG_BOUNDOBJECT() << "terminate() received";
    if (_owner)
    {
      if (boost::shared_ptr<ObjectHost> owner = _owner->lock())
        owner->removeObject(_objectId);
      else
        QI_LOG_DEBUG_BOUNDOBJECT() << "terminate() received an object with an expired owner";
    }
    else
      qiLogWarning() << "terminate() received on object without an owner";
  }

  static void destroyAbstractFuture(AnyReference value)
  {
    value.destroy();
  }

  DispatchStatus BoundObject::onMessage(const qi::Message& msg, MessageSocketPtr socket)
  {
    boost::recursive_mutex::scoped_lock lock(_callMutex);
    bool exceptionWasThrown = false;
    try {
      if (msg.version() > Message::Header::currentVersion())
      {
        std::stringstream ss;
        ss << "Cannot negotiate QiMessaging connection: "
           << "remote end doesn't support binary protocol v" << msg.version();
        serverResultAdapter(qi::makeFutureError<AnyReference>(ss.str()), Signature(),
                            _gethost(), socket, msg.address(), Signature(), CancelableKitWeak());
        return DispatchStatus::MessageHandled_WithError;
      }

      QI_LOG_DEBUG_BOUNDOBJECT() << "msg " << msg.address()
                                 << " type=" << msg.type()
                                 << ", size=" << msg.buffer().size();

      QI_ASSERT_TRUE(msg.object() == _objectId);
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

      qi::Signature methodParametersSignature;

      // Validate call target
      if (msg.type() == qi::Message::Type_Call) {
        const qi::MetaMethod *mm = obj.metaObject().method(funcId);
        if (!mm) {
          std::stringstream ss;
          ss << "No such method " << msg.address();
          qiLogError() << ss.str();
          throw std::runtime_error(ss.str());
        }
        methodParametersSignature = mm->parametersSignature();
      }

      else if (msg.type() == qi::Message::Type_Post) {
        const qi::MetaSignal *ms = obj.metaObject().signal(funcId);
        if (ms)
          methodParametersSignature = ms->parametersSignature();
        else {
          const qi::MetaMethod *mm = obj.metaObject().method(funcId);
          if (mm)
            methodParametersSignature = mm->parametersSignature();
          else {
            qiLogError() << "No such signal/method on event message " << msg.address();
            return DispatchStatus::MessageHandled_WithError;
          }
        }
      }
      else if (msg.type() == qi::Message::Type_Cancel)
      {
        unsigned int origMsgId = msg.value("I", socket).to<unsigned int>();
        cancelCall(socket, msg, origMsgId);
        return DispatchStatus::MessageHandled;
      }
      else
      {
        qiLogError() << "Unexpected message type " << Message::typeToString(msg.type()) << " ("
                     << msg.type() << ") on " << msg.address();
        return DispatchStatus::MessageNotHandled;
      }

      qi::Signature messageValueSignature = methodParametersSignature;

      // If the DynamicPayload flag is set, then the message value is a dynamic value instead.
      if (msg.flags() & Message::TypeFlag_DynamicPayload)
        messageValueSignature = qi::Signature::fromType(Signature::Type_Dynamic);

      // If the ReturnType flag is set, the parameters tuple sent by the caller has
      // its signature appended to it. In that case, we adapt the message value
      // expected signature so that we may deserialize the signature of the tuple.
      bool hasReturnType = (msg.flags() & Message::TypeFlag_ReturnType) ? true : false;
      if (hasReturnType)
      {
        static const auto typeOfSignature = qi::typeOf<Signature>();
        messageValueSignature = "(" + messageValueSignature.toString() + typeOfSignature->signature().toString() + ")";
      }
      // This value is kept alive so that we may reference its parts, which
      // contain the list of the function parameters.
      auto messageValue = msg.value(messageValueSignature, socket);
      std::string returnSignature;
      // The reference to the tuple value containing the function parameters.
      AnyReference parametersTuple;
      if (hasReturnType)
      {
        // The value contained in the message is a tuple of 2 elements: the
        // tuple of parameters and a string representation of the expected
        // return value type signature.
        parametersTuple = messageValue[0];
        returnSignature = messageValue[1].to<std::string>();
      }
      else
        // The value contained in the message is the tuple of parameters.
        parametersTuple = messageValue.asReference();
      GenericFunctionParameters parameters = parametersTuple.asTupleValuePtr();
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
        qi::Future<AnyReference> fut = obj.metaCall(funcId, parameters, callType, sig);
        AtomicIntPtr cancelRequested = boost::make_shared<Atomic<int> >(0);
        {
          QI_LOG_DEBUG_BOUNDOBJECT()
            << "Registering future for " << socket.get() << ", message:" << msg.id();
          boost::mutex::scoped_lock futlock(_cancelables->guard);
          _cancelables->map[socket][msg.id()] = std::make_pair(fut, cancelRequested);
        }
        Signature retSig;
        const MetaMethod* mm = obj.metaObject().method(funcId);
        if (mm)
          retSig = mm->returnSignature();
        _currentSocket.reset();

        fut.connect(boost::bind<void>
                    (&BoundObject::serverResultAdapter, _1, retSig, _gethost(), socket, msg.address(), sig,
                     CancelableKitWeak(_cancelables), cancelRequested));
      }
        break;
      case Message::Type_Post: {
        if (obj == _self) // we need a sync call (see comment above), post does not provide it
          obj.metaCall(funcId, parameters, MetaCallType_Direct);
        else
          obj.metaPost(funcId, parameters);
      }
        break;
      default:
        qiLogError() << "unknown request of type " << (int)msg.type() << " on service: " << msg.address();
      }
      //########################
    } catch (const std::runtime_error &e) {
      exceptionWasThrown = true;
      if (msg.type() == Message::Type_Call) {
        qi::Promise<AnyReference> prom;
        prom.setError(e.what());
        serverResultAdapter(prom.future(), Signature(), _gethost(), socket, msg.address(), Signature(),
                            CancelableKitWeak(_cancelables));
      }
    } catch (...) {
      exceptionWasThrown = true;
      if (msg.type() == Message::Type_Call) {
        qi::Promise<AnyReference> prom;
        prom.setError("Unknown error catch");
        serverResultAdapter(prom.future(), Signature(), _gethost(), socket, msg.address(), Signature(),
                            CancelableKitWeak(_cancelables));
      }
    }

    if (!exceptionWasThrown)
      return DispatchStatus::MessageHandled;
    else
      return DispatchStatus::MessageHandled_WithError;
  }

  void BoundObject::cancelCall(MessageSocketPtr socket, const Message& /*cancelMessage*/, MessageId origMsgId)
  {
    QI_LOG_DEBUG_BOUNDOBJECT() << "Canceling call: " << origMsgId << " on client " << socket.get();
    std::pair<Future<AnyReference>, AtomicIntPtr > fut;
    {
      boost::mutex::scoped_lock lock(_cancelables->guard);
      CancelableMap& cancelableCalls = _cancelables->map;
      CancelableMap::iterator it = cancelableCalls.find(socket);
      if (it == cancelableCalls.end())
      {
        QI_LOG_DEBUG_BOUNDOBJECT() << "Socket " << socket.get() << " not recorded";
        return;
      }
      FutureMap::iterator futIt = it->second.find(origMsgId);

      if (futIt == it->second.end())
      {
        QI_LOG_DEBUG_BOUNDOBJECT() << "No recorded future for message " << origMsgId;
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
        QI_LOG_DEBUG_BOUNDOBJECT() << "Message " << origMsgId << ": return value is not a future.";
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

  qi::BoundObjectPtr makeServiceBoundObjectPtr(unsigned int serviceId,
                                               qi::AnyObject object,
                                               qi::MetaCallType mct)
  {
    return BoundObject::makePtr(serviceId, Message::GenericObject_Main, object, mct);
  }

  qi::Future<AnyValue> BoundObject::property(const AnyValue& prop)
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

  Future<void> BoundObject::setProperty(const AnyValue& prop, AnyValue val)
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

  std::vector<std::string> BoundObject::properties()
  {
    // FIXME implement
    std::vector<std::string> res;
    const MetaObject& mo = _object.metaObject();
    MetaObject::PropertyMap map = mo.propertyMap();
    for (MetaObject::PropertyMap::iterator it = map.begin(); it != map.end(); ++it)
      res.push_back(it->second.name());
    return res;
  }

  bool BoundObject::bindToSocket(const MessageSocketPtr& socket) noexcept
  {
    if (!socket)
      return false;

    QI_LOG_DEBUG_BOUNDOBJECT() << "Binding to socket " << socket;
    auto syncConnectionList = _messageDispatchConnectionList.synchronize();
    const auto end = syncConnectionList->end();
    auto connectionIt =
      std::find_if(syncConnectionList->begin(), end,
                   [&](const MessageDispatchConnection& conn) { return conn.socket() == socket; });
    if (connectionIt != end)
      // This object is already accepting messages from this socket, do nothing.
      return false;

    MessageDispatcher::MessageHandler handler =
      track([this, socket](const Message& msg) { return onMessage(msg, socket); }, weak_from_this());
    syncConnectionList->emplace_back(socket,
                                     MessageDispatcher::RecipientId{ _serviceId, _objectId },
                                     std::move(handler));
    return true;
  }

  bool BoundObject::unbindFromSocket(const MessageSocketPtr& socket) noexcept
  {
    if (!socket)
      return false;

    QI_LOG_DEBUG_BOUNDOBJECT() << "Unbinding from socket " << socket;

    // We consider that this method was a success if any of the following actions had an effect.
    const auto removedConnections = removeConnections(socket);
    const auto removedCancelables = removeCancelables(socket);
    const auto removedLinks = removeLinks(socket);

    // Remove all hosted objects that were created in the context of this socket for this object.
    QI_LOG_DEBUG_BOUNDOBJECT() << "Removing children objects from socket " << socket;
    const auto removedObjects = removeObjectsFromSocket(socket);

    QI_LOG_DEBUG_BOUNDOBJECT() << "Calling callback of socket disconnection";
    ka::invoke_catch(
      exceptionLogError(logCategory,
                        "The callback called when a socket is unbound has thrown an exception"),
      [&] {
        // Do not lock the value when calling the callback to avoid a deadlock if the callback
        // is reset within itself.
        auto callback = _onSocketUnboundCallback.get();
        if (callback)
          callback(socket);
      });

    return removedConnections != 0
        || removedCancelables != 0
        || removedLinks != 0
        || removedObjects != 0;
  }

  void BoundObject::_removeCachedFuture(CancelableKitWeak kit, MessageSocketPtr sock, MessageId id)
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
    MessageSocketPtr socket, const Signature& forcedSignature)
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
  void BoundObject::serverResultAdapterNext(AnyReference val, // the future
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
        convertAndSetValue(ret, value.asReference(), targetSignature, host, socket, forcedReturnSignature);
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

  void BoundObject::serverResultAdapter(Future<AnyReference> future,
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
          boost::function<void()> cb = boost::bind(&BoundObject::serverResultAdapterNext, val, targetSignature,
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
          convertAndSetValue(ret, val, targetSignature, host, socket, forcedReturnSignature);
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

  std::atomic<unsigned int> BoundObject::_nextId(
    Message::GenericObject_Main + 1 // Start the object id values after the ones fixed by the protocol.
  );

  std::size_t BoundObject::removeConnections(const MessageSocketPtr& socket) noexcept
  {
    QI_LOG_DEBUG_BOUNDOBJECT() << "Removing connections to socket " << socket;

    auto syncConnectionList = _messageDispatchConnectionList.synchronize();
    auto end = syncConnectionList->end();
    auto newEnd = std::remove_if(syncConnectionList->begin(), end,
                                 [&](const MessageDispatchConnection& conn) {
                                   return conn.socket() == socket;
                                 });

    const auto count = end - newEnd;
    QI_ASSERT_TRUE(count >= 0);

    syncConnectionList->erase(newEnd, end);
    return static_cast<std::size_t>(count);
  }

  std::size_t BoundObject::removeCancelables(const MessageSocketPtr& socket) noexcept
  {
    QI_LOG_DEBUG_BOUNDOBJECT() << "Removing cancelables from socket " << socket;
    boost::mutex::scoped_lock lock(_cancelables->guard);
    return _cancelables->map.erase(socket);
  }

  std::size_t BoundObject::removeLinks(const MessageSocketPtr& socket) noexcept
  {
    QI_LOG_DEBUG_BOUNDOBJECT() << "Disconnecting links from socket " << socket;

    boost::recursive_mutex::scoped_lock lock(_callMutex);
    auto it = _links.find(socket);

    std::size_t count = 0;

    if (it != _links.end())
    {
      count = it->second.size();
      for (const auto& linkSlot : it->second)
      {
        // FIXME: Do this in the destructor of `RemoteSignalLink` instead, and make it move only.
        const auto remoteLink = linkSlot.second;
        _object.disconnect(remoteLink.localSignalLinkId.value()).async().then([](Future<void> f) {
          if (f.hasError())
            qiLogError() << f.error();
        });
      }
      _links.erase(it);
    }

    return count;
  }

  namespace detail
  {
    namespace boundObject
    {
      SocketBinding::SocketBinding() noexcept = default;

      SocketBinding::SocketBinding(BoundObjectPtr object, MessageSocketPtr socket) noexcept
        : _object(object)
        , _socket(socket)
      {
        QI_ASSERT_NOT_NULL(_object);
        QI_ASSERT_NOT_NULL(socket);
        const auto res = _object->bindToSocket(socket);
        QI_IGNORE_UNUSED(res);
        QI_ASSERT_TRUE(res);
      }

      SocketBinding::SocketBinding(SocketBinding&&) noexcept = default;

      SocketBinding& SocketBinding::operator=(SocketBinding&& other) noexcept
      {
        if (&other == this)
          return *this;

        reset();
        _object = ka::exchange(other._object, {});
        _socket = ka::exchange(other._socket, {});
        return *this;
      }

      SocketBinding::~SocketBinding()
      {
        reset();
      }

      bool SocketBinding::operator==(const SocketBinding& rhs) const noexcept
      {
        return _object == rhs._object
          && !_socket.owner_before(rhs._socket)
          && !rhs._socket.owner_before(_socket);
      }

      bool SocketBinding::operator<(const SocketBinding& rhs) const noexcept
      {
        return _object < rhs._object ||
               (!(rhs._object < _object) && _socket.owner_before(rhs._socket));
      }

      void SocketBinding::reset() noexcept
      {
        if (!_object)
          return;
        if (auto shSock = _socket.lock())
        {
          const auto res = _object->unbindFromSocket(shSock);
          QI_IGNORE_UNUSED(res);
          QI_ASSERT_TRUE(res);
        }
      }
    }
  }

}
