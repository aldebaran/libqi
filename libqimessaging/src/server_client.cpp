/*
**  Author(s):
**  - Cedric Gestes <gestes@aldebaran-robotics.com>
**
**  Copyright (C) 2012 Aldebaran Robotics
*/

#include "server_client.hpp"
#include <qimessaging/message.hpp>
#include <qimessaging/signature.hpp>

namespace qi {

  static qi::MetaObject serverMetaObject() {
    qi::ObjectBuilder  ob;

    std::string ret;
    std::string sig;

    ret = qi::signatureFromType<unsigned int>::value();
    sig = "registerEvent::(";
    qi::signatureFromType<unsigned int>::value(sig);
    qi::signatureFromType<unsigned int>::value(sig);
    qi::signatureFromType<unsigned int>::value(sig);
    sig += ")";
    ob.xAdvertiseMethod(ret, sig, 0);

    ret = qi::signatureFromType<bool>::value();
    sig = "unregisterEvent::(";
    qi::signatureFromType<unsigned int>::value(sig);
    qi::signatureFromType<unsigned int>::value(sig);
    qi::signatureFromType<unsigned int>::value(sig);
    sig += ")";
    ob.xAdvertiseMethod(ret, sig, 0);

    ret = qi::signatureFromType<qi::MetaObject>::value();
    sig = "metaObject::(";
    qi::signatureFromType<unsigned int>::value(sig);
    qi::signatureFromType<unsigned int>::value(sig);
    sig += ")";
    ob.xAdvertiseMethod(ret, sig, 0);

    qi::MetaObject m = ob.object().metaObject();
    assert(m.methodId("registerEvent::(III)") == qi::Message::ServerFunction_RegisterEvent);
    assert(m.methodId("unregisterEvent::(III)") == qi::Message::ServerFunction_UnregisterEvent);
    assert(m.methodId("metaObject::(II)") == qi::Message::ServerFunction_MetaObject);
    return m;
  }

  ServerClient::ServerClient(qi::TransportSocket *socket)
    : _object(socket, qi::Message::Service_Server, serverMetaObject())
  {
  }

  ServerClient::~ServerClient()
  {
    boost::shared_ptr<qi::RemoteObjectPrivate> rop;
    rop = boost::dynamic_pointer_cast<qi::RemoteObjectPrivate>(_object._p);
    //do not delete _socket it is deleted by _object at the moment.
    rop->_ts = 0;
  }

  qi::Future<unsigned int> ServerClient::registerEvent(unsigned int serviceId, unsigned eventId, unsigned int linkId) {
    return _object.call<unsigned int>("registerEvent", serviceId, eventId, linkId);
  }

  qi::Future<bool> ServerClient::unregisterEvent(unsigned int serviceId, unsigned eventId, unsigned linkId) {
    return _object.call<bool>("unregisterEvent", serviceId, eventId, linkId);
  }

  qi::Future<qi::MetaObject> ServerClient::metaObject(unsigned int serviceId, unsigned int objectId) {
    return _object.call<qi::MetaObject>("metaObject", serviceId, objectId);
  };

}
