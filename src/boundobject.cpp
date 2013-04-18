/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#include <boost/make_shared.hpp>

#include <qitype/genericobject.hpp>
#include "boundobject.hpp"
#include "serverresult.hpp"

qiLogCategory("qimessaging.boundobject");

static const int gObjectOffset = 10;

namespace qi {

  static GenericValuePtr forwardEvent(const GenericFunctionParameters& params,
                                   unsigned int service, unsigned int object,
                                   unsigned int event, TransportSocketPtr client,
                                   ObjectHost* context)
  {
    qi::Message msg;
    msg.setValues(params, context);
    msg.setService(service);
    msg.setFunction(event);
    msg.setType(Message::Type_Event);
    msg.setObject(object);
    client->send(msg);
    return GenericValuePtr();
  }


  ServiceBoundObject::ServiceBoundObject(unsigned int serviceId, unsigned int objectId,
                                         qi::ObjectPtr object,
                                         qi::MetaCallType mct,
                                         bool bindTerminate,
                                         ObjectHost* owner)
    : ObjectHost(serviceId)
    , _links()
    , _serviceId(serviceId)
    , _objectId(objectId)
    , _object(object)
    , _callType(mct)
    , _owner(owner)
  {
    onDestroy.setCallType(MetaCallType_Direct);
    _self = createServiceBoundObjectType(this, bindTerminate);
  }

  ServiceBoundObject::~ServiceBoundObject()
  {
    ObjectHost::clear();
    if (_owner)
      _owner->removeObject(_objectId);
    onDestroy(this);
  }

  qi::ObjectPtr ServiceBoundObject::createServiceBoundObjectType(ServiceBoundObject *self, bool bindTerminate) {
    static qi::ObjectTypeBuilder<ServiceBoundObject>* ob = 0;
    if (!ob)
    {
      ob = new qi::ObjectTypeBuilder<ServiceBoundObject>();
      ob->advertiseMethod("registerEvent"  , &ServiceBoundObject::registerEvent, MetaCallType_Auto, qi::Message::BoundObjectFunction_RegisterEvent);
      ob->advertiseMethod("unregisterEvent", &ServiceBoundObject::unregisterEvent, MetaCallType_Auto, qi::Message::BoundObjectFunction_UnregisterEvent);
      ob->advertiseMethod("metaObject"     , &ServiceBoundObject::metaObject, MetaCallType_Auto, qi::Message::BoundObjectFunction_MetaObject);
      ob->advertiseMethod("terminate",       &ServiceBoundObject::terminate, MetaCallType_Auto, qi::Message::BoundObjectFunction_Terminate);
      ob->advertiseMethod("getProperty",       &ServiceBoundObject::getProperty, MetaCallType_Auto, qi::Message::BoundObjectFunction_GetProperty);
      ob->advertiseMethod("setProperty",       &ServiceBoundObject::setProperty, MetaCallType_Auto, qi::Message::BoundObjectFunction_SetProperty);
      ob->advertiseMethod("properties",       &ServiceBoundObject::properties, MetaCallType_Auto, qi::Message::BoundObjectFunction_Properties);
      //global currentSocket: we are not multithread or async capable ob->setThreadingModel(ObjectThreadingModel_MultiThread);
    }
    ObjectPtr result = ob->object(self);
    return result;
  }

  //Bound Method
  Link ServiceBoundObject::registerEvent(unsigned int objectId, unsigned int eventId, Link remoteLinkId) {
    GenericFunction mc = makeDynamicGenericFunction(boost::bind(&forwardEvent, _1, _serviceId, _objectId, eventId, _currentSocket, this));
    Link linkId = _object->connect(eventId, mc);
    qiLogDebug() << "SBO rl " << remoteLinkId <<" ll " << linkId;
    _links[_currentSocket][remoteLinkId] = RemoteLink(linkId, eventId);
    return linkId;
  }

  //Bound Method
  void ServiceBoundObject::unregisterEvent(unsigned int objectId, unsigned int QI_UNUSED(event), Link remoteLinkId) {
    ServiceLinks&          sl = _links[_currentSocket];
    ServiceLinks::iterator it = sl.find(remoteLinkId);

    if (it == sl.end())
    {
      std::stringstream ss;
      ss << "Unregister request failed for " << remoteLinkId <<" " << objectId;
      qiLogError() << ss.str();
      throw std::runtime_error(ss.str());
    }
    _object->disconnect(it->second.localLinkId);
  }


  //Bound Method
  qi::MetaObject ServiceBoundObject::metaObject(unsigned int objectId) {
    //we inject specials methods here
    return qi::MetaObject::merge(_self->metaObject(), _object->metaObject());
  }

  void ServiceBoundObject::terminate(unsigned int)
  {
    qiLogDebug() << "terminate() received";
    if (_owner)
      _owner->removeObject(_objectId);
    else
      qiLogWarning() << "terminate() received on object without owner";
  }

  void ServiceBoundObject::onMessage(const qi::Message &msg, TransportSocketPtr socket) {
    if (msg.object() > _objectId)
    {
      qiLogDebug() << "onChildMessage " << msg.address();
      ObjectHost::onMessage(msg, socket);
      return;
    }

    qiLogDebug() << "onMessage " << msg.address();
    qi::ObjectPtr    obj;
    unsigned int     funcId;
    //choose between special function (on BoundObject) or normal calls
    if (msg.function() < static_cast<unsigned int>(gObjectOffset)) {
      obj = _self;
    } else {
      obj = _object;
    }
    funcId = msg.function();

    std::string sigparam;
    GenericFunctionParameters mfp;

    if (msg.type() == qi::Message::Type_Call) {
      const qi::MetaMethod *mm = obj->metaObject().method(funcId);
      if (!mm) {
        std::stringstream ss;
        ss << "No such method " << msg.address();
        qiLogError() << ss.str();
        qi::Promise<GenericValuePtr> prom;
        prom.setError(ss.str());
        serverResultAdapter(prom.future(), this, socket, msg.address());
        return;
      }
      sigparam = mm->parametersSignature();
    }

    else if (msg.type() == qi::Message::Type_Post) {
      const qi::MetaSignal *ms = obj->metaObject().signal(funcId);
      if (ms)
        sigparam = signatureSplit(ms->signature())[2];
      else {
        const qi::MetaMethod *mm = obj->metaObject().method(funcId);
        if (mm)
          sigparam = mm->parametersSignature();
        else {
          qiLogError() << "No such signal/method on event message " << msg.address();
          return;
        }
      }
    }
    else
    {
      qiLogError() << "Unexpected message type " << msg.type();
      return;
    }

    GenericValuePtr value = msg.value(sigparam, socket);
    mfp = value.asTupleValuePtr();
    /* Because of 'global' _currentSocket, we cannot support parallel
    * executions at this point.
    * Both on self, and on obj which can use currentSocket() too.
    *
    * So put a lock, and rely on metaCall we invoke being asynchronous for// execution
    * This is decided by _callType, set from BoundObject ctor argument, passed by Server, which
    * uses its internal _defaultCallType, passed to its constructor, default
    * to queued. When Server is instanciated by ObjectHost, it uses the default
    * value.
    *
    * As a consequence, users of currentSocket() must set _callType to Direct.
    */
    switch (msg.type())
    {
    case Message::Type_Call: {
        boost::mutex::scoped_lock lock(_mutex);
        _currentSocket = socket;
        qi::Future<GenericValuePtr>  fut = obj->metaCall(funcId, mfp,
            obj==_self ? MetaCallType_Direct: _callType);
        _currentSocket.reset();
        fut.connect(boost::bind<void>(&serverResultAdapter, _1, (ObjectHost*)this, socket, msg.address()));
      }
      break;
    case Message::Type_Post: {
        obj->metaPost(funcId, mfp);
      }
      break;
    default:
        qiLogError() << "unknown request of type " << (int)msg.type() << " on service: " << msg.address();
    }
    //########################
    value.destroy();
  }

  void ServiceBoundObject::onSocketDisconnected(TransportSocketPtr client, int error)
  {
    // Disconnect event links set for this client.
    BySocketServiceLinks::iterator it = _links.find(client);
    if (it != _links.end())
    {
      for (ServiceLinks::iterator jt = it->second.begin(); jt != it->second.end(); ++jt)
      {
        try
        {
          _object->disconnect(jt->second.localLinkId);
        }
        catch (const std::runtime_error& e)
        {
          qiLogError() << e.what();
        }
      }
      _links.erase(it);
    }
  }

  qi::BoundObjectPtr makeServiceBoundObjectPtr(unsigned int serviceId, qi::ObjectPtr object, qi::MetaCallType mct) {
    boost::shared_ptr<ServiceBoundObject> ret = boost::make_shared<ServiceBoundObject>(serviceId, Message::GenericObject_Main, object, mct);
    return ret;
  }

  GenericValue ServiceBoundObject::getProperty(const GenericValue& prop)
  {
    if (prop.kind() == Type::String)
      return _object->getProperty<GenericValue>(prop.toString());
    else if (prop.kind() == Type::Int)
      return _object->type->getProperty(_object->value, prop.toUInt());
    else
      throw std::runtime_error("Expected int or string for property index");
  }

  void ServiceBoundObject::setProperty(const GenericValue& prop, GenericValue val)
  {
    qi::Future<void> result;
    if (prop.kind() == Type::String)
      result = _object->setProperty(prop.toString(), val);
    else if (prop.kind() == Type::Int)
      result = _object->type->setProperty(_object->value, prop.toUInt(), val);
    else
      throw std::runtime_error("Expected int or string for property index");
    if (!result.isFinished())
      qiLogWarning() << "Assertion failed, setProperty() call not finished";
    // Throw the future error
    result.value();
  }

  std::vector<std::string> ServiceBoundObject::properties()
  {
    // FIXME implement
    std::vector<std::string> res;
    const MetaObject& mo = _object->metaObject();
    MetaObject::PropertyMap map = mo.propertyMap();
    for (MetaObject::PropertyMap::iterator it = map.begin(); it != map.end(); ++it)
      res.push_back(it->second.name());
    return res;
  }
}
