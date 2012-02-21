/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2012 Cedric GESTES
*/

#include "remoteobject_p.hpp"
#include <qimessaging/message.hpp>
#include <qimessaging/transport.hpp>

namespace qi {

RemoteObject::RemoteObject(qi::TransportSocket *ts, unsigned int service)
  : _ts(ts),
    _service(service)
{
}

void RemoteObject::metaCall(const std::string &method, const std::string &sig, DataStream &in, DataStream &out)
{
  qi::Message msg(in._buffer);
  msg.setType(qi::Message::Call);
  msg.setService(_service);
  msg.setPath(0);
  msg.setFunction(1);

  _ts->send(msg);
  _ts->waitForId(msg.id());

  qi::Message ret(out._buffer);
  _ts->read(msg.id(), &ret);
}

}
