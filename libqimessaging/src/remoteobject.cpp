/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2012 Cedric GESTES
*/

#include "remoteobject_p.hpp"
#include "src/object_p.hpp"
#include "src/metasignal_p.hpp"
#include <qimessaging/message.hpp>
#include <qimessaging/transport_socket.hpp>
#include <qi/log.hpp>
#include <boost/thread/mutex.hpp>

namespace qi {

  RemoteObject::RemoteObject()
    : Object()
  {
  }

  RemoteObject::RemoteObject(TransportSocketPtr socket, unsigned int service, qi::MetaObject mo)
    : Object(new RemoteObjectPrivate(socket, service, mo))
  {
  }

  RemoteObject::~RemoteObject()
  {
  }

  void RemoteObject::close() {
    boost::shared_ptr<RemoteObjectPrivate> p = boost::dynamic_pointer_cast<RemoteObjectPrivate>(_p);
    if (p)
      p->close();
  }


  //### RemoteObjectPrivate

  RemoteObjectPrivate::RemoteObjectPrivate(TransportSocketPtr socket, unsigned int service, qi::MetaObject mo)
    : ObjectPrivate(mo)
    , _socket(socket)
    , _service(service)
  {
    _linkMessageDispatcher = _socket->messagePendingConnect(service, boost::bind<void>(&RemoteObjectPrivate::onMessagePending, this, _1));
  }

  RemoteObjectPrivate::~RemoteObjectPrivate()
  {
    //close may already have been called. (by Session_Service.close)
    close();
  }

  void RemoteObjectPrivate::onMessagePending(const qi::Message &msg)
  {
    qi::Promise<MetaFunctionResult>                           promise;
    bool                                                      found = false;
    std::map<int, qi::Promise<MetaFunctionResult> >::iterator it;

    qiLogDebug("RemoteObject") << "msg " << msg.type() << " " << msg.buffer().size();
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
        if (!found) {
          qiLogError("remoteobject") << "no promise found for req id:" << msg.id();
          return;
        }
        promise.setValue(MetaFunctionResult(msg.buffer()));

        return;
      case qi::Message::Type_Error: {
        qi::IDataStream ds(msg.buffer());
        qi::Buffer     buf;
        std::string    sig;
        ds >> sig;
        ds >> buf;
        promise.setError(sig);
        return;
      }
      case qi::Message::Type_Event:
        trigger(msg.function(), MetaFunctionParameters(msg.buffer()));
        return;
      default:
        qiLogError("remoteobject") << "Message (#" << msg.id() << ") type not handled: " << msg.type();
        return;
    }
  }


  qi::Future<MetaFunctionResult> RemoteObjectPrivate::metaCall(unsigned int method, const qi::MetaFunctionParameters &in, qi::Object::MetaCallType callType)
  {
    qi::Promise<MetaFunctionResult> out;
    qi::Message msg;
    msg.setBuffer(in.getBuffer());
    msg.setType(qi::Message::Type_Call);
    msg.setService(_service);
    msg.setObject(qi::Message::Object_Main);
    msg.setFunction(method);

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
    if (!_socket->send(msg)) {
      qiLogError("remoteobject") << "error while sending answer";
      qi::MetaMethod*   meth = metaObject().method(method);
      std::stringstream ss;
      if (meth) {
        ss << "Network error while sending data to method: '";
        ss << meth->signature();;
        ss << "'";
      } else {
        ss << "Network error while sending data an unknown method (id=" << method << ")";
      }
      out.setError(ss.str());

      {
        boost::mutex::scoped_lock lock(_mutex);
        _promises.erase(msg.id());
      }
    }
    return out.future();
  }

  void RemoteObjectPrivate::metaEmit(unsigned int event, const qi::MetaFunctionParameters &args)
  {
    // Bounce the emit request to server
    // TODO: one optimisation that could be done is to trigger the local
    // subscribers immediately.
    // But it is a bit complex, because the server will bounce the
    // event back to us.
    qiLogError("Not implemented yet lol");
    qi::Message msg;
    msg.setBuffer(args.getBuffer());
    msg.setType(Message::Type_Event);
    msg.setService(_service);
    msg.setObject(qi::Message::Object_Main);
    msg.setFunction(event);
    if (!_socket->send(msg)) {
      qiLogError("remoteobject") << "error while registering event";
    }
  }

  unsigned int RemoteObjectPrivate::connect(unsigned int event, const SignalSubscriber& sub)
  {
    // Bind the function locally.
    unsigned int uid = ObjectPrivate::connect(event, sub);
    // Notify the Service that we are interested in its event.
    // Provide our uid as payload
    qi::Message msg;
    qi::Buffer buf;
    qi::ODataStream ds(buf);
    ds << _service << event << uid;
    msg.setBuffer(buf);
    msg.setObject(qi::Message::Object_Main);
    msg.setType(Message::Type_Event);
    msg.setService(Message::Service_Server);
    msg.setFunction(Message::ServerFunction_RegisterEvent);

    if (!_socket->send(msg)) {
      qiLogError("remoteobject") << "error while registering event";
    }
    qiLogDebug("remoteobject") <<"connect() to " << event <<" gave " << uid;
    return uid;
  }

  bool RemoteObjectPrivate::disconnect(unsigned int linkId)
  {
    unsigned int event = linkId >> 16;

    ObjectPrivate::SignalSubscriberMap::iterator i = _subscribers.find(event);
    if (i == _subscribers.end())
    {
      qiLogWarning("qi.object") << "Disconnect on non instanciated signal";
      return false;
    }

    if (ObjectPrivate::disconnect(linkId))
    {
      // Tell the remote we are no longer interested.
      qi::Message msg;
      qi::Buffer buf;
      qi::ODataStream ds(buf);
      ds << _service << event << linkId;
      msg.setBuffer(buf);
      msg.setType(Message::Type_Event);
      msg.setService(Message::Service_Server);
      msg.setObject(Message::Object_Main);
      msg.setFunction(Message::ServerFunction_UnregisterEvent);
      if (!_socket->send(msg)) {
        qiLogError("remoteobject") << "error while registering event";
      }
      return true;
    }
    else
    {
      qiLogWarning("remoteobject") << "Disconnect failure on " << linkId;
      return false;
    }
  }

  void RemoteObjectPrivate::close() {
    _socket->messagePendingDisconnect(_service, _linkMessageDispatcher);
  }

}
