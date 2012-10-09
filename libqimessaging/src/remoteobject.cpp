/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/
#include "remoteobject_p.hpp"
#include "object_p.hpp"
#include "metasignal_p.hpp"
#include <qimessaging/message.hpp>
#include <qimessaging/transportsocket.hpp>
#include <qi/log.hpp>
#include <boost/thread/mutex.hpp>
#include "metaobject_p.hpp"

namespace qi {


  static qi::MetaObject &createRemoteObjectSpecialMetaObject() {
    static qi::MetaObject *mo = 0;

    if (!mo) {
      mo = new qi::MetaObject;
      mo->_p->addMethod("I", "registerEvent::(III)");
      mo->_p->addMethod("v", "unregisterEvent::(III)");
      mo->_p->addMethod("({I(ssI)}{I(sI)})", "metaObject::(I)");

      assert(mo->methodId("registerEvent::(III)") == qi::Message::BoundObjectFunction_RegisterEvent);
      assert(mo->methodId("unregisterEvent::(III)") == qi::Message::BoundObjectFunction_UnregisterEvent);
      assert(mo->methodId("metaObject::(I)") == qi::Message::BoundObjectFunction_MetaObject);
    }
    return *mo;
  }

  RemoteObject::RemoteObject(unsigned int service, qi::TransportSocketPtr socket)
    : _socket()
    , _service(service)
    , _linkMessageDispatcher(0)
    , _self(makeDynamicObjectPtr(this))
  {
    //simple metaObject with only special methods. (<10)
    setMetaObject(createRemoteObjectSpecialMetaObject());
    setTransportSocket(socket);
    //init should be called to makesure the metaObject is valid.
  }

  RemoteObject::RemoteObject(unsigned int service, qi::MetaObject metaObject, TransportSocketPtr socket)
    : _socket(socket)
    , _service(service)
    , _linkMessageDispatcher(0)
    , _self(makeDynamicObjectPtr(this))
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
    if (_socket)
      _socket->messagePendingDisconnect(_service, _linkMessageDispatcher);
    _socket = socket;
    //do not set the socket on the remote object
    if (socket)
      _linkMessageDispatcher = _socket->messagePendingConnect(_service, boost::bind<void>(&RemoteObject::onMessagePending, this, _1));
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

  void RemoteObject::onMessagePending(const qi::Message &msg)
  {
    qi::Promise<GenericValue> promise;
    bool found = false;
    std::map<int, qi::Promise<GenericValue> >::iterator it;

    qiLogDebug("RemoteObject") << this << " msg " << msg.type() << " " << msg.buffer().size();
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
        IDataStream in(msg.buffer());
        promise.setValue(type->deserialize(in));
        return;
      }
      case qi::Message::Type_Error: {
        qi::IDataStream ds(msg.buffer());
        std::string    err;
        std::string    sig;
        ds >> sig;
        if (sig != "s") {
          qiLogError("qi.RemoteObject") << "Invalid error signature: " << sig;
          //houston we have an error about the error..
          promise.setError("unknown error");
          return;
        }
        ds >> err;
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
            sig = signatureSplit(sig)[2];
            sig = sig.substr(1, sig.length()-2);
            GenericFunctionParameters args
              = GenericFunctionParameters::fromBuffer(sig, msg.buffer());
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


  qi::Future<GenericValue> RemoteObject::metaCall(unsigned int method, const qi::GenericFunctionParameters &in, MetaCallType callType)
  {
    qi::Promise<GenericValue> out;
    qi::Message msg;
    msg.setBuffer(in.toBuffer());
#ifndef NDEBUG
    std::string sig = metaObject().method(method)->signature();
    sig = signatureSplit(sig)[2];
    // Remove extra tuple layer
    sig = sig.substr(1, sig.length()-2);
    if (sig != msg.buffer().signature())
    {
      qiLogError("remoteobject") << "call signature mismatch "
        << sig << ' '
        << msg.buffer().signature();
    }
#endif
    msg.setType(qi::Message::Type_Call);
    msg.setService(_service);
    msg.setObject(qi::Message::GenericObject_Main);
    msg.setFunction(method);
    qiLogDebug("remoteobject") << this << " metacall " << msg.service() << " "
     << msg.function() <<" " << msg.id();
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
      if (!_socket->isConnected())
        ss << " Socket is not connected.";
      qiLogError("remoteobject") << ss.str();
      out.setError(ss.str());

      {
        boost::mutex::scoped_lock lock(_mutex);
        _promises.erase(msg.id());
      }
    }
    return out.future();
  }

  void RemoteObject::metaEmit(unsigned int event, const qi::GenericFunctionParameters &args)
  {
    // Bounce the emit request to server
    // TODO: one optimisation that could be done is to trigger the local
    // subscribers immediately.
    // But it is a bit complex, because the server will bounce the
    // event back to us.
    qi::Message msg;
    msg.setBuffer(args.toBuffer());
    msg.setType(Message::Type_Event);
    msg.setService(_service);
    msg.setObject(qi::Message::GenericObject_Main);
    msg.setFunction(event);
    if (!_socket->send(msg)) {
      qiLogError("remoteobject") << "error while emiting event";
    }
  }

  qi::Future<unsigned int> RemoteObject::connect(unsigned int event, const SignalSubscriber& sub)
  {
    // Bind the subscriber locally.
    unsigned int uid = DynamicObject::connect(event, sub);

    qiLogDebug("remoteobject") <<"connect() to " << event <<" gave " << uid;
    return _self->call<unsigned int>("registerEvent", _service, event, uid);
  }

  qi::Future<void> RemoteObject::disconnect(unsigned int linkId)
  {
    unsigned int event = linkId >> 16;
    //disconnect locally
    bool ok = DynamicObject::disconnect(linkId);
    if (!ok)
    {
      std::stringstream ss;
      ss << "Disconnection failure for " << linkId;
      qiLogWarning("qi.object") << ss.str();
      return qi::makeFutureError<void>(ss.str());
    }
    return _self->call<void>("unregisterEvent", _service, event, linkId);
  }

  void RemoteObject::close() {
    if (_socket)
      _socket->messagePendingDisconnect(_service, _linkMessageDispatcher);
  }

}
