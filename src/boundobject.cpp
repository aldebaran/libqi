/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#include <boost/make_shared.hpp>

#include <qitype/anyobject.hpp>
#include <qitype/objecttypebuilder.hpp>
#include "boundobject.hpp"
#include "serverresult.hpp"

qiLogCategory("qimessaging.boundobject");

namespace qi {

  static AnyReference forwardEvent(const GenericFunctionParameters& params,
                                   unsigned int service, unsigned int object,
                                   unsigned int event, Signature sig,
                                   TransportSocketPtr client,
                                   ObjectHost* context)
  {
    qi::Message msg;
    msg.setValues(params, sig, context);
    msg.setService(service);
    msg.setFunction(event);
    msg.setType(Message::Type_Event);
    msg.setObject(object);
    client->send(msg);
    return AnyReference();
  }


  ServiceBoundObject::ServiceBoundObject(unsigned int serviceId, unsigned int objectId,
                                         qi::AnyObject object,
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
    qiLogDebug() << "~ServiceBoundObject()";
    ObjectHost::clear();
    if (_owner)
      _owner->removeObject(_objectId);
    onDestroy(this);
    qiLogDebug() << "~ServiceBoundObject() reseting object " << _object.use_count();
    _object.reset();
    qiLogDebug() << "~ServiceBoundObject() finishing";
  }

  qi::AnyObject ServiceBoundObject::createServiceBoundObjectType(ServiceBoundObject *self, bool bindTerminate) {
    static qi::ObjectTypeBuilder<ServiceBoundObject>* ob = 0;

#ifdef _WIN32
    boost::mutex::scoped_lock lock(detail::initializationMutex());
#else
    static boost::mutex mutex;
    boost::mutex::scoped_lock lock(mutex);
#endif

    if (!ob)
    {
      ob = new qi::ObjectTypeBuilder<ServiceBoundObject>();
      /* Network-related stuff.
      */
      ob->advertiseMethod("registerEvent"  , &ServiceBoundObject::registerEvent, MetaCallType_Auto, qi::Message::BoundObjectFunction_RegisterEvent);
      ob->advertiseMethod("unregisterEvent", &ServiceBoundObject::unregisterEvent, MetaCallType_Auto, qi::Message::BoundObjectFunction_UnregisterEvent);
      ob->advertiseMethod("terminate",       &ServiceBoundObject::terminate, MetaCallType_Auto, qi::Message::BoundObjectFunction_Terminate);
      /* GenericObject-related stuff.
      * Those methods could be advertised and implemented by GenericObject itself.
      * But since we already have a wrapper system in place in BoundObject, us it.
      * There is no use-case that requires the methods below without a BoundObject present.
      */
      ob->advertiseMethod("metaObject"     , &ServiceBoundObject::metaObject, MetaCallType_Auto, qi::Message::BoundObjectFunction_MetaObject);
      ob->advertiseMethod("property",       &ServiceBoundObject::property, MetaCallType_Auto, qi::Message::BoundObjectFunction_GetProperty);
      ob->advertiseMethod("setProperty",       &ServiceBoundObject::setProperty, MetaCallType_Auto, qi::Message::BoundObjectFunction_SetProperty);
      ob->advertiseMethod("properties",       &ServiceBoundObject::properties, MetaCallType_Auto, qi::Message::BoundObjectFunction_Properties);
      //global currentSocket: we are not multithread or async capable ob->setThreadingModel(ObjectThreadingModel_MultiThread);
    }
    AnyObject result = ob->object(self, &AnyObject::deleteGenericObjectOnly);
    return result;
  }

  //Bound Method
  SignalLink ServiceBoundObject::registerEvent(unsigned int objectId, unsigned int eventId, SignalLink remoteSignalLinkId) {
    // fetch signature
    const MetaSignal* ms = _object.metaObject().signal(eventId);
    if (!ms)
      throw std::runtime_error("No such signal");
    AnyFunction mc = AnyFunction::fromDynamicFunction(boost::bind(&forwardEvent, _1, _serviceId, _objectId, eventId, ms->parametersSignature(), _currentSocket, this));
    SignalLink linkId = _object.connect(eventId, mc);
    qiLogDebug() << "SBO rl " << remoteSignalLinkId <<" ll " << linkId;
    _links[_currentSocket][remoteSignalLinkId] = RemoteSignalLink(linkId, eventId);
    return linkId;
  }

  //Bound Method
  void ServiceBoundObject::unregisterEvent(unsigned int objectId, unsigned int QI_UNUSED(event), SignalLink remoteSignalLinkId) {
    ServiceSignalLinks&          sl = _links[_currentSocket];
    ServiceSignalLinks::iterator it = sl.find(remoteSignalLinkId);

    if (it == sl.end())
    {
      std::stringstream ss;
      ss << "Unregister request failed for " << remoteSignalLinkId <<" " << objectId;
      qiLogError() << ss.str();
      throw std::runtime_error(ss.str());
    }
    _object.disconnect(it->second.localSignalLinkId);
    sl.erase(it);
    if (sl.empty())
      _links.erase(_currentSocket);
  }


  //Bound Method
  qi::MetaObject ServiceBoundObject::metaObject(unsigned int objectId) {
    //we inject specials methods here
    return qi::MetaObject::merge(_self.metaObject(), _object.metaObject());
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
    try {
      if (msg.version() > qi::Message::currentVersion())
      {
        std::stringstream ss;
        ss << "Cannot negotiate QiMessaging connection: "
           << "remote end doesn't support binary protocol v" << msg.version();
        serverResultAdapter(qi::makeFutureError<AnyReference>(ss.str()), Signature(),
                            _owner ? _owner : this, socket, msg.address());
        return;
      }

      qiLogDebug() << this << "(" << service() << '/' << _objectId << ") msg " << msg.address() << " " << msg.buffer().size();

      if (msg.object() > _objectId)
      {
        qiLogDebug() << "Passing message to children";
        ObjectHost::onMessage(msg, socket);
        return;
      }

      qi::AnyObject    obj;
      unsigned int     funcId;
      //choose between special function (on BoundObject) or normal calls
      // Manageable functions are at the end of reserver range but dispatch to _object
      if (msg.function() < Manageable::startId) {
        obj = _self;
      } else {
        obj = _object;
      }
      funcId = msg.function();

      qi::Signature sigparam;
      GenericFunctionParameters mfp;

      if (msg.type() == qi::Message::Type_Call) {
        const qi::MetaMethod *mm = obj.metaObject().method(funcId);
        if (!mm) {
          std::stringstream ss;
          ss << "No such method " << msg.address();
          qiLogError() << ss.str();
          throw std::runtime_error(ss.str());
        }
        sigparam = mm->parametersSignature();
      }

      else if (msg.type() == qi::Message::Type_Post) {
        const qi::MetaSignal *ms = obj.metaObject().signal(funcId);
        if (ms)
          sigparam = ms->parametersSignature();
        else {
          const qi::MetaMethod *mm = obj.metaObject().method(funcId);
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

      AnyReference value = msg.value(sigparam, socket);
      if (sigparam == "m")
      {
        // received dynamically typed argument pack, unwrap
        AnyValue* content = value.ptr<AnyValue>();
        // steal it
        AnyReference pContent(content->type, content->value);
        content->type = 0;
        content->value = 0;
        // free the object content
        value.destroy();
        value = pContent;
      }
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
        qi::Future<AnyReference>  fut = obj.metaCall(funcId, mfp,
                                                         obj==_self ? MetaCallType_Direct: _callType);
        Signature retSig;
        const MetaMethod* mm = obj.metaObject().method(funcId);
        if (mm)
          retSig = mm->returnSignature();
        _currentSocket.reset();
        fut.connect(boost::bind<void>(&serverResultAdapter, _1, retSig, _owner?_owner:(ObjectHost*)this, socket, msg.address()));
      }
        break;
      case Message::Type_Post: {
        if (obj == _self) // we need a sync call (see comment above), post does not provide it
          obj.metaCall(funcId, mfp, MetaCallType_Direct);
        else
          obj.metaPost(funcId, mfp);
      }
        break;
      default:
        qiLogError() << "unknown request of type " << (int)msg.type() << " on service: " << msg.address();
      }
      //########################
      value.destroy();
    } catch (const std::runtime_error &e) {
      if (msg.type() == Message::Type_Call) {
        qi::Promise<AnyReference> prom;
        prom.setError(e.what());
        serverResultAdapter(prom.future(), Signature(), _owner?_owner:this, socket, msg.address());
      }
    } catch (...) {
      if (msg.type() == Message::Type_Call) {
        qi::Promise<AnyReference> prom;
        prom.setError("Unknown error catch");
        serverResultAdapter(prom.future(), Signature(), _owner?_owner:this, socket, msg.address());
      }
    }
  }

  void ServiceBoundObject::onSocketDisconnected(TransportSocketPtr client, std::string error)
  {
    // Disconnect event links set for this client.
    BySocketServiceSignalLinks::iterator it = _links.find(client);
    if (it != _links.end())
    {
      for (ServiceSignalLinks::iterator jt = it->second.begin(); jt != it->second.end(); ++jt)
      {
        try
        {
          _object.disconnect(jt->second.localSignalLinkId);
        }
        catch (const std::runtime_error& e)
        {
          qiLogError() << e.what();
        }
      }
      _links.erase(it);
    }
  }

  qi::BoundAnyObject makeServiceBoundAnyObject(unsigned int serviceId, qi::AnyObject object, qi::MetaCallType mct) {
    boost::shared_ptr<ServiceBoundObject> ret = boost::make_shared<ServiceBoundObject>(serviceId, Message::GenericObject_Main, object, mct);
    return ret;
  }

  AnyValue ServiceBoundObject::property(const AnyValue& prop)
  {
    if (prop.kind() == TypeKind_String)
      return _object.property<AnyValue>(prop.toString());
    else if (prop.kind() == TypeKind_Int)
    { // missing accessor, go to bacend
      GenericObject* go = _object.asGenericObject();
      return go->type->property(go->value, prop.toUInt());
    }
    else
      throw std::runtime_error("Expected int or string for property index");
  }

  void ServiceBoundObject::setProperty(const AnyValue& prop, AnyValue val)
  {
    qi::Future<void> result;
    if (prop.kind() == TypeKind_String)
      result = _object.setProperty(prop.toString(), val);
    else if (prop.kind() == TypeKind_Int)
    {
      GenericObject* go = _object.asGenericObject();
      result = go->type->setProperty(go->value, prop.toUInt(), val);
    }
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
    const MetaObject& mo = _object.metaObject();
    MetaObject::PropertyMap map = mo.propertyMap();
    for (MetaObject::PropertyMap::iterator it = map.begin(); it != map.end(); ++it)
      res.push_back(it->second.name());
    return res;
  }
}
