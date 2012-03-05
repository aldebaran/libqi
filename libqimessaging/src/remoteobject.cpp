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
  delete _meta;
}

void RemoteObject::metaCall(unsigned int method, const std::string &sig, FunctorParameters &in, qi::FunctorResult &out)
{
  qi::Message msg(static_cast<Buffer *>(in.datastream().ioDevice()));
  qi::Message ret;
  msg.setType(qi::Message::Type_Call);
  msg.setService(_service);
  msg.setPath(qi::Message::Path_Main);
  //todo handle failure
  msg.setFunction(method);

  if (!_ts->send(msg)) {
    out.setError(1);
    return;
  }
  if (!_ts->waitForId(msg.id())) {
    out.setError(1);
    return;
  }
  if (!_ts->read(msg.id(), &ret)) {
    out.setError(1);
    return;
  }
  out.datastream().setIODevice(ret.buffer());
}

void RemoteObject::onSocketReadyRead(TransportSocket *client, int id)
{
  qi::FunctorResultPromiseBase                            *promise;
  qi::Message                                              msg;
  std::map<int, qi::FunctorResultPromiseBase *>::iterator  it;

  client->read(id, &msg);
  it = _promises.find(id);
  if (it != _promises.end()) {
    promise = _promises[id];
    qi::FunctorResult ret(msg.buffer());
    promise->setValue(ret);
    _promises.erase(it);
    delete promise;
  } else {
    qiLogError("remoteobject") << "no promise found for req id:" << id;
  }
}

void RemoteObject::metaCall(unsigned int method, const std::string &sig, qi::FunctorParameters &in, qi::FunctorResultPromiseBase *out)
{
  qi::Message msg(static_cast<Buffer *>(in.datastream().ioDevice()));
  qi::Message ret;
  msg.setType(qi::Message::Type_Call);
  msg.setService(_service);
  msg.setPath(qi::Message::Path_Main);
  //todo handle failure
  msg.setFunction(method);

  if (!_ts->send(msg)) {
    //TODO
    //out.setError(1);
    return;
  }
  //allocated from caller, owned by us then. (clean up by onReadyRead)
  _promises[msg.id()] = out;
}


}
