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

    *mo = mob.metaObject();

    assert(mo->methodId("registerEvent::(IIL)") == qi::Message::BoundObjectFunction_RegisterEvent);
    assert(mo->methodId("unregisterEvent::(IIL)") == qi::Message::BoundObjectFunction_UnregisterEvent);
    assert(mo->methodId("metaObject::(I)") == qi::Message::BoundObjectFunction_MetaObject);
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
    //close may already have been called. (by Session_Service.close)
    close();
    destroy();
  }

  //### RemoteObject

  void RemoteObject::setTransportSocket(qi::TransportSocketPtr socket) {
    if (socket == _socket)
      return;
    if (_socket) {
      close();
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
    close();
  }

  void RemoteObject::onMetaObject(qi::Future<qi::MetaObject> fut, qi::Promise<void> prom) {
    if (fut.hasError()) {
      prom.setError(fut.error());
      return;
    }
    setMetaObject(fut.value());
    prom.setValue(0);
  }



  //retrieve the metaObject from the network
  qi::Future<void> RemoteObject::fetchMetaObject() {
    qi::Promise<void> prom(qi::FutureCallbackType_Sync);
    qi::Future<qi::MetaObject> fut = _self.call<qi::MetaObject>("metaObject", 0U);
    fut.connect(boost::bind<void>(&RemoteObject::onMetaObject, this, _1, prom));
    return prom.future();
  }

  //should be done in the object thread
  void RemoteObject::onMessagePending(const qi::Message &msg)
  {
    qiLogDebug() << this << "(" << _service << '/' << _object << ") msg " << msg.address() << " " << msg.buffer().size();
    if (msg.object() != _object)
    {
      qiLogDebug() << "Passing message to host";
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
          AnyReference value = msg.value(sig, _socket);

          GenericFunctionParameters args;
          if (sig == "m")
            args = value.asDynamic().asTupleValuePtr();
          else
            args = value.asTupleValuePtr();
          qiLogDebug() << "Triggering local event listeners with args : " << args.size();
          sb->trigger(args);
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


    if (msg.type() != qi::Message::Type_Reply && msg.type() != qi::Message::Type_Error) {
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
      } else  {
        qiLogError() << "no promise found for req id:" << msg.id()
        << "  obj: " << msg.service() << "  func: " << msg.function();
        return;
      }
    }

    switch (msg.type()) {
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
          qi::AnyReference val = msg.value(mm->returnSignature(), _socket);
          promise.setValue(val);
        } catch (std::runtime_error &err) {
          promise.setError(err.what());
        }

        qiLogDebug() << "Message passed to promise";
        return;
      }

      case qi::Message::Type_Error: {
        try {
          static std::string sigerr("m");
          qi::AnyReference gvp = msg.value(sigerr, _socket).asDynamic();
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


  qi::Future<AnyReference> RemoteObject::metaCall(AnyObject, unsigned int method, const qi::GenericFunctionParameters &in, MetaCallType callType)
  {
    MetaMethod *mm = metaObject().method(method);
    if (!mm) {
      std::stringstream ss;
      ss << "Method " << method << " not found on service " << _service;
      return makeFutureError<AnyReference>(ss.str());
    }


    /* The promise will be set:
     - From here in case of error
     - From a network callback, called asynchronously in thread pool
     So it is safe to use a sync promise.
     */
    qi::Promise<AnyReference> out(FutureCallbackType_Sync);
    qi::Message msg;
   // qiLogDebug() << this << " metacall " << msg.service() << " " << msg.function() <<" " << msg.id();
    {
      boost::mutex::scoped_lock lock(_promisesMutex);
      // Check socket while holding the lock to avoid a race with close()
      // where we would add a promise to the map after said map got cleared
      if (!_socket || !_socket->isConnected())
      {
        return makeFutureError<AnyReference>("Socket is not connected");
      }
      if (_promises.find(msg.id()) != _promises.end())
      {
        qiLogError() << "There is already a pending promise with id "
                                   << msg.id();
      }
      _promises[msg.id()] = out;
    }
    qi::Signature funcSig = mm->parametersSignature();
    msg.setValues(in, funcSig, this);
    msg.setType(qi::Message::Type_Call);
    msg.setService(_service);
    msg.setObject(_object);
    msg.setFunction(method);

    TransportSocketPtr sock = _socket;
    //error will come back as a error message
    if (!sock || !sock->isConnected() || !sock->send(msg)) {
      qi::MetaMethod*   meth = metaObject().method(method);
      std::stringstream ss;
      if (meth) {
        ss << "Network error while sending data to method: '";
        ss << meth->toString();;
        ss << "'.";
      } else {
        ss << "Network error while sending data an unknown method (id=" << method << ").";
      }
      if (!sock || !sock->isConnected()) {
        ss << " Socket is not connected.";
        qiLogVerbose() << ss.str();
      } else {
        qiLogError() << ss.str();
      }
      out.setError(ss.str());

      {
        boost::mutex::scoped_lock lock(_promisesMutex);
        _promises.erase(msg.id());
      }
    }
    return out.future();
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

    msg.setValues(in, funcSig, this);
    msg.setType(Message::Type_Post);
    msg.setService(_service);
    msg.setObject(_object);
    msg.setFunction(event);
    TransportSocketPtr sock = _socket;
    if (!sock || !sock->send(msg)) {
      qiLogError() << "error while emitting event";
      return;
    }
  }

  static void onEventConnected(qi::Future<SignalLink> fut, qi::Promise<SignalLink> prom, SignalLink id) {
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

    qiLogDebug() <<"connect() to " << event <<" gave " << uid;
    qi::Future<SignalLink> fut = _self.call<SignalLink>("registerEvent", _service, event, uid);
    fut.connect(boost::bind<void>(&onEventConnected, _1, prom, uid));
    return prom.future();
  }

  qi::Future<void> RemoteObject::metaDisconnect(SignalLink linkId)
  {
    unsigned int event = linkId >> 16;
    //disconnect locally
    qi::Future<void> fut = DynamicObject::metaDisconnect(linkId);
    if (fut.hasError())
    {
      std::stringstream ss;
      ss << "Disconnection failure for " << linkId << ", error:" << fut.error();
      qiLogWarning() << ss.str();
      return qi::makeFutureError<void>(ss.str());
    }
    TransportSocketPtr sock = _socket;
    if (sock && sock->isConnected())
      return _self.call<void>("unregisterEvent", _service, event, linkId);
    return qi::makeFutureError<void>("No remote unregister: socket disconnected");
  }

  void RemoteObject::close()
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
      qiLogVerbose() << "Reporting error for request " << it->first << "(socket disconnected)";
      it->second.setError("Socket disconnected");
    }
  }

 qi::Future<AnyValue> RemoteObject::metaProperty(unsigned int id)
 {
   qiLogDebug() << "bouncing property";
   // FIXME: perform some validations on this end?
   return _self.call<AnyValue>("property", id);
 }

 qi::Future<void> RemoteObject::metaSetProperty(unsigned int id, AnyValue val)
 {
   qiLogDebug() << "bouncing setProperty";
   return _self.call<void>("setProperty", id, val);
 }
}

#ifdef _MSC_VER
# pragma warning( pop )
#endif
