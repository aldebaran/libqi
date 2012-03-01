/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2012 Cedric GESTES
*/

#include "remoteobject_p.hpp"
#include <qimessaging/message.hpp>
#include <qimessaging/transport_socket.hpp>

namespace qi {

RemoteObject::RemoteObject(qi::TransportSocket *ts, unsigned int service, qi::MetaObject *mo)
  : _ts(ts)
  , _service(service)
{
  _meta = mo;
}

RemoteObject::~RemoteObject()
{
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

}
