/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2012 Cedric GESTES
*/

#include "remoteobject_p.hpp"
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
  //allocate by caller, but owned by us then
  _meta = mo;
}

RemoteObject::~RemoteObject()
{
  delete _ts;
}

void RemoteObject::onSocketTimeout(TransportSocket *client, int id)
{
  {
    boost::mutex::scoped_lock lock(_mutex);
    std::map<int, qi::FunctorResult>::iterator it = _promises.find(id);
    if (it != _promises.end())
    {
      it->second.setError("network timeout");
      _promises.erase(it);
    }
  }
}

void RemoteObject::onSocketReadyRead(TransportSocket *client, int id)
{
  qi::FunctorResult                          promise;
  qi::Message                                msg;
  std::map<int, qi::FunctorResult>::iterator it;

  client->read(id, &msg);

  {
    boost::mutex::scoped_lock lock(_mutex);
    it = _promises.find(id);
    if (it != _promises.end()) {
      promise = _promises[id];
      _promises.erase(it);
    }
  }

  switch (msg.type()) {
    case qi::Message::Type_Reply:
      if (!promise.isValid()) {
        qiLogError("remoteobject") << "no promise found for req id:" << id;
        return;
      }
      promise.setValue(msg.buffer());
      return;
    case qi::Message::Type_Error: {
      qi::IDataStream ds(msg.buffer());
      qi::Buffer     buf;
      std::string    sig;
      ds >> sig;
      ds >> buf;
      promise.setError(sig, buf);
      return;
    }
    case qi::Message::Type_Event:
      trigger(msg.function(), FunctorParameters(msg.buffer()));
      return;
    default:
      qiLogError("remoteobject") << "Message (#" << id << ") type not handled: " << msg.type();
      return;
  }

}

void RemoteObject::metaCall(unsigned int method, const FunctorParameters &in, FunctorResult out)
{
  qi::Message msg;
  msg.setBuffer(in.buffer());
  qi::Message ret;
  msg.setType(qi::Message::Type_Call);
  msg.setService(_service);
  msg.setPath(qi::Message::Path_Main);
  //todo handle failure
  msg.setFunction(method);

  //allocated from caller, owned by us then. (clean up by onReadyRead)

  {
    boost::mutex::scoped_lock lock(_mutex);
    _promises[msg.id()] = out;
  }
  if (!_ts->send(msg)) {
    qiLogError("remoteobject") << "error while sending answer";
    qi::MetaMethod *meth = metaObject().method(method);
    std::stringstream msg;
    if (meth) {
      msg << "Network error while sending data to method: '";
      msg << meth->signature();;
      msg << "'";
    } else {
      msg << "Network error while sending data an unknown method (id=" << method << ")";
    }
    out.setError(msg.str());
    //TODO
    //out.setError(1);
    return;
  }
}

void RemoteObject::metaEmit(unsigned int event, const FunctorParameters &args)
{
  // Bounce the emit request to server
  // TODO: one optimisation that could be done is to trigger the local
  // subscribers immediately.
  // But it is a bit complex, because the server will bounce the
  // event back to us.
  qiLogError("Not implemented yet lol");
}

unsigned int RemoteObject::connect(unsigned int event, const MetaEvent::Subscriber& sub)
{
  // Bind the function locally.
  unsigned int uid = Object::connect(event, sub);
  // Notify the Service that we are interested in its event.
  // Provide our uid as payload
  qi::Message msg;
  qi::Buffer buf;
  qi::ODataStream ds(buf);
  ds << uid;
  msg.setBuffer(buf);
  msg.setType(qi::Message::Type_Register_Event);
  msg.setService(_service);
  msg.setPath(qi::Message::Path_Main);
  msg.setFunction(event);
  if (!_ts->send(msg)) {
    qiLogError("remoteobject") << "error while registering event";
  }
  return uid;
}

bool RemoteObject::disconnect(unsigned int linkId)
{
  // Figure out which event this link is associated to
  std::vector<MetaEvent>::iterator i;
  unsigned int event = (unsigned int)-1;
  for (i = _meta->events().begin(); i!= _meta->events().end(); ++i)
  {
    MetaEventPrivate::Subscribers::iterator j = i->_p->_subscribers.find(linkId);
    if (j != i->_p->_subscribers.end())
    {
      event = i->index();
      break;
    }
  }
  if (event == (unsigned int)-1)
    return false;
  if (Object::disconnect(linkId))
  {
    // Tell the remote we are no longer interested.
    qi::Message msg;
    qi::Buffer buf;
    qi::ODataStream ds(buf);
    ds << linkId;
    msg.setBuffer(buf);
    msg.setType(qi::Message::Type_Unregister_Event);
    msg.setService(_service);
    msg.setPath(qi::Message::Path_Main);
    msg.setFunction(event);
    if (!_ts->send(msg)) {
      qiLogError("remoteobject") << "error while registering event";
    }
    return true;
  }
  else
    return false;
}

}
