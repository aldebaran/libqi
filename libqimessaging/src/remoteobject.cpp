/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2012 Cedric GESTES
*/

#include "remoteobject_p.hpp"
#include "src/object_p.hpp"
#include "src/metaevent_p.hpp"
#include <qimessaging/message.hpp>
#include <qimessaging/transport_socket.hpp>
#include <qi/log.hpp>
#include <boost/thread/mutex.hpp>

namespace qi {

RemoteObject::RemoteObject(qi::TransportSocket *ts, unsigned int service, qi::MetaObject *mo)
  : _ts(ts)
  , _service(service)
{
  _ts->addCallbacks(this);
  _p->setMetaObject(mo);
}

RemoteObject::~RemoteObject()
{
  if (_ts) {
    _ts->disconnect();
    _ts->removeCallbacks(this);
  }
  delete _ts;
}

void RemoteObject::onSocketTimeout(TransportSocket *client, int id, void *data)
{
  {
    boost::mutex::scoped_lock lock(_mutex);
    std::map<int, qi::Promise<MetaFunctionResult> >::iterator it = _promises.find(id);
    if (it != _promises.end())
    {
      it->second.setError("network timeout");
      _promises.erase(it);
    }
  }
}

void RemoteObject::onSocketReadyRead(TransportSocket *client, int id, void *data)
{
  qi::Promise<MetaFunctionResult>                     promise;
  bool found = false;
  qi::Message                                msg;
  std::map<int, qi::Promise<MetaFunctionResult> >::iterator it;

  client->read(id, &msg);
  qiLogDebug("RemoteObject") << "msg " << msg.type() << " " << msg.buffer().size();
  {
    boost::mutex::scoped_lock lock(_mutex);
    it = _promises.find(id);
    if (it != _promises.end()) {
      promise = _promises[id];
      _promises.erase(it);
      found = true;
    }
  }

  switch (msg.type()) {
    case qi::Message::Type_Reply:
      if (!found) {
        qiLogError("remoteobject") << "no promise found for req id:" << id;
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
      qiLogError("remoteobject") << "Message (#" << id << ") type not handled: " << msg.type();
      return;
  }

}


qi::Future<MetaFunctionResult> RemoteObject::metaCall(unsigned int method, const qi::MetaFunctionParameters &in, MetaCallType callType)
{
  qi::Promise<MetaFunctionResult> out;
  qi::Message msg;
  msg.setBuffer(in.getBuffer());
  msg.setType(qi::Message::Type_Call);
  msg.setService(_service);
  msg.setObject(qi::Message::Object_Main);
  //todo handle failure
  msg.setFunction(method);

  if (!_ts->isConnected()) {
    out.setError("Not connected");
    return out.future();
  }
  //allocated from caller, owned by us then. (clean up by onReadyRead)

  {
    boost::mutex::scoped_lock lock(_mutex);
    if (_promises.find(msg.id()) != _promises.end())
    {
      qiLogError("remoteobject") << "There is already a pending promise with id "
                                 << msg.id();
    }
    _promises[msg.id()] = out;
  }
  if (!_ts->send(msg)) {
    qiLogError("remoteobject") << "error while sending answer";
    qi::MetaMethod *meth = metaObject().method(method);
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

void RemoteObject::metaEmit(unsigned int event, const qi::MetaFunctionParameters &args)
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
  if (!_ts->send(msg)) {
    qiLogError("remoteobject") << "error while registering event";
  }
}

unsigned int RemoteObject::connect(unsigned int event, const SignalSubscriber& sub)
{
  // Bind the function locally.
  unsigned int uid = Object::connect(event, sub);
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

  if (!_ts->send(msg)) {
    qiLogError("remoteobject") << "error while registering event";
  }
  qiLogDebug("remoteobject") <<"connect() to " << event <<" gave " << uid;
  return uid;
}

bool RemoteObject::disconnect(unsigned int linkId)
{
  unsigned int event = linkId >> 16;
  unsigned int link = linkId & 0xFFFF;

  ObjectPrivate::SignalSubscriberMap::iterator i = _p->_subscribers.find(event);
  if (i == _p->_subscribers.end())
  {
    qiLogWarning("qi.object") << "Disconnect on non instanciated signal";
    return false;
  }

  if (Object::disconnect(linkId))
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
    if (!_ts->send(msg)) {
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

}
