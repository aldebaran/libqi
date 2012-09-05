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

  static qi::MetaObject *serverMetaObject() {
    qi::MetaObject*        mo = new qi::MetaObject();
    qi::MetaObjectBuilder  mob(mo);

    std::string ret;
    std::string sig;

    mob.xAdvertiseMethod("", "__metaObject::()", 0);

    ret = qi::signatureFromType<unsigned int>::value();
    sig = "registerEvent::(";
    qi::signatureFromType<unsigned int>::value(sig);
    qi::signatureFromType<unsigned int>::value(sig);
    qi::signatureFromType<unsigned int>::value(sig);
    sig += ")";
    mob.xAdvertiseMethod(ret, sig, 0);

    ret = qi::signatureFromType<bool>::value();
    sig = "unregisterEvent::(";
    qi::signatureFromType<unsigned int>::value(sig);
    qi::signatureFromType<unsigned int>::value(sig);
    qi::signatureFromType<unsigned int>::value(sig);
    sig += ")";
    mob.xAdvertiseMethod(ret, sig, 0);

    ret = qi::signatureFromType<qi::MetaObject>::value();
    sig = "metaObject::(";
    qi::signatureFromType<unsigned int>::value(sig);
    qi::signatureFromType<unsigned int>::value(sig);
    sig += ")";
    mob.xAdvertiseMethod(ret, sig, 0);


    MetaObject::MethodMap mm = mo->methods();
    MetaObject::MethodMap::iterator it;
    for (it = mm.begin(); it != mm.end(); ++it) {
      std::cout << "id: " << it->first << " : " << it->second.signature() << std::endl;
    }

    assert(mo->methodId("registerEvent::(III)") == qi::Message::ServerFunction_RegisterEvent);
    assert(mo->methodId("unregisterEvent::(III)") == qi::Message::ServerFunction_UnregisterEvent);
    assert(mo->methodId("metaObject::(II)") == qi::Message::ServerFunction_MetaObject);
    return mo;
  }

  ServerClient::ServerClient(qi::TransportSocket *socket)
    : _object(socket, qi::Message::Service_Server, serverMetaObject())
  {
  }

  ServerClient::~ServerClient()
  {
    _object._ts = 0;
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
