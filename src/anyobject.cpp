/*
**  Copyright (C) 2012, 2013 Aldebaran Robotics
**  See COPYING for the license
*/
#include <iostream>

#include <qitype/anyobject.hpp>
#include <qitype/objecttypebuilder.hpp>
#include "staticobjecttype.hpp"
#include "anyobject_p.hpp"
#include "metaobject_p.hpp" // for generateErrorString

#ifdef _MSC_VER
#  pragma warning( push )
#  pragma warning( disable: 4355 )
#endif

qiLogCategory("qitype.object");

namespace qi {

  GenericObject::GenericObject(ObjectTypeInterface *type, void *value)
  : type(type)
  , value(value)
  {
  }

  GenericObject::~GenericObject() {
  }

  ManageablePrivate::ManageablePrivate()
  : objectMutex(new boost::recursive_timed_mutex)
  , statsEnabled(false)
  , traceEnabled(false)
  {
  }

  Manageable::Manageable()
  : traceObject(boost::bind(&Manageable::enableTrace, this, _1))
  {
    _p = new ManageablePrivate();
    _p->eventLoop = 0;
    _p->dying = false;
  }

  Manageable::Manageable(const Manageable& b)
  : traceObject(boost::bind(&Manageable::enableTrace, this, _1))
  {
    _p = new ManageablePrivate();
    _p->eventLoop = b._p->eventLoop;
    _p->dying = false;
  }

  void Manageable::operator = (const Manageable& b)
  {
    this->~Manageable();
    _p = new ManageablePrivate();
    _p->eventLoop = b._p->eventLoop;
    _p->dying = false;
  }

  Manageable::~Manageable()
  {
    _p->dying = true;
    std::vector<SignalSubscriber> copy;
    {
      boost::mutex::scoped_lock sl(_p->registrationsMutex);
      copy = _p->registrations;
    }
    for (unsigned i = 0; i < copy.size(); ++i)
    {
      copy[i].source->disconnect(copy[i].linkId);
    }
    delete _p;
  }

  void Manageable::forceEventLoop(EventLoop* el)
  {
    _p->eventLoop = el;
  }

  EventLoop* Manageable::eventLoop() const
  {
    return _p->eventLoop;
  }

  Manageable::TimedMutexPtr Manageable::mutex()
  {
    return _p->objectMutex;
  }

  bool Manageable::isStatsEnabled() const
  {
    return _p->statsEnabled;
  }

  void Manageable::enableStats(bool state)
  {
    _p->statsEnabled = state;
  }

  void Manageable::pushStats(int slotId, float wallTime, float userTime, float systemTime)
  {
    boost::mutex::scoped_lock(_p->registrationsMutex);
    MethodStatistics& ms = _p->stats[slotId];
    ms.push(wallTime, userTime, systemTime);
  }

  ObjectStatistics Manageable::stats() const
  {
    boost::mutex::scoped_lock(_p->registrationsMutex);
    return _p->stats;
  }

  void Manageable::clearStats()
  {
    boost::mutex::scoped_lock(_p->registrationsMutex);
    _p->stats.clear();
  }

  bool Manageable::isTraceEnabled() const
  {
    return _p->traceEnabled;
  }

  void Manageable::enableTrace(bool state)
  {
    _p->traceEnabled = state;
  }

  int Manageable::_nextTraceId()
  {
    return ++_p->traceId;
  }

  const MetaObject &GenericObject::metaObject() {
    if (!type || !value) {
      static qi::MetaObject fail;
      qiLogWarning() << "Operating on invalid GenericObject..";
      return fail;
    }
    return type->metaObject(value);
  }

  qi::Future<AnyReference>
  GenericObject::metaCall(unsigned int method, const GenericFunctionParameters& params, MetaCallType callType)
  {
    FutureCallbackType futureType = (callType == MetaCallType_Direct) ? FutureCallbackType_Sync: FutureCallbackType_Async;
    if (!type || !value) {
      const std::string s = "Operating on invalid GenericObject..";
      qiLogWarning() << s;
      return qi::makeFutureError<AnyReference>(s, futureType);
    }
    try {
      return type->metaCall(value, shared_from_this(), method, params, callType);
    } catch (const std::exception &e) {
      return qi::makeFutureError<AnyReference>(e.what(), futureType);
    }
    catch (...)
    {
      return qi::makeFutureError<AnyReference>("Unknown exception caught", futureType);
    }
  }

  void GenericObject::metaPost(unsigned int event, const GenericFunctionParameters& args)
  {
    if (!type || !value) {
      qiLogWarning() << "Operating on invalid GenericObject..";
      return;
    }
    type->metaPost(value, shared_from_this(), event, args);
  }

  unsigned int GenericObject::findMethod(const std::string& nameWithOptionalSignature, const GenericFunctionParameters& args)
  {
    return metaObject().findMethod(nameWithOptionalSignature, args);
  }

  qi::Future<AnyReference>
  GenericObject::metaCall(const std::string &nameWithOptionalSignature, const GenericFunctionParameters& args, MetaCallType callType)
  {
    if (!type || !value) {
      const std::string s = "Invalid object";
      qiLogError() << s;
      return qi::makeFutureError<AnyReference>(s);
    }
    int methodId = findMethod(nameWithOptionalSignature, args);
    if (methodId < 0) {
      std::string resolvedSig = args.signature(true).toString();
      return makeFutureError<AnyReference>(MetaObjectPrivate::generateErrorString(nameWithOptionalSignature, metaObject().findCompatibleMethod(nameWithOptionalSignature), false));
    }
    return metaCall(methodId, args, callType);
  }

  /// Resolve signature and bounce
  void GenericObject::metaPost(const std::string &nameWithOptionalSignature, const GenericFunctionParameters &in) {
    if (!value || !type) {
      qiLogWarning() << "Operating on invalid GenericObject..";
      return;
    }

    bool hasSig = (nameWithOptionalSignature.find(':') != std::string::npos);
    int eventId = metaObject().signalId(
      hasSig ? qi::signatureSplit(nameWithOptionalSignature)[1] : nameWithOptionalSignature);
    if (eventId < 0)
      eventId = findMethod(nameWithOptionalSignature, in);
    if (eventId < 0) {
      std::stringstream ss;
      std::string name = qi::signatureSplit(nameWithOptionalSignature)[1];
      ss << "Can't find method or signal: " << nameWithOptionalSignature << std::endl;
      ss << "  Method Candidate(s):" << std::endl;
      std::vector<MetaMethod>           mml2 = metaObject().findMethod(name);
      std::vector<MetaMethod>::const_iterator it2;
      for (it2 = mml2.begin(); it2 != mml2.end(); ++it2) {
        ss << "  " << it2->toString() << std::endl;
      }
      qiLogError() << ss.str();
      return;
    }
    metaPost(eventId, in);
  }

  //TODO: use functor.signature instead of nameWithSignature.
  /// Resolve signature and bounce
  qi::FutureSync<SignalLink> GenericObject::connect(const std::string &name, const SignalSubscriber& functor)
  {
    if (!type || !value) {
      qiLogWarning() << "Operating on invalid GenericObject..";
      return qi::makeFutureError<SignalLink>("Operating on invalid GenericObject..");
    }
    int eventId = metaObject().signalId(name);

    if (eventId < 0) {
      std::stringstream ss;
      ss << "Can't find signal: " << name;
      qiLogError() << ss.str();
      return qi::makeFutureError<SignalLink>(ss.str());
    }
    return connect(eventId, functor);
  }

  qi::FutureSync<SignalLink> GenericObject::connect(unsigned int event, const SignalSubscriber& sub)
  {
    if (!type || !value) {
      qiLogWarning() << "Operating on invalid GenericObject..";
      return qi::makeFutureError<SignalLink>("Operating on invalid GenericObject..");
    }
    return type->connect(value, shared_from_this(), event, sub);
  }

  qi::FutureSync<void> GenericObject::disconnect(SignalLink linkId)
  {
    if (!type || !value) {
      qiLogWarning() << "Operating on invalid GenericObject..";
      return qi::makeFutureError<void>("Operating on invalid GenericObject");
    }
    return type->disconnect(value, shared_from_this(), linkId);
  }

  qi::FutureSync<AnyValue> GenericObject::property(unsigned int id) {
    return type->property(value, id);
  }

  qi::FutureSync<void> GenericObject::setProperty(unsigned int id, const AnyValue& val) {
    return type->setProperty(value, id, val);
  }

  qi::FutureSync<SignalLink> GenericObject::connect(unsigned int signal, AnyObject target, unsigned int slot)
  {
    return connect(signal, SignalSubscriber(target, slot));
  }

  /*
  std::vector<SignalSubscriber> GenericObject::subscribers(int eventId) const
  {
    std::vector<SignalSubscriber> res;
    if (!_p) {
      qiLogWarning() << "Operating on invalid GenericObject..";
      return res;
    }
    return _p->subscribers(eventId);
  }*/

  void GenericObject::post(const std::string& nameWithOptionalSignature,
                         qi::AutoAnyReference p1,
                         qi::AutoAnyReference p2,
                         qi::AutoAnyReference p3,
                         qi::AutoAnyReference p4,
                         qi::AutoAnyReference p5,
                         qi::AutoAnyReference p6,
                         qi::AutoAnyReference p7,
                         qi::AutoAnyReference p8)
  {
    if (!type || !value) {
      qiLogWarning() << "Operating on invalid GenericObject..";
      return;
    }
    qi::AutoAnyReference* vals[8]= {&p1, &p2, &p3, &p4, &p5, &p6, &p7, &p8};
    std::vector<qi::AnyReference> params;
    for (unsigned i=0; i<8; ++i)
      if (vals[i]->type())
        params.push_back(*vals[i]);
    metaPost(nameWithOptionalSignature, GenericFunctionParameters(params));
  }

  int ObjectTypeInterface::inherits(TypeInterface* other)
  {
    /* A registered class C can have to TypeInterface* around:
    * - TypeImpl<C*>
    * - The staticObjectTypeInterface that was created by the builder.
    * So assume that any of them can be in the parentTypes list.
    */
    if (this == other)
      return 0;
    const std::vector<std::pair<TypeInterface*, int> >& parents = parentTypes();
    qiLogDebug() << infoString() <<" has " << parents.size() <<" parents";
    for (unsigned i=0; i<parents.size(); ++i)
    {
      if (parents[i].first->info() == other->info())
        return parents[i].second;
      ObjectTypeInterface* op = dynamic_cast<ObjectTypeInterface*>(parents[i].first);
      if (op)
      {
        int offset = op->inherits(other);
        if (offset != -1)
        {
          qiLogDebug() << "Inheritance offsets " << parents[i].second
           << " " << offset;
          return parents[i].second + offset;
        }
      }
      qiLogDebug() << parents[i].first->infoString() << " does not match " << other->infoString()
      <<" " << ((op != 0) == (dynamic_cast<ObjectTypeInterface*>(other) != 0));
    }
    return -1;
  }
  namespace manageable
  {
    static Manageable::MethodMap* methodMap = 0;
    static Manageable::SignalMap* signalMap = 0;
    static MetaObject* metaObject = 0;
  }
  void Manageable::_build()
  {
    if (manageable::methodMap)
      return;
    manageable::methodMap = new MethodMap();
    manageable::signalMap = new SignalMap();
    manageable::metaObject = new MetaObject();
    ObjectTypeBuilder<Manageable> builder;
    unsigned int id = startId;
    builder.advertiseMethod("isStatsEnabled", &Manageable::isStatsEnabled, MetaCallType_Auto, id++);
    builder.advertiseMethod("enableStats", &Manageable::enableStats,       MetaCallType_Auto, id++);
    builder.advertiseMethod("stats", &Manageable::stats,                   MetaCallType_Auto, id++);
    builder.advertiseMethod("clearStats", &Manageable::clearStats,         MetaCallType_Auto, id++);
    builder.advertiseMethod("isTraceEnabled", &Manageable::isTraceEnabled, MetaCallType_Auto, id++);
    builder.advertiseMethod("enableTrace", &Manageable::enableTrace,       MetaCallType_Auto, id++);
    builder.advertiseSignal("traceObject", &Manageable::traceObject, id++);
    assert(id <= endId);
    const ObjectTypeData& typeData = builder.typeData();
    *manageable::methodMap = typeData.methodMap;
    *manageable::signalMap = typeData.signalGetterMap;
    *manageable::metaObject = builder.metaObject();
  }

  Manageable::MethodMap& Manageable::manageableMmethodMap()
  {
    _build();
    return *manageable::methodMap;
  }

  Manageable::SignalMap& Manageable::manageableSignalMap()
  {
    _build();
    return *manageable::signalMap;
  }

  MetaObject& Manageable::manageableMetaObject()
  {
    _build();
    return *manageable::metaObject;
  }

  namespace detail
  {
    ProxyGeneratorMap& proxyGeneratorMap()
    {
      static ProxyGeneratorMap* map = 0;
      if (!map)
        map = new ProxyGeneratorMap();
      return *map;
    }
  }
}


#ifdef _MSC_VER
#  pragma warning( pop )
#endif

