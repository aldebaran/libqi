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
#include <qimessaging/message.hpp>
#include <qimessaging/transportsocket.hpp>
#include <qimessaging/binarydecoder.hpp>
#include <qi/log.hpp>
#include <boost/thread/mutex.hpp>
#include <qi/eventloop.hpp>

namespace qi {


  static qi::MetaObject &createRemoteObjectSpecialMetaObject() {
    static qi::MetaObject *mo = 0;

    if (!mo) {

      mo = new qi::MetaObject;
      qi::MetaObjectBuilder mob;
      mob.addMethod("I", "registerEvent::(III)", qi::Message::BoundObjectFunction_RegisterEvent);
      mob.addMethod("v", "unregisterEvent::(III)", qi::Message::BoundObjectFunction_UnregisterEvent);
      mob.addMethod("({I(Isss[(ss)]s)}{I(Is)}s)", "metaObject::(I)", qi::Message::BoundObjectFunction_MetaObject);
      *mo = mob.metaObject();

      assert(mo->methodId("registerEvent::(III)") == qi::Message::BoundObjectFunction_RegisterEvent);
      assert(mo->methodId("unregisterEvent::(III)") == qi::Message::BoundObjectFunction_UnregisterEvent);
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
        qiLogVerbose("RemoteObject") << "Reporting error for request " << it->first << "(socket disconnected)";
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
    qiLogDebug("RemoteObject") << this << "(" << _service << '/' << _object << " msg " << msg.address() << " " << msg.buffer().size();
    if (msg.object() != _object)
    {
      ObjectHost::onMessage(msg, _socket);
      return;
    }
    qi::Promise<GenericValuePtr> promise;
    bool found = false;
    std::map<int, qi::Promise<GenericValuePtr> >::iterator it;

    {
      boost::mutex::scoped_lock lock(_mutex);
      it = _promises.find(msg.id());
      if (it != _promises.end()) {
        promise = _promises[msg.id()];
        _promises.erase(it);
        found = true;
      }
    }

    switch (msg.type()) {
      case qi::Message::Type_Reply:
      {
        if (!found) {
          qiLogError("remoteobject") << "no promise found for req id:" << msg.id()
          << "  obj: " << msg.service() << "  func: " << msg.function();
          return;
        }
        // Get call signature
        MetaMethod* mm =  metaObject().method(msg.function());
        if (!mm)
        {
          qiLogError("remoteobject") << "Result for unknown function "
           << msg.function();
           promise.setError("Result for unknown function");
           return;
        }
        Type* type = Type::fromSignature(mm->sigreturn());
        if (!type)
        {
          promise.setError("Unable to find a type for signature " + mm->sigreturn());
          return;
        }
        BinaryDecoder in(msg.buffer());
        promise.setValue(qi::details::deserialize(type, in, _socket));
        return;
      }
      case qi::Message::Type_Error: {
        qi::BinaryDecoder ds(msg.buffer());
        std::string    err;
        std::string    sig;
        ds.read(sig);
        if (sig != "s") {
          qiLogError("qi.RemoteObject") << "Invalid error signature: " << sig;
          //houston we have an error about the error..
          promise.setError("unknown error");
          return;
        }
        ds.read(err);
        qiLogVerbose("remoteobject") << "Received error message"  << msg.address() << ":" << err;
        promise.setError(err);
        return;
      }
      case qi::Message::Type_Event: {
        SignalBase* sb = signalBase(msg.event());
        if (sb)
        {
          try {
            std::string sig = sb->signature();
            // Remove top-level tuple
            sig = sig.substr(1, sig.length()-2);
            GenericFunctionParameters args = msg.parameters(sig, _socket);
            qiLogDebug("remoteobject") << "Triggering local event listeners";
            sb->trigger(args);
            args.destroy();
          }
          catch (const std::exception& e)
          {
            qiLogWarning("remoteobject") << "Deserialize error on event: " << e.what();
          }
        }
        else
        {
          qiLogWarning("remoteobject") << "Event message on unknown signal " << msg.event();
          qiLogDebug("remoteobject") << metaObject().signalMap().size();
        }
        return;
      }
      default:
        qiLogError("remoteobject") << "Message " << msg.address() << " type not handled: " << msg.type();
        return;
    }
  }


  qi::Future<GenericValuePtr> RemoteObject::metaCall(Manageable*, unsigned int method, const qi::GenericFunctionParameters &in, MetaCallType callType)
  {
    qi::Promise<GenericValuePtr> out;
    qi::Message msg;
    msg.setParameters(in, this);
#ifndef NDEBUG
    std::string sig = metaObject().method(method)->signature();
    sig = signatureSplit(sig)[2];
    // Remove extra tuple layer
    sig = sig.substr(1, sig.length()-2);
    if (sig != msg.signature())
    {
      qiLogWarning("remoteobject") << "call signature mismatch '"
                                   << sig << "' (internal) vs '"
                                   << msg.signature() << "' (message) for:" << metaObject().method(method)->signature();
    }
#endif
    msg.setType(qi::Message::Type_Call);
    msg.setService(_service);
    msg.setObject(_object);
    msg.setFunction(method);
    // qiLogDebug("remoteobject") << this << " metacall " << msg.service() << " " << msg.function() <<" " << msg.id();
    {
      boost::mutex::scoped_lock lock(_mutex);
      if (_promises.find(msg.id()) != _promises.end())
      {
        qiLogError("remoteobject") << "There is already a pending promise with id "
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
        qiLogVerbose("remoteobject") << ss.str();
      } else {
        qiLogError("remoteobject") << ss.str();
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
    msg.setParameters(args, this);
    msg.setType(Message::Type_Post);
    msg.setService(_service);
    msg.setObject(_object);
    msg.setFunction(event);
    if (!_socket->send(msg)) {
      qiLogError("remoteobject") << "error while emiting event";
    }
  }

  static void onEventConnected(qi::Future<unsigned int> fut, qi::Promise<unsigned int> prom, unsigned int id) {
    if (fut.hasError()) {
      prom.setError(fut.error());
      return;
    }
    prom.setValue(id);
  }

  qi::Future<unsigned int> RemoteObject::metaConnect(unsigned int event, const SignalSubscriber& sub)
  {
    qi::Promise<unsigned int> prom;

    // Bind the subscriber locally.
    unsigned int uid = DynamicObject::metaConnect(event, sub);

    qiLogDebug("remoteobject") <<"connect() to " << event <<" gave " << uid;
    qi::Future<unsigned int> fut = _self->call<unsigned int>("registerEvent", _service, event, uid);
    fut.connect(boost::bind<void>(&onEventConnected, _1, prom, uid));
    return prom.future();
  }

  qi::Future<void> RemoteObject::metaDisconnect(unsigned int linkId)
  {
    unsigned int event = linkId >> 16;
    //disconnect locally
    qi::Future<void> fut = DynamicObject::metaDisconnect(linkId);
    if (fut.hasError())
    {
      std::stringstream ss;
      ss << "Disconnection failure for " << linkId << ", error:" << fut.error();
      qiLogWarning("qi.object") << ss.str();
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

}

#ifdef _MSC_VER
# pragma warning( pop )
#endif
