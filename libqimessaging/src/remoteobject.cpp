/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2012 Cedric GESTES
*/

#include "remoteobject_p.hpp"
#include <qimessaging/message.hpp>
#include <qimessaging/transport_socket.hpp>
#include <qi/log.hpp>
#include <boost/thread/mutex.hpp>

namespace qi {

RemoteObject::RemoteObject(qi::TransportSocket *ts, unsigned int service, qi::MetaObject *mo)
  : _ts(ts)
  , _service(service)
{
  _ts->setCallbacks(this);
  //allocate by caller, but owned by us then
  _meta = mo;
}

RemoteObject::~RemoteObject()
{
  delete _ts;
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

  if (!promise.isValid()) {
    qiLogError("remoteobject") << "no promise found for req id:" << id;
    return;
  }

  switch (msg.type()) {
    case qi::Message::Type_Reply:
      promise.setValue(msg.buffer());
      return;
    case qi::Message::Type_Error:
      promise.setError(msg.buffer());
      return;
    default:
      qiLogError("remoteobject") << "Message (#" << id << ") type not handled: " << msg.type();
      return;
  }

}

void RemoteObject::metaCall(int method, const FunctorParameters &in, FunctorResult out)
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
    //TODO
    //out.setError(1);
    return;
  }
}


}
