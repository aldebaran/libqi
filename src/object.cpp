/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/
#include <iostream>

#include <qitype/genericobject.hpp>
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
      ss << "  " << mm.signature() << " (" << it->second << ')' << std::endl;
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
  unsigned int
  GenericObject::findMethod(const std::string& signature, const GenericFunctionParameters& args)
  {
    typedef std::vector<MetaObject::CompatibleMethod> Methods;
    const MetaObject& mo = metaObject();
    if (signature.find(':') != signature.npos)
      return mo.methodId(signature);
    for (unsigned dyn = 0; dyn<2; ++dyn)
    {
      std::string resolvedSig = "(";
      for (unsigned i=0; i<args.size(); ++i)
        resolvedSig += args[i].signature(dyn==1);
      resolvedSig = resolvedSig + ')';
      std::string fullSig = signature + "::" + resolvedSig;
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
        qiLogVerbose() << generateErrorString("ambiguous overload", signature, mml, false);
      else
        return it->first.uid();
    }
    return -1;
  }
  qi::Future<GenericValuePtr>
  GenericObject::metaCall(const std::string &signature, const GenericFunctionParameters& args, MetaCallType callType)
  {
    qi::Promise<GenericValuePtr> out;
    if (!type || !value) {
      qiLogWarning() << "Operating on invalid GenericObject..";
      out.setError("Invalid object");
      return out.future();
    }
    int methodId = findMethod(signature, args);
    if (methodId < 0) {
      return makeFutureError<GenericValuePtr>(generateErrorString("method", signature, metaObject().findCompatibleMethod(signature)));
    }
    return metaCall(methodId, args, callType);
  }

  /// Resolve signature and bounce
  bool GenericObject::xMetaPost(const std::string &signature, const GenericFunctionParameters &in) {
    if (!value || !type) {
      qiLogWarning() << "Operating on invalid GenericObject..";
      return false;
    }
       int eventId = metaObject().signalId(signature);
    if (eventId < 0)
      eventId = metaObject().methodId(signature);
    if (eventId < 0) {
      std::stringstream ss;
      ss << "Can't find event: " << signature << std::endl
         << "  Candidate(s):" << std::endl;
      std::vector<MetaSignal>           mml = metaObject().findSignal(qi::signatureSplit(signature)[1]);
      std::vector<MetaSignal>::const_iterator it;

      for (it = mml.begin(); it != mml.end(); ++it) {
        ss << "  " << it->signature() << std::endl;
      }
      qiLogError() << ss.str();
      return false;
    }
    metaPost(eventId, in);
    return true;
  }

  /// Resolve signature and bounce
  qi::FutureSync<Link> GenericObject::xConnect(const std::string &signature, const SignalSubscriber& functor)
  {
    if (!type || !value) {
      qiLogWarning() << "Operating on invalid GenericObject..";
      return qi::makeFutureError<Link>("Operating on invalid GenericObject..");
    }
    int eventId = metaObject().signalId(signature);

  #ifndef QI_REQUIRE_SIGNATURE_EXACT_MATCH
    if (eventId < 0) {
      // Try to find an other event with compatible signature
      std::vector<qi::MetaSignal> mml = metaObject().findSignal(qi::signatureSplit(signature)[1]);
      Signature sargs(signatureSplit(signature)[2]);
      for (unsigned i = 0; i < mml.size(); ++i)
      {
        Signature s(signatureSplit(mml[i].signature())[2]);
        qiLogDebug() << "Checking compatibility " << s.toString() << ' '
         << sargs.toString();
         // Order is reversed from method call check.
        if (s.isConvertibleTo(sargs))
        {
          qiLogVerbose()
              << "Signature mismatch, but found compatible type "
              << mml[i].signature() <<" for " << signature;
          eventId = mml[i].uid();
          break;
        }
      }
    }
#endif
    if (eventId < 0) {
      std::stringstream ss;
      ss << "Can't find event: " << signature << std::endl
         << "  Candidate(s):" << std::endl;
      std::vector<MetaSignal>           mml = metaObject().findSignal(qi::signatureSplit(signature)[1]);
      std::vector<MetaSignal>::const_iterator it;

      for (it = mml.begin(); it != mml.end(); ++it) {
        ss << "  " << it->signature() << std::endl;
      }
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

  void GenericObject::emitEvent(const std::string& eventName,
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
    // Signature construction
    std::string signature = eventName + "::(";
    for (unsigned i=0; i< params.size(); ++i)
      signature += params[i].signature();
    signature += ")";
    xMetaPost(signature, GenericFunctionParameters(params));
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

