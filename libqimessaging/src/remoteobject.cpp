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
  delete _meta;
}

void RemoteObject::onSocketReadyRead(TransportSocket *client, int id)
{
  qi::FunctorResultPromiseBase                            *promise = 0;
  qi::Message                                              msg(qi::Message::Create_WithoutBuffer);;
  std::map<int, qi::FunctorResultPromiseBase *>::iterator  it;

  client->read(id, &msg);

  {
    boost::mutex::scoped_lock lock(_mutex);
    it = _promises.find(id);
    if (it != _promises.end()) {
      promise = _promises[id];
      _promises.erase(it);
    }
  }

  if (promise) {
    qi::FunctorResult ret(msg.buffer());
    promise->setValue(ret);
    delete promise;
  } else {
    qiLogError("remoteobject") << "no promise found for req id:" << id;
    return;
  }
}

void RemoteObject::metaCall(unsigned int method, const std::string &sig, qi::FunctorParameters &in, qi::FunctorResultPromiseBase *out)
{
  qi::Message msg(in.buffer());
  qi::Message ret(qi::Message::Create_WithoutBuffer);;
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
    //TODO
    //out.setError(1);
    return;
  }
}


}
