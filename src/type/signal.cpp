/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/
#include <atomic>
#include <map>
#include <numeric>

#include <boost/thread/recursive_mutex.hpp>
#include <boost/make_shared.hpp>

#include <qi/signal.hpp>
#include <qi/anyvalue.hpp>
#include <qi/anyobject.hpp>
#include <qi/assert.hpp>

#include "signal_p.hpp"

qiLogCategory("qitype.signal");

namespace qi {

  SignalBasePrivate::~SignalBasePrivate()
  {
    {
      boost::recursive_mutex::scoped_lock lock(mutex);
      onSubscribers = SignalBase::OnSubscribers();
    }
    disconnectAll();
  }

  Future<bool> SignalBasePrivate::disconnect(const SignalLink& l)
  {
    SignalSubscriber s;
    {
      // Acquire signal mutex
      boost::recursive_mutex::scoped_lock sigLock(mutex);
      SignalSubscriberMap::iterator it = subscriberMap.find(l);
      if (it == subscriberMap.end())
        return Future<bool>{false};
      s = it->second;
      // Remove from map (but SignalSubscriber object still good)
      subscriberMap.erase(it);
      if (subscriberMap.empty() && onSubscribers)
        onSubscribers(false).value();
      // Acquire subscriber mutex before releasing mutex
      boost::mutex::scoped_lock subLock(s._p->mutex);
      // Release signal mutex
      sigLock.unlock();
      // Ensure no call on subscriber occurs once this function returns
      s._p->enabled = false;

      if (s._p->activeThreads.empty()
          || (s._p->activeThreads.size() == 1
            && *s._p->activeThreads.begin() == boost::this_thread::get_id()))
      { // One active callback in this thread, means above us in call stack
        // So we cannot trash s right now
        return Future<bool>{true};
      }
      // More than one active callback, or one in a state that prevent us
      // from knowing in which thread it will run
      subLock.release()->unlock();
    }
    return s.waitForInactive().async().andThen([](void*){ return true; });
  }


  Future<bool> SignalBasePrivate::disconnectAll()
  {
    return disconnectAllStep(true);
  }

  Future<bool> SignalBasePrivate::disconnectAllStep(bool overallSuccess)
  {
    SignalLink link;
    FutureBarrier<bool> barrier;
    while (true)
    {
      {
        boost::recursive_mutex::scoped_lock sl(mutex);
        SignalSubscriberMap::iterator it = subscriberMap.begin();
        if (it == subscriberMap.end())
          break;
        link = it->first;
      }
      // allow for multiple disconnects to occur at the same time, we must not
      // keep the lock
      barrier.addFuture(disconnect(link));
    }
    return barrier.future().andThen([](const std::vector<Future<bool>>& successes)
    {
      for (const auto& success: successes)
        if (!success) return false;
      return true;
    });
  }

  SignalSubscriberPrivate::SignalSubscriberPrivate() = default;
  SignalSubscriberPrivate::~SignalSubscriberPrivate() = default;

  SignalSubscriber::SignalSubscriber()
    : _p(std::make_shared<SignalSubscriberPrivate>())
    , linkId(_p->linkId)
  {
  }

  SignalSubscriber::SignalSubscriber(const SignalSubscriber& other) = default;
  SignalSubscriber& SignalSubscriber::operator=(const SignalSubscriber& other) = default;

  SignalSubscriber::SignalSubscriber(const AnyObject& target, unsigned int method)
    : SignalSubscriber()
  { // The slot has its own threading model: use sync call type (default)
    _p->target.reset(new AnyWeakObject(target));
    _p->method = method;
  }

  SignalSubscriber::SignalSubscriber(AnyFunction func, MetaCallType model)
     : SignalSubscriber()
  {
    _p->handler = func;
    _p->threadingModel = model;
  }

  SignalSubscriber::SignalSubscriber(AnyFunction func, ExecutionContext* ec)
    : SignalSubscriber()
  { // The execution context will reschedule the call like it wants, use sync call type
    _p->handler = func;
    _p->executionContext = ec;
  }

  SignalSubscriber::~SignalSubscriber() = default;

  static std::atomic<SignalLink> linkUid{1};

  void SignalBase::setCallType(MetaCallType callType)
  {
    QI_ASSERT(_p);
    boost::recursive_mutex::scoped_lock lock(_p->mutex);
    _p->defaultCallType = callType;
  }

  void SignalBase::operator()(
      qi::AutoAnyReference p1,
      qi::AutoAnyReference p2,
      qi::AutoAnyReference p3,
      qi::AutoAnyReference p4,
      qi::AutoAnyReference p5,
      qi::AutoAnyReference p6,
      qi::AutoAnyReference p7,
      qi::AutoAnyReference p8)
  {
    qi::AutoAnyReference* vals[8]= {&p1, &p2, &p3, &p4, &p5, &p6, &p7, &p8};
    std::vector<qi::AnyReference> params;
    for (unsigned i = 0; i < 8; ++i)
      if (vals[i]->isValid())
        params.push_back(*vals[i]);
    qi::Signature signature = qi::makeTupleSignature(params);

    auto mct = [&] () {
      QI_ASSERT(_p);
      boost::recursive_mutex::scoped_lock lock(_p->mutex);
      if (signature != _p->signature)
      {
        qiLogError() << "Dropping emit: signature mismatch: "
                     << signature.toString() << " " << _p->signature.toString();
        return MetaCallType_Auto;
      }
      return _p->defaultCallType;
    }();

    trigger(params, mct);
  }

  void SignalBase::trigger(const GenericFunctionParameters& params, MetaCallType callType)
  {
    QI_ASSERT(_p);
    SignalBase::Trigger trigger;
    {
      boost::recursive_mutex::scoped_lock lock(_p->mutex);
      trigger = _p->triggerOverride;
    }
    if (trigger)
      trigger(params, callType);
    else
      callSubscribers(params, callType);
  }

  void SignalBase::setTriggerOverride(Trigger t)
  {
    QI_ASSERT(_p);
    boost::recursive_mutex::scoped_lock lock(_p->mutex);
    _p->triggerOverride = t;
  }

  void SignalBase::setOnSubscribers(OnSubscribers onSubscribers)
  {
    QI_ASSERT(_p);
    boost::recursive_mutex::scoped_lock lock(_p->mutex);
    _p->onSubscribers = onSubscribers;
  }

  void SignalBase::callSubscribers(const GenericFunctionParameters& params, MetaCallType callType)
  {
    MetaCallType mct = callType;
    QI_ASSERT(_p);

    SignalSubscriberMap copy;
    {
      boost::recursive_mutex::scoped_lock lock(_p->mutex);
      if (mct == qi::MetaCallType_Auto)
        mct = _p->defaultCallType;

      copy = _p->subscriberMap;
    }
    qiLogDebug() << (void*)this << " Invoking signal subscribers: " << copy.size();
    for (auto& i: copy)
    {
      qiLogDebug() << (void*)this << " Invoking signal subscriber";
      SignalSubscriber s = i.second; // holds the subscription alive
      s.call(params, mct);
    }
    qiLogDebug() << (void*)this << " done invoking signal subscribers";
  }

  void SignalSubscriber::callImpl(const GenericFunctionParameters& args)
  {
    // verify-enabled-then-register-active op must be locked
    {
      boost::mutex::scoped_lock sl(_p->mutex);
      if (!_p->enabled)
        return;
      addActive(false);
    }
    //do not throw
    bool mustDisconnect = false;
    try
    {
      _p->handler(args);
    }
    catch (const qi::PointerLockException&)
    {
      qiLogDebug() << "PointerLockFailure excepton, will disconnect";
      mustDisconnect = true;
    }
    catch (const std::exception& e)
    {
      qiLogWarning() << "Exception caught from signal subscriber: " << e.what();
    }
    catch (...)
    {
      qiLogWarning() << "Unknown exception caught from signal subscriber";
    }
    removeActive(true);

    if (mustDisconnect)
    {
      boost::mutex::scoped_lock sl(_p->mutex);
      // if enabled is false, we are already disconnected
      if (_p->enabled)
      {
        boost::shared_ptr<SignalBasePrivate> sbp = _p->source->_p;
        sl.unlock();
        sbp->disconnect(_p->linkId);
      }
    }
  }

  void SignalSubscriber::call(const GenericFunctionParameters& args, MetaCallType callType)
  {
    // this is held alive by caller
    if (_p->handler)
    {
      bool async = true;
      if (_p->threadingModel != MetaCallType_Auto)
        async = (_p->threadingModel == MetaCallType_Queued);
      else if (callType != MetaCallType_Auto)
        async = (callType == MetaCallType_Queued);

      qiLogDebug() << "subscriber call async=" << async <<" ct " << callType <<" tm " << _p->threadingModel;
      if (_p->executionContext || async)
      {
        // We will check enabled when we will be scheduled in the target
        // thread, and we hold this SignalSubscriber alive, so no need to
        // explicitly track the asynccall

        // courtesy-check of el, but it should be kept alive longuer than us
        qi::ExecutionContext* ec = _p->executionContext;
        if (!ec)
        {
          ec = getEventLoop();
          if (!ec) // this is an assert basicaly, no sense trying to do something clever.
            throw std::runtime_error("Event loop was destroyed");
        }

        auto subscriberCopy = *this;
        std::shared_ptr<GenericFunctionParameters> argsCopy{ new auto(args.copy()), [](GenericFunctionParameters* object) {
          object->destroy(); // see GenericFunctionParameters::copy() for details
          delete object;
        } };

        ec->post([subscriberCopy, argsCopy] () mutable{
          subscriberCopy.callImpl(*argsCopy);
        });

      }
      else
      {
        callImpl(args);
      }
    }
    else if (_p->target)
    {
      AnyObject lockedTarget = _p->target->lock();
      if (!lockedTarget)
      {
        boost::mutex::scoped_lock sl(_p->mutex);
        if (_p->enabled)
        {
          // see above
          boost::shared_ptr<SignalBasePrivate> sbp = _p->source->_p;
          sl.unlock();
          sbp->disconnect(_p->linkId);
        }
      }
      else // no need to keep anything locked, whatever happens this is not used
        lockedTarget.metaPost(_p->method, args);
    }
  }

  SignalSubscriber SignalSubscriber::setCallType(MetaCallType ct)
  {
    _p->threadingModel = ct;
    return *this;
  }

  //check if we are called from the same thread that triggered us.
  //in that case, do not wait.
  FutureSync<void> SignalSubscriber::waitForInactive()
  {
    boost::thread::id tid = boost::this_thread::get_id();
    boost::mutex::scoped_lock sl(_p->mutex);
    if (_p->activeThreads.empty())
      return Future<void>{0};

    // There cannot be two activeThreads entry for the same tid
    // because activeThreads is not set at the post() stage
    if (_p->activeThreads.size() == 1
      && *_p->activeThreads.begin() == tid)
    { // One active callback in this thread, means above us in call stack
      // So we cannot wait for it
      return Future<void>{0};
    }
    return _p->inactive.future();
  }

  void SignalSubscriber::addActive(bool acquireLock, boost::thread::id id)
  {
    boost::mutex::scoped_lock l;
    if (acquireLock)
      l = boost::mutex::scoped_lock{_p->mutex};

    if (_p->activeThreads.empty())
      _p->inactive = Promise<void>{};
    _p->activeThreads.push_back(id);
  }

  void SignalSubscriber::removeActive(bool acquireLock, boost::thread::id id)
  {
    boost::mutex::scoped_lock sl(_p->mutex, boost::defer_lock_t());
    if (acquireLock)
      sl.lock();

    for (unsigned i=0; i<_p->activeThreads.size(); ++i)
    {
      if (_p->activeThreads[i] == id)
      { // fast remove by swapping with last and then pop_back
        _p->activeThreads[i] = _p->activeThreads[_p->activeThreads.size() - 1];
        _p->activeThreads.pop_back();
      }
    }
    if (_p->activeThreads.empty())
      _p->inactive.setValue(0);
  }

  SignalLink SignalSubscriber::link() const
  {
    return _p->linkId;
  }

  SignalSubscriber::operator SignalLink() const
  {
    return _p->linkId;
  }

  SignalSubscriber SignalBase::connect(AnyObject o, unsigned int slot)
  {
    return connect(SignalSubscriber(o, slot));
  }

  Signature SignalSubscriber::signature() const
  {
    if (_p->handler)
    {
      if (_p->handler.functionType() == dynamicFunctionTypeInterface())
        return Signature(); // no arity checking is possible
      else
        return _p->handler.parametersSignature();
    }
    else if (_p->target)
    {
      AnyObject locked = _p->target->lock();
      if (!locked)
        return Signature();
      const MetaMethod* ms = locked.metaObject().method(_p->method);
      if (!ms)
      {
        qiLogWarning() << "Method " << _p->method <<" not found.";
        return Signature();
      }
      else
        return ms->parametersSignature();
    }
    else
      return Signature();
  }

  SignalSubscriber SignalBase::connect(const SignalSubscriber& src)
  {
    return connectAsync(src).value();
  }

  Future<SignalSubscriber> SignalBase::connectAsync(const SignalSubscriber& src)
  {
    qiLogDebug() << (void*)this << " connecting new subscriber";
    QI_ASSERT(_p);
    // Check arity. Does not require to acquire weakLock.
    int signalArity = signature().children().size();
    int subscriberArity = -1;
    Signature subscriberSignature = src.signature();
    if (subscriberSignature.isValid())
      subscriberArity = subscriberSignature.children().size();

    if (signature() != "m" && subscriberSignature.isValid())
    {
      if (signalArity != subscriberArity)
      {
        std::stringstream s;
        s << "Subscriber has incorrect arity (expected maximum "
          << signalArity << " , got " << subscriberArity << ")";
        throw std::runtime_error(s.str());
      }
      if (!signature().isConvertibleTo(subscriberSignature))
      {
        std::stringstream s;
        s << "Subscriber is not compatible to signal : "
          << signature().toString() << " vs " << subscriberSignature.toString();
        throw std::runtime_error(s.str());
      }
    }

    boost::recursive_mutex::scoped_lock sl(_p->mutex);
    bool first = _p->subscriberMap.empty();
    SignalLink res = ++linkUid;
    SignalSubscriber& subscriberInMap = _p->subscriberMap[res];
    subscriberInMap = src;
    subscriberInMap._p->linkId = res;
    subscriberInMap._p->source = this;
    Future<void> callingOnSubscribers{0};
    if (first && _p->onSubscribers)
      callingOnSubscribers = _p->onSubscribers(true);

    // Return a copy asynchronously. Too bad it makes few allocations.
    SignalSubscriber subscriberToReturn = subscriberInMap;
    return callingOnSubscribers.andThen([subscriberToReturn](void*) mutable { return subscriberToReturn; });
  }

  void SignalBase::createNewTrackLink(int& id, SignalLink*& pLink)
  {
    id = ++_p->trackId;
    {
      boost::recursive_mutex::scoped_lock l(_p->mutex);
      pLink = &_p->trackMap[id];
    }
  }

  void SignalBase::disconnectTrackLink(int id)
  {
    boost::recursive_mutex::scoped_lock sl(_p->mutex);
    TrackMap::iterator it = _p->trackMap.find(id);
    if (it == _p->trackMap.end())
      return;

    _p->subscriberMap.erase(it->second);
    _p->trackMap.erase(it);
  }

  bool SignalBase::disconnectAll()
  {
    QI_ASSERT(_p);
    return _p->disconnectAll().value();
  }

  Future<bool> SignalBase::disconnectAllAsync()
  {
    QI_ASSERT(_p);
    return _p->disconnectAll();
  }

  SignalBase::SignalBase(const qi::Signature& sig, OnSubscribers onSubscribers)
    : _p(new SignalBasePrivate)
  {
    //Dynamic mean AnyArguments here.
    if (sig.type() != qi::Signature::Type_Dynamic && sig.type() != qi::Signature::Type_Tuple)
      throw std::runtime_error("Signal signature should be tuple, or AnyArguments");
    _p->onSubscribers = onSubscribers;
    _p->signature = sig;
  }

  SignalBase::SignalBase(OnSubscribers onSubscribers)
  : _p(new SignalBasePrivate)
  {
    _p->onSubscribers = onSubscribers;
  }


  qi::Signature SignalBase::signature() const
  {
    QI_ASSERT(_p);
    boost::recursive_mutex::scoped_lock lock(_p->mutex);
    return _p->signature;
  }

  void SignalBase::_setSignature(const qi::Signature& s)
  {
    boost::recursive_mutex::scoped_lock lock(_p->mutex);
    _p->signature = s;
  }


  bool SignalBase::disconnect(const SignalLink &link) {
    QI_ASSERT(_p);
    return _p->disconnect(link).value();
  }

  Future<bool> SignalBase::disconnectAsync(const SignalLink &link) {
    QI_ASSERT(_p);
    return _p->disconnect(link);
  }

  SignalBase::~SignalBase()
  {
  }

  std::vector<SignalSubscriber> SignalBase::subscribers()
  {
    std::vector<SignalSubscriber> res;
    QI_ASSERT(_p);
    boost::recursive_mutex::scoped_lock sl(_p->mutex);
    for (const auto& i: _p->subscriberMap)
      res.push_back(i.second);
    return res;
  }

  bool SignalBase::hasSubscribers()
  {
    QI_ASSERT(_p);
    boost::recursive_mutex::scoped_lock sl(_p->mutex);
    return !_p->subscriberMap.empty();
  }

  SignalSubscriber SignalBase::connect(AnyObject obj, const std::string& slot)
  {
    const MetaObject& mo = obj.metaObject();
    const MetaSignal* sig = mo.signal(slot);
    if (sig)
      return connect(SignalSubscriber(obj, sig->uid()));
    std::vector<MetaMethod> method = mo.findMethod(slot);
    if (method.empty())
      throw std::runtime_error("No match found for slot " + slot);
    if (method.size() > 1)
      throw std::runtime_error("Ambiguous slot name " + slot);
    return connect(SignalSubscriber(obj, method.front().uid()));
  }

  const SignalLink SignalBase::invalidSignalLink =
      std::numeric_limits<SignalLink>::max();

}
