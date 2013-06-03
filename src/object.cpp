/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/
#include <iostream>

#include <qitype/genericobject.hpp>
#include "staticobjecttype.hpp"
#include "object_p.hpp"

qiLogCategory("qitype.object");

namespace qi {

  GenericObject::GenericObject(ObjectType *type, void *value)
  : type(type)
  , value(value)
  {
  }

  GenericObject::~GenericObject() {
  }

  ManageablePrivate::ManageablePrivate()
  : objectMutex(new boost::timed_mutex)
  , statsEnabled(false)
  , traceEnabled(false)
  {
  }

  Manageable::Manageable()
  {
    _p = new ManageablePrivate();
    _p->eventLoop = 0;
    _p->dying = false;
  }

  Manageable::Manageable(const Manageable& b)
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

  void Manageable::pushStats(int slotId, float value)
  {
    boost::mutex::scoped_lock(_p->registrationsMutex);
    MethodStatistics& ms = _p->stats[slotId];
    ms.cumulatedTime += value;
    if (!ms.count)
    {
      ms.minTime = value;
      ms.maxTime = value;
    }
    else
    {
      ms.minTime = std::min(ms.minTime, value);
      ms.maxTime = std::max(ms.maxTime, value);
    }
    ++ms.count;
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

  qi::Future<GenericValuePtr>
  GenericObject::metaCall(unsigned int method, const GenericFunctionParameters& params, MetaCallType callType)
  {
    qi::Promise<GenericValuePtr> out;
    if (!type || !value) {
      qiLogWarning() << "Operating on invalid GenericObject..";
      out.setError("Invalid object");
      return out.future();
    }
    try {
      return type->metaCall(value, this, method, params, callType);
    } catch (const std::exception &e) {
      out.setError(e.what());
      return out.future();
    }
    catch (...)
    {
      out.setError("Unknown exception caught");
      return out.future();
    }
  }

  void GenericObject::metaPost(unsigned int event, const GenericFunctionParameters& args)
  {
    if (!type || !value) {
      qiLogWarning() << "Operating on invalid GenericObject..";
      return;
    }
    type->metaPost(value, this, event, args);
  }

  static std::string generateErrorString(const std::string &type, const std::string &signature,
    const std::vector<MetaObject::CompatibleMethod> &candidates, bool logError = true)
  {
    std::stringstream                           ss;
    std::vector<MetaObject::CompatibleMethod>::const_iterator it;

    ss << "Can't find " << type << ": " << signature << std::endl
       << "  Candidate(s):" << std::endl;
    for (it = candidates.begin(); it != candidates.end(); ++it) {
      const qi::MetaMethod       &mm = it->first;
      ss << "  " << mm.toString() << " (" << it->second << ')' << std::endl;
    }
    if (logError)
      qiLogError() << ss.str();
    return ss.str();
  }

  struct less_pair_second
  {
    template<typename T> bool operator()(const T& a, const T& b) const
    {
      return a.second < b.second;
    }
  };

  unsigned int GenericObject::findMethod(const std::string& nameWithOptionalSignature, const GenericFunctionParameters& args)
  {
    typedef std::vector<MetaObject::CompatibleMethod> Methods;
    const MetaObject& mo = metaObject();
    if (nameWithOptionalSignature.find(':') != nameWithOptionalSignature.npos)
      return mo.methodId(nameWithOptionalSignature);
    for (unsigned dyn = 0; dyn<2; ++dyn)
    {
      std::string resolvedSig = args.signature(dyn==1).toString();
      std::string fullSig = nameWithOptionalSignature + "::" + resolvedSig;
      qiLogDebug() << "Finding method for resolved signature " << fullSig;
      // First try an exact match, wich is much faster if we're lucky.
      int methodId = mo.methodId(fullSig);
      if (methodId >= 0)
        return methodId;
      Methods mml = mo.findCompatibleMethod(fullSig);
      if (mml.empty())
        continue;
      if (mml.size() == 1)
        return mml.front().first.uid();
      // get best match
      Methods::iterator it = std::max_element(mml.begin(), mml.end(), less_pair_second());
      int count = 0;
      for (unsigned i=0; i<mml.size(); ++i)
      {
        if (mml[i].second == it->second)
          ++count;
      }
      assert(count);
      if (count > 1)
        qiLogVerbose() << generateErrorString("ambiguous overload", nameWithOptionalSignature, mml, false);
      else
        return it->first.uid();
    }
    return -1;
  }

  qi::Future<GenericValuePtr>
  GenericObject::metaCall(const std::string &nameWithOptionalSignature, const GenericFunctionParameters& args, MetaCallType callType)
  {
    qi::Promise<GenericValuePtr> out;
    if (!type || !value) {
      qiLogWarning() << "Operating on invalid GenericObject..";
      out.setError("Invalid object");
      return out.future();
    }
    int methodId = findMethod(nameWithOptionalSignature, args);
    if (methodId < 0) {
      return makeFutureError<GenericValuePtr>(generateErrorString("method", nameWithOptionalSignature, metaObject().findCompatibleMethod(nameWithOptionalSignature)));
    }
    return metaCall(methodId, args, callType);
  }

  /// Resolve signature and bounce
  void GenericObject::metaPost(const std::string &nameWithOptionalSignature, const GenericFunctionParameters &in) {
    if (!value || !type) {
      qiLogWarning() << "Operating on invalid GenericObject..";
      return;
    }

    int eventId = metaObject().signalId(qi::signatureSplit(nameWithOptionalSignature)[1]);
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
  qi::FutureSync<Link> GenericObject::connect(const std::string &name, const SignalSubscriber& functor)
  {
    if (!type || !value) {
      qiLogWarning() << "Operating on invalid GenericObject..";
      return qi::makeFutureError<Link>("Operating on invalid GenericObject..");
    }
    int eventId = metaObject().signalId(name);

    if (eventId < 0) {
      std::stringstream ss;
      ss << "Can't find signal: " << name;
      qiLogError() << ss.str();
      return qi::makeFutureError<Link>(ss.str());
    }
    return connect(eventId, functor);
  }

  qi::FutureSync<Link> GenericObject::connect(unsigned int event, const SignalSubscriber& sub)
  {
    if (!type || !value) {
      qiLogWarning() << "Operating on invalid GenericObject..";
      return qi::makeFutureError<Link>("Operating on invalid GenericObject..");
    }
    return type->connect(value, this, event, sub);
  }

  qi::FutureSync<void> GenericObject::disconnect(Link linkId)
  {
    if (!type || !value) {
      qiLogWarning() << "Operating on invalid GenericObject..";
      return qi::makeFutureError<void>("Operating on invalid GenericObject");
    }
    return type->disconnect(value, this, linkId);
  }

  qi::FutureSync<GenericValue> GenericObject::property(unsigned int id) {
    return type->property(value, id);
  }

  qi::FutureSync<void> GenericObject::setProperty(unsigned int id, const GenericValue& val) {
    return type->setProperty(value, id, val);
  }

  qi::FutureSync<Link> GenericObject::connect(unsigned int signal, ObjectPtr target, unsigned int slot)
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
                         qi::AutoGenericValuePtr p1,
                         qi::AutoGenericValuePtr p2,
                         qi::AutoGenericValuePtr p3,
                         qi::AutoGenericValuePtr p4,
                         qi::AutoGenericValuePtr p5,
                         qi::AutoGenericValuePtr p6,
                         qi::AutoGenericValuePtr p7,
                         qi::AutoGenericValuePtr p8)
  {
    if (!type || !value) {
      qiLogWarning() << "Operating on invalid GenericObject..";
      return;
    }
    qi::AutoGenericValuePtr* vals[8]= {&p1, &p2, &p3, &p4, &p5, &p6, &p7, &p8};
    std::vector<qi::GenericValuePtr> params;
    for (unsigned i=0; i<8; ++i)
      if (vals[i]->type)
        params.push_back(*vals[i]);
    metaPost(nameWithOptionalSignature, GenericFunctionParameters(params));
  }

  int ObjectType::inherits(Type* other)
  {
    /* A registered class C can have to Type* around:
    * - TypeImpl<C*>
    * - The staticObjectType that was created by the builder.
    * So assume that any of them can be in the parentTypes list.
    */
    if (this == other)
      return 0;
    const std::vector<std::pair<Type*, int> >& parents = parentTypes();
    qiLogDebug() << infoString() <<" has " << parents.size() <<" parents";
    for (unsigned i=0; i<parents.size(); ++i)
    {
      if (parents[i].first->info() == other->info())
        return parents[i].second;
      ObjectType* op = dynamic_cast<ObjectType*>(parents[i].first);
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
      <<" " << ((op != 0) == (dynamic_cast<ObjectType*>(other) != 0));
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

