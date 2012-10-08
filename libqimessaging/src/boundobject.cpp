/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#include <qimessaging/objecttypebuilder.hpp>
#include "boundobject.hpp"
#include "serverresult.hpp"
#include "metaobject_p.hpp"

namespace qi {

  static GenericValue forwardEvent(const GenericFunction& params,
                                   unsigned int service, unsigned int event, TransportSocketPtr client)
  {
    qi::Message msg;
    msg.setBuffer(params.toBuffer());
    msg.setService(service);
    msg.setFunction(event);
    msg.setType(Message::Type_Event);
    msg.setObject(Message::GenericObject_Main);
    client->send(msg);
    return GenericValue();
  }


  ServiceBoundObject::ServiceBoundObject(unsigned int serviceId, qi::ObjectPtr object, qi::MetaCallType mct)
    : _links()
    , _serviceId(serviceId)
    , _object(object)
    , _callType(mct)
  {
    _self = createServiceBoundObjectType(this);
  }

  ServiceBoundObject::~ServiceBoundObject()
  {
  }

  static qi::MetaObject serviceSpecialMetaObject(qi::ObjectPtr self) {
    qi::MetaObject mob;

    unsigned int regId = mob._p->addMethod("I", "registerEvent::(III)");
    unsigned int unrId = mob._p->addMethod("v", "unregisterEvent::(III)");
    unsigned int metId = mob._p->addMethod("{IssI}{IsI}I", "metaObject::(I)");

    //X here a transportsocket
    unsigned int regId2 = self->metaObject().methodId("registerEvent::(IIIX)");
    unsigned int unrId2 = self->metaObject().methodId("unregisterEvent::(IIIX)");
    unsigned int metId2 = self->metaObject().methodId("metaObject::(IX)");
    assert(regId == regId2);
    assert(unrId == unrId2);
    assert(metId == metId2);

    int id = self->metaObject().methodId("metaObject::(IX)");
    assert(self->metaObject().method(id)->sigreturn() == mob.method(id)->sigreturn());

    //assert(mob.methodId())
    return mob;
  }

  qi::ObjectPtr ServiceBoundObject::createServiceBoundObjectType(ServiceBoundObject *self) {
    qi::ObjectTypeBuilder<ServiceBoundObject> ob;

    ob.advertiseMethod("registerEvent"  , &ServiceBoundObject::registerEvent);
    ob.advertiseMethod("unregisterEvent", &ServiceBoundObject::unregisterEvent);
    ob.advertiseMethod("metaObject"     , &ServiceBoundObject::metaObject);
    return ob.object(self);
  }

  //Bound Method
  unsigned int ServiceBoundObject::registerEvent(unsigned int objectId, unsigned int eventId, unsigned int remoteLinkId, qi::TransportSocketPtr socket) {
    //throw on error
    GenericFunction mc = makeDynamicGenericFunction(boost::bind(&forwardEvent, _1, _serviceId, eventId, socket));
    unsigned int linkId = _object->connect(eventId, mc);

    _links[socket][remoteLinkId] = RemoteLink(linkId, eventId);
    return linkId;
  }

  //Bound Method
  void ServiceBoundObject::unregisterEvent(unsigned int objectId, unsigned int eventId, unsigned int remoteLinkId, qi::TransportSocketPtr socket) {
    //throw on error
    ServiceLinks&          sl = _links[socket];
    ServiceLinks::iterator it = sl.find(remoteLinkId);

    if (it == sl.end())
    {
      std::stringstream ss;
      ss << "Unregister request failed for " << remoteLinkId <<" " << objectId;
      qiLogError("qi::Server") << ss.str();
      throw std::runtime_error(ss.str());
    }
    _object->disconnect(it->second.localLinkId);
  }

  static void metaObjectConcat(qi::MetaObject *dest, const qi::MetaObject &source) {
    //assert dest->maxid < 10
    //TODO
    //if (*(dest->_p->uid) > 10)
    //  qiLogError("BoundObject") << "Special metaObject too fat";

    if (!dest->_p->addMethods(10, source.methodMap()))
      qiLogError("BoundObject") << "cant merge metaobject (methods)";
    if (!dest->_p->addSignals(10, source.signalMap()))
      qiLogError("BoundObject") << "cant merge metaobject (signals)";
    //concat source to dest
  }

  //Bound Method
  qi::MetaObject ServiceBoundObject::metaObject(unsigned int objectId, qi::TransportSocketPtr socket) {
    qi::MetaObject mo = serviceSpecialMetaObject(_self);
    //we inject specials methods here
    metaObjectConcat(&mo, _object->metaObject());
    return mo;
  }

  void ServiceBoundObject::onMessage(const qi::Message &msg, TransportSocketPtr socket) {
    qi::ObjectPtr obj;
    unsigned int  funcId;
    bool          isSpecialCall;
    //choose between special function (on BoundObject) or normal calls
    if (msg.function() < 10) {
      obj = _self;
      isSpecialCall = true;
      funcId = msg.function();
    } else {
      obj = _object;
      funcId = msg.function() - 10;
      isSpecialCall = false;
    }

    GenericFunctionParameters mfp(msg.buffer());

    //socket object always take the TransportSocketPtr as last parameter, inject it!
    if (isSpecialCall && (msg.type() == qi::Message::Type_Call || msg.type() == Message::Type_Event)) {
      const qi::MetaMethod *mm = obj->metaObject().method(funcId);
      if (mm) {
        std::vector<std::string> v = qi::signatureSplit(mm->signature());
        Signature                s(Signature(v[2]).begin().children());
        Signature::iterator      it = s.begin();
        unsigned                 i;
        std::stringstream        sig;
        //discard the last parameter, it's the TransportSocketPtr (which we dont have atm)
        for (i = 0; i < s.size() - 1; ++i) {
          sig << it.signature();
          ++it;
        }
        mfp.setSignature(sig.str());
        mfp.convertToValues();

        //Create a new value, because MFP will destroy all object on shutdown
        //qi::TransportSocketPtr *psocket = new qi::TransportSocketPtr(socket);
        //TODO: should pushBack clone object itself, when invalidOnDestroy is set?
        mfp.pushBack(socket);
      }
    }
    switch (msg.type())
    {
    case Message::Type_Call: {
         qi::Future<MetaFunctionResult> fut = obj->metaCall(funcId, mfp, _callType);
         fut.connect(boost::bind<void>(&serverResultAdapter, _1, socket, msg.address()));
      }
      break;
    case Message::Type_Event: {
        obj->metaEmit(funcId, mfp);
      }
      break;
    default:
        qiLogError("qi.Server") << "unknown request of type " << (int)msg.type() << " on service: " << msg.address();
    }
  }

  void ServiceBoundObject::onSocketDisconnected(TransportSocketPtr client, int error)
  {
    // Disconnect event links set for this client.
    BySocketServiceLinks::iterator it = _links.find(client);
    if (it != _links.end())
    {
      for (ServiceLinks::iterator jt = it->second.begin(); jt != it->second.end(); ++jt)
        _object->disconnect(jt->second.localLinkId);
    }
    _links.erase(it);
  }

  qi::BoundObjectPtr makeServiceBoundObjectPtr(unsigned int serviceId, qi::ObjectPtr object) {
    boost::shared_ptr<ServiceBoundObject> ret(new ServiceBoundObject(serviceId, object));
    return ret;
  }


}
