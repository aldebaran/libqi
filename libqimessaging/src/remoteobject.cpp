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

void RemoteObject::metaCall(unsigned int method, const std::string &sig, DataStream &in, DataStream &out)
{
  qi::Message msg(static_cast<Buffer *>(in.ioDevice()));
  msg.setType(qi::Message::Type_Call);
  msg.setService(_service);
  msg.setPath(qi::Message::Path_Main);
  //todo handle failure
  msg.setFunction(method);

  _ts->send(msg);
  _ts->waitForId(msg.id());

  qi::Message ret;
  _ts->read(msg.id(), &ret);
  //TODO: ret(out.ioDevice())
  out.setIODevice(ret.buffer());
}

}
