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


  static qi::MetaObject &createRemoteObjectSpecialMetaObject() {
    static qi::MetaObject *mo = 0;

    if (!mo) {

      mo = new qi::MetaObject;
      qi::MetaObjectBuilder mob;
      mob.addMethod("L", "registerEvent::(IIL)", qi::Message::BoundObjectFunction_RegisterEvent);
      mob.addMethod("v", "unregisterEvent::(IIL)", qi::Message::BoundObjectFunction_UnregisterEvent);
      mob.addMethod("({I(Isss[(ss)]s)}{I(Is)}{I(Iss)}s)", "metaObject::(I)", qi::Message::BoundObjectFunction_MetaObject);

      *mo = mob.metaObject();

      assert(mo->methodId("registerEvent::(IIL)") == qi::Message::BoundObjectFunction_RegisterEvent);
      assert(mo->methodId("unregisterEvent::(IIL)") == qi::Message::BoundObjectFunction_UnregisterEvent);
      assert(mo->methodId("metaObject::(I)") == qi::Message::BoundObjectFunction_MetaObject);
    }
    return *mo;
  }

  RemoteObject::RemoteObject(unsigned int service, qi::TransportSocketPtr socket)
    : ObjectHost(service)
    , _socket()
    , _service(service)
    , _object(1)
    , _linkMessageDispatcher(0)
    , _self(makeDynamicObjectPtr(this, false))
  {
    //simple metaObject with only special methods. (<10)
    setMetaObject(createRemoteObjectSpecialMetaObject());
    setTransportSocket(socket);
    //fetchMetaObject should be called to make sure the metaObject is valid.
  }

  RemoteObject::RemoteObject(unsigned int service, unsigned int object, qi::MetaObject metaObject, TransportSocketPtr socket)
    : ObjectHost(service)
    , _socket()
    , _service(service)
    , _object(object)
    , _linkMessageDispatcher(0)
    , _self(makeDynamicObjectPtr(this, false))
  {
    setMetaObject(metaObject);
    setTransportSocket(socket);
  }

  RemoteObject::~RemoteObject()
  {
    //close may already have been called. (by Session_Service.close)
    close();
  }

  //### RemoteObject

  void RemoteObject::setTransportSocket(qi::TransportSocketPtr socket) {
    if (socket == _socket)
      return;
    if (_socket) {
      _socket->messagePendingDisconnect(_service,
        _object <= Message::GenericObject_Main?TransportSocket::ALL_OBJECTS:_object,
        _linkMessageDispatcher);
      _socket->disconnected.disconnect(_linkDisconnected);
    }
    _socket = socket;
    //do not set the socket on the remote object
    if (socket) {
      _linkMessageDispatcher = _socket->messagePendingConnect(_service,
        _object <= Message::GenericObject_Main?TransportSocket::ALL_OBJECTS:_object,
        boost::bind<void>(&RemoteObject::onMessagePending, this, _1));
      _linkDisconnected      = _socket->disconnected.connect (boost::bind<void>(&RemoteObject::onSocketDisconnected, this, _1));
    }
  }

  //should be done in the object thread
  void RemoteObject::onSocketDisconnected(int error)
  {
    {
      boost::mutex::scoped_lock lock(_mutex);
      std::map<int, qi::Promise<GenericValuePtr> >::iterator it = _promises.begin();
      while (it != _promises.end()) {
        qiLogVerbose() << "Reporting error for request " << it->first << "(socket disconnected)";
        it->second.setError("Socket disconnected");
        _promises.erase(it);
        it = _promises.begin();
      }
    }
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
    qi::Promise<void> prom;
    qi::Future<qi::MetaObject> fut = _self->call<qi::MetaObject>("metaObject", 0U);
    fut.connect(boost::bind<void>(&RemoteObject::onMetaObject, this, _1, prom));
    return prom.future();
  }

  //should be done in the object thread
  void RemoteObject::onMessagePending(const qi::Message &msg)
  {
    qiLogDebug() << this << "(" << _service << '/' << _object << " msg " << msg.address() << " " << msg.buffer().size();
    if (msg.object() != _object)
    {
      qiLogDebug() << "Passing message to host";
      ObjectHost::onMessage(msg, _socket);
      return;
    }


    if (msg.type() == qi::Message::Type_Event) {
      SignalBase* sb = signalBase(msg.event());
      if (sb)
      {
        try {
          // Signal associated with properties have incorrect signature,
          // Trust MetaObject.
          //std::string sig = sb->signature();
          const MetaSignal* ms  = _self->metaObject().signal(msg.event());
          std::string sig = ms->signature();
          sig = signatureSplit(sig)[2];

          // Remove top-level tuple
          //sig = sig.substr(1, sig.length()-2);
          GenericFunctionParameters args = msg.value(sig, _socket).asTupleValuePtr();
          qiLogDebug() << "Triggering local event listeners";
          sb->trigger(args);
          args.destroy();
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

    qi::Promise<GenericValuePtr> promise;
    {
      boost::mutex::scoped_lock lock(_mutex);
      std::map<int, qi::Promise<GenericValuePtr> >::iterator it;
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
          qi::GenericValuePtr val = msg.value(mm->sigreturn(), _socket);
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
          qi::GenericValuePtr gvp = msg.value(sigerr, _socket).asDynamic();
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


  qi::Future<GenericValuePtr> RemoteObject::metaCall(Manageable*, unsigned int method, const qi::GenericFunctionParameters &in, MetaCallType callType)
  {
    qi::Promise<GenericValuePtr> out;
    qi::Message msg;
    qi::GenericValuePtr args = msg.setValues(in, this);
#ifndef NDEBUG
    std::string sig = metaObject().method(method)->signature();
    sig = signatureSplit(sig)[2];
    if (sig != args.signature())
    {
      qiLogWarning() << "call signature mismatch '"
                                   << sig << "' (internal) vs '"
                                   << args.signature() << "' (message) for:" << metaObject().method(method)->signature();
    }
#endif
    msg.setType(qi::Message::Type_Call);
    msg.setService(_service);
    msg.setObject(_object);
    msg.setFunction(method);
    // qiLogDebug() << this << " metacall " << msg.service() << " " << msg.function() <<" " << msg.id();
    {
      boost::mutex::scoped_lock lock(_mutex);
      if (_promises.find(msg.id()) != _promises.end())
      {
        qiLogError() << "There is already a pending promise with id "
                                   << msg.id();
      }
      _promises[msg.id()] = out;
    }

    //error will come back as a error message
    if (!_socket || !_socket->isConnected() || !_socket->send(msg)) {
      qi::MetaMethod*   meth = metaObject().method(method);
      std::stringstream ss;
      if (meth) {
        ss << "Network error while sending data to method: '";
        ss << meth->signature();;
        ss << "'.";
      } else {
        ss << "Network error while sending data an unknown method (id=" << method << ").";
      }
      if (!_socket->isConnected()) {
        ss << " Socket is not connected.";
        qiLogVerbose() << ss.str();
      } else {
        qiLogError() << ss.str();
      }
      out.setError(ss.str());

      {
        boost::mutex::scoped_lock lock(_mutex);
        _promises.erase(msg.id());
      }
    }
    return out.future();
  }

  void RemoteObject::metaPost(Manageable*, unsigned int event, const qi::GenericFunctionParameters &args)
  {
    // Bounce the emit request to server
    // TODO: one optimisation that could be done is to trigger the local
    // subscribers immediately.
    // But it is a bit complex, because the server will bounce the
    // event back to us.
    qi::Message msg;
    msg.setValue(qi::makeGenericTuple(args), this);
    msg.setType(Message::Type_Post);
    msg.setService(_service);
    msg.setObject(_object);
    msg.setFunction(event);
    if (!_socket->send(msg)) {
      qiLogError() << "error while emiting event";
    }
  }

  static void onEventConnected(qi::Future<Link> fut, qi::Promise<Link> prom, Link id) {
    if (fut.hasError()) {
      prom.setError(fut.error());
      return;
    }
    prom.setValue(id);
  }

  qi::Future<Link> RemoteObject::metaConnect(unsigned int event, const SignalSubscriber& sub)
  {
    qi::Promise<Link> prom;

    // Bind the subscriber locally.
    Link uid = DynamicObject::metaConnect(event, sub);

    qiLogDebug() <<"connect() to " << event <<" gave " << uid;
    qi::Future<Link> fut = _self->call<Link>("registerEvent", _service, event, uid);
    fut.connect(boost::bind<void>(&onEventConnected, _1, prom, uid));
    return prom.future();
  }

  qi::Future<void> RemoteObject::metaDisconnect(Link linkId)
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
    if (_socket->isConnected())
      return _self->call<void>("unregisterEvent", _service, event, linkId);
    return qi::makeFutureError<void>("No remote unregister: socket disconnected");
  }

  void RemoteObject::close() {
    if (_socket) {
      _socket->messagePendingDisconnect(_service,
        _object <= Message::GenericObject_Main?TransportSocket::ALL_OBJECTS:_object,
        _linkMessageDispatcher);
      _socket->disconnected.disconnect(_linkDisconnected);
    }
  }

 qi::Future<GenericValue> RemoteObject::getProperty(unsigned int id)
 {
   qiLogDebug() << "bouncing getProperty";
   // FIXME: perform some validations on this end?
   return _self->call<GenericValue>("getProperty", id);
 }

 qi::Future<void> RemoteObject::setProperty(unsigned int id, GenericValue val)
 {
   qiLogDebug() << "bouncing setProperty";
   return _self->call<void>("setProperty", id, val);
 }
}

#ifdef _MSC_VER
# pragma warning( pop )
#endif
